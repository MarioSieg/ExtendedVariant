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
#include <limits>
#include <tuple>
#include <type_traits>
#include <variant>

// std extensions
namespace stdex
{
	template <typename... Ts>
	class variant
	{
	public:
		struct detail final
		{
			/* Validate generic types: */
			static_assert(sizeof...(Ts), "Type list must be above zero!");
			static_assert(sizeof...(Ts) < std::numeric_limits<std::size_t>::max(), "Type count must <= size_t::max!");
			static_assert(std::conjunction_v<std::is_object<Ts>..., std::negation<std::is_array<Ts>>..., std::is_destructible<Ts>...>,
				"Types must be objects, no array and destructible!");

			/* The maximum size of one type in the collection. */
			static constexpr std::size_t max_size{ std::max({sizeof(Ts)...}) };

			/* The maximum alignment of one type in the collection. */
			static constexpr std::size_t max_align{ std::max({alignof(Ts)...}) };

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

			/* Queries discriminator data types depending on generic type count. */
			template <const std::size_t N = sizeof...(Ts)>
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

			/* Direct discriminator type. */
			using discriminator_v = typename discriminator<>::type;

			/* Invoke constructor on raw blob. */
			template <typename T, typename... Ctor, typename = std::is_constructible<T, Ctor...>>
			static inline auto construct(void* const blob, Ctor&&... ctor) -> void
			{
				new(blob) T(std::forward<Ctor>(ctor)...);
			}

			/* Invoke destructor on raw blob. */
			template <typename T, typename = std::enable_if<std::is_destructible_v<T>>>
			static inline auto destruct(void* const blob) noexcept(true) -> void
			{
				(*static_cast<T*>(blob)).~T();
			}

		};

		using discriminator_v = typename detail::discriminator_v;
		using storage_v = typename detail::storage;

	private:
		/* Index. */
		discriminator_v discriminator_;

		/* Data storage. */
		alignas(detail::max_align) storage_v storage_;

	public:
		/* <<< STL Interface >>> */

		constexpr variant() noexcept(std::is_nothrow_constructible_v<typename detail::first>) : discriminator_ {0}, storage_ {}
		{
			static_assert(std::is_default_constructible_v<typename detail::first>, "Default constructor requires the first element to be default constructible!");
			if constexpr (!std::is_scalar_v<typename detail::first>)
			{
				detail::template construct<typename detail::first>(&this->storage_);
			}
		}

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
			discriminator_v r{ 0 };
			const auto accumulator = [&r](const bool equ) noexcept(true)
			{
				r += !equ;
				return equ;
			};
			(accumulator(std::is_same_v<T, Ts>) || ...);
			return r;
		}
	};
}

#endif