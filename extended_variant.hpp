/*
	MIT License

	Copyright 2021 Mario Sieg "pinsrq" <mt3000@gmx.de>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

#ifndef EXTENDED_VARIANT_HPP
#define EXTENDED_VARIANT_HPP

#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

// std extensions
namespace stdex
{
	namespace detail
	{
		/* Allocator. */
		template <typename T, typename... Ctor, const bool NothrowAlloc = false, typename = std::is_constructible<T, Ctor...>>
		inline auto alloc(T*& outObj, Ctor&&...ctor) noexcept(std::is_nothrow_constructible_v<T, Ctor...>) -> void
		{
			if constexpr (NothrowAlloc)
			{
				outObj = &static_cast<T&>(*new(std::nothrow) T(std::forward<Ctor>(ctor)...));
			}
			else
			{
				outObj = static_cast<T*>(new T(std::forward<Ctor>(ctor)...));
			}
		}

		/* Deallocator. */
		template <typename T, typename = std::enable_if_t<std::is_destructible_v<T>>>
		inline auto dealloc(T*& inoutObj) -> void
		{
			delete inoutObj;
			inoutObj = nullptr;
		}

		/* Invoke constructor on raw blob. */
		template <typename T, typename... Ctor, typename = std::enable_if_t<std::is_constructible_v<T, Ctor...>>>
		inline auto construct(void* const blob, Ctor&&...ctor) noexcept(std::is_nothrow_constructible_v<T, Ctor...>) -> void
		{
			new(blob) T(std::forward<Ctor>(ctor)...);
		}

		/* Invoke destructor on raw blob. */
		template <typename T, typename = std::enable_if_t<std::is_destructible_v<T>>>
		inline auto destruct(void* const blob) noexcept(true) -> void
		{
			(*static_cast<T*>(blob)).~T();
		}

		/* Queries discriminator data types depending on generic type count. */
		template <const std::size_t N>
		struct discriminator final
		{
			/* Selects the minimum index type depending on the number of types.
			 * if type_count in range(0, uint8_max) => uint8
			 * if type_count in range(0, uint16_max) => uint16
			 * if type_count in range(0, uint32_max) => uint32
			 * if type_count in range(0, size_t_max) => size_t
			 */
			using type =
			std::conditional_t<N <= std::numeric_limits<std::uint8_t>::max(), std::uint8_t,
			                   std::conditional_t<N <= std::numeric_limits<std::uint16_t>::max(), std::uint16_t,
			                                      std::conditional_t<N <= std::numeric_limits<std::uint32_t>::max(), std::uint32_t, std::size_t>>>;
		};

		/* Validate single type. */
		template <typename... Ts>
		struct monotonic_validator final
		{
			static constexpr bool value
			{
				std::conjunction_v
				<
					std::is_object<Ts>...,
					std::negation<std::is_array<Ts>>...,
					std::is_destructible<Ts>...
				>
			};
		};

		/* Validate single type. */
		template <typename... Ts>
		constexpr auto monotonic_validator_v {monotonic_validator<Ts...>::value};
	}

	/* A cleaner and more intuitive std::variant alternative. */
	template <typename... Ts>
	class variant final
	{
	public:
		struct detail final
		{
			/* Validate generic types: */
			static_assert(sizeof...(Ts), "Type list must be above zero!");
			static_assert(sizeof...(Ts) < std::numeric_limits<std::size_t>::max(), "Type count must <= size_t::max!");
			static_assert(stdex::detail::monotonic_validator<Ts...>::value, "Types must be destructible objects and no arrays!");

			/* The maximum size of one type in the collection. */
			static constexpr std::size_t max_size {std::max({sizeof(Ts)...})};

			/* The maximum alignment of one type in the collection. */
			static constexpr std::size_t max_align {std::max({alignof(Ts)...})};

			/* A normal std::variant holding the types. */
			using std_variant = std::variant<Ts...>;

			/* A normal std::tuple holding the types. */
			using std_tuple = std::tuple<Ts...>;

			/* First type. */
			using first = std::tuple_element_t<0, std_tuple>;

			/* Last type. */
			using last = std::tuple_element_t<sizeof...(Ts) - 1, std_tuple>;

			/* The type used to store the data. */
			using storage = std::array<std::byte, max_size>;

			/* Direct discriminator type. */
			using discriminator_v = typename stdex::detail::discriminator<sizeof...(Ts)>::type;
		};

		using discriminator_v = typename detail::discriminator_v;
		using storage_v = typename detail::storage;

	private:
		/* Data storage. */
		alignas(detail::max_align) storage_v storage_;

		/* Index. */
		discriminator_v discriminator_;

		template <typename T>
		inline auto access_as() noexcept(true) -> T&
		{
			return *reinterpret_cast<T*>(std::addressof(this->storage_));
		}

		template <typename T>
		inline auto access_as() const noexcept(true) -> const T&
		{
			return *reinterpret_cast<const T*>(std::addressof(this->storage_));
		}

	public:
		/* <<< STL Interface >>> */

		constexpr variant() noexcept(std::is_nothrow_constructible_v<typename detail::first>);

		~variant();

		[[nodiscard]]
		constexpr auto index() const noexcept(true) -> discriminator_v
		{
			return this->discriminator_;
		}

		/* <<< Extensions >>> */

		/* Returns the index of the specified type. */
		template <typename T>
		[[nodiscard]]
		static constexpr auto index_of() noexcept(true) -> discriminator_v
		{
			discriminator_v r {0};
			const auto      accumulator = [&r](const bool equ) noexcept(true)
			{
				r += !equ;
				return equ;
			};
			(accumulator(std::is_same_v<T, Ts>) || ...);
			return r;
		}

		/* Check if variant currently holds T. */
		template <typename T, typename = std::enable_if_t<stdex::detail::monotonic_validator_v<T>>>
		[[nodiscard]]
		constexpr auto holds_alternative() const noexcept(true) -> bool
		{
			return this->discriminator_ == index_of<T>();
		}

		/* Check if variant currently holds T and if the values match. */
		template <typename T, typename = std::enable_if_t<stdex::detail::monotonic_validator_v<T>>>
		[[nodiscard]]
		inline auto holds_value(T&& other) const noexcept(true) -> bool
		{
			return this->discriminator_ == index_of<T>() && this->access_as<T>() == other;
		}

		/* Returns optional which contains the value if T is the current type, else std::nullopt. */
		template <typename T, typename = std::enable_if_t<stdex::detail::monotonic_validator_v<T>>>
		[[nodiscard]]
		inline auto get() const noexcept(true) -> std::optional<T>
		{
			return this->holds_alternative<T>() ? std::optional<T> {this->access_as<T>()} : std::optional<T> {std::nullopt};
		}

		/*
		 * Returns the containing value of T if T is the current type, else the default value of T.
		 * T must be default constructible.
		 */
		template <typename T, typename = std::enable_if_t<::stdex::detail::monotonic_validator_v<T> && std::is_default_constructible_v<T>>>
		[[nodiscard]]
		inline auto get_or_default() const noexcept(true) -> T
		{
			return this->holds_alternative<T>() ? this->access_as<T>() : T { };
		}

		/*
		 *  Returns the containing value of T if T is the current type, else the a custom value of T.
		 */
		template <typename T, typename = std::enable_if_t<stdex::detail::monotonic_validator_v<T>>>
		[[nodiscard]]
		inline auto get_or_custom_value(T&& instead) const noexcept(true) -> T
		{
			return this->holds_alternative<T>() ? this->access_as<T>() : instead;
		}


		/*
		 *  Returns the containing value of T if T is the current type, else invokes the lambda which returns a value instead.
		 */
		template <typename T, typename F, typename... Args, typename = std::enable_if_t<stdex::detail::monotonic_validator_v<T>>>
		[[nodiscard]]
		inline auto get_or_invoke(F&& functor, Args&&...args) const noexcept(true) -> T
		{
			static_assert(std::is_convertible_v<decltype(std::invoke(functor, std::forward<Args>()...)), T>, "Functor must return a T convertible type!");
			return this->holds_alternative<T>() ? this->access_as<T>() : std::invoke(functor, std::forward<Args>()...);
		}
	};

	namespace detail
	{
		template <typename... Ty>
		struct recursive_invoker;

		template <typename T, typename... Ty>
		struct recursive_invoker<T, Ty...> final
		{
			using mapping = variant<T, Ty...>;

			template <typename... Ctor>
			static inline auto dynamic_construct(void* const blob, const typename mapping::discriminator_v idx, Ctor&&...ctor) noexcept(std::is_nothrow_constructible_v<T, Ctor...>) -> void
			{
				if (idx == variant<T, Ty...>::template index_of<T>())
				{
					construct<T>(blob, std::forward<Ctor>(ctor)...);
				}
				else
				{
					recursive_invoker<Ty...>::dynamic_construct(blob, idx, std::forward<Ctor>(ctor)...);
				}
			}

			static inline auto dynamic_destruct(void* const blob, const typename mapping::discriminator_v idx) noexcept(std::is_nothrow_destructible_v<T>) -> void
			{
				if (idx == mapping::template index_of<T>())
				{
					destruct<T>(blob);
				}
				else
				{
					recursive_invoker<Ty...>::dynamic_destruct(blob, idx);
				}
			}
		};

		template <>
		struct recursive_invoker<> final
		{
			template <typename... Ctor>
			static inline auto dynamic_construct(void* const, const std::size_t, Ctor&&...) noexcept(true) -> void { }

			static inline auto dynamic_destruct(void* const, const std::size_t) noexcept(true) -> void { }
		};
	}

	template <typename ... Ts>
	constexpr variant<Ts...>::variant() noexcept(std::is_nothrow_constructible_v<typename detail::first>) : storage_ { }, discriminator_ {0}
	{
		static_assert(std::is_default_constructible_v<typename detail::first>, "Default constructor requires the first element to be default constructible!");
		if constexpr (!std::is_scalar_v<typename detail::first>)
		{
			stdex::detail::construct<typename detail::first>(std::addressof(this->storage_));
		}
	}

	template <typename ... Ts>
	inline variant<Ts...>::~variant()
	{
		stdex::detail::recursive_invoker<Ts...>::dynamic_destruct(std::addressof(this->storage_), this->discriminator_);
	}
}

#endif
