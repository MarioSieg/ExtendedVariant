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

#include "extended_variant.hpp"

#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

// std extensions
namespace stdex
{
	// static tests
	class variant_tests final
	{
		// max size
		static_assert(variant<std::int8_t, std::int8_t>::detail::max_size == 1);
		static_assert(variant<std::int8_t, std::int16_t>::detail::max_size == 2);
		static_assert(variant<std::int32_t, std::int16_t>::detail::max_size == 4);
		static_assert(variant<std::int32_t, std::int64_t>::detail::max_size == 8);
		static_assert(variant<std::int8_t, std::string>::detail::max_size == sizeof(std::string));
		static_assert(variant<std::int8_t, std::int16_t>::detail::max_size == 2);

		// max alignment
		static_assert(variant<std::int8_t, std::int8_t>::detail::max_align == 1);
		static_assert(variant<std::int8_t, std::int16_t>::detail::max_align == 2);
		static_assert(variant<std::int32_t, std::int16_t>::detail::max_align == 4);
		static_assert(variant<std::int32_t, std::int64_t>::detail::max_align == 8);
		static_assert(variant<std::int32_t, std::string>::detail::max_align == alignof(std::string));
		static_assert(variant<std::int32_t, std::int32_t, std::max_align_t>::detail::max_align == alignof(std::max_align_t));

		// std variant
		static_assert(std::is_same_v<variant<std::int8_t, float, std::string>::detail::std_variant, std::variant<std::int8_t, float, std::string>>);

		// index of
		static_assert(variant<std::int8_t, float, std::string>::index_of<std::int8_t>() == 0);
		static_assert(variant<std::int8_t, float, std::string>::index_of<float>() == 1);
		static_assert(variant<std::int8_t, float, std::string>::index_of<std::string>() == 2);

		// discriminator
		static_assert(std::is_same_v<variant<std::int8_t, float, std::string>::discriminator_v, std::uint8_t>);
		static_assert(std::is_same_v<detail::discriminator<std::numeric_limits<std::uint8_t>::max()>::type, std::uint8_t>);
		static_assert(std::is_same_v<detail::discriminator<std::numeric_limits<std::uint8_t>::max() + 1>::type, std::uint16_t>);
		static_assert(std::is_same_v<detail::discriminator<std::numeric_limits<std::uint16_t>::max()>::type, std::uint16_t>);
		static_assert(std::is_same_v<detail::discriminator<std::numeric_limits<std::uint16_t>::max() + 1>::type, std::uint32_t>);
		static_assert(std::is_same_v<detail::discriminator<std::numeric_limits<std::uint32_t>::max()>::type, std::uint32_t>);
		static_assert(std::is_same_v<detail::discriminator<static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) + 1>::type, std::size_t>);
		static_assert(std::is_same_v<detail::discriminator<std::numeric_limits<std::size_t>::max()>::type, std::size_t>);
	};
}

using stdex::variant;

auto main() -> int
{
	/* constructing: */
	{
		const variant<int, float> a { };
		assert(a.index() == decltype(a)::index_of<int>());
		assert(a.index() == 0);

		const variant<short, int, float> b { };
		assert(b.index() == decltype(b)::index_of<short>());
		assert(b.index() == 0);

		static std::int8_t val {1};

		struct dummy
		{
			std::int8_t field;

			dummy()
			{
				++val;
				field = 20;
			}

			~dummy()
			{
				val = 123;
			}
		};
		static_assert(sizeof(dummy) == 1);

		struct dummy2
		{
			alignas(variant<dummy, char16_t>::detail::max_align) std::array<std::byte, variant<dummy, char16_t>::detail::max_size> a;
			std::uint8_t                                                                                                           b;
		};

		std::cout << sizeof(dummy2) << '\n';
		std::cout << alignof(dummy2) << '\n';

		static_assert(sizeof(variant<dummy, char16_t>::detail::std_tuple) == 4);
		static_assert(std::is_same_v<variant<dummy, char16_t>::discriminator_v, std::uint8_t>);
		static_assert(variant<dummy, char16_t>::detail::max_align == 2);
		static_assert(variant<dummy, char16_t>::detail::max_size == 2);
		static_assert(sizeof(variant<dummy, char16_t>) == 4);

		std::cout << sizeof(variant<dummy, char16_t>) << '\n';
		std::cout << alignof(variant<dummy, char16_t>) << '\n';

		{
			const variant<dummy, char16_t> c { };
			assert(c.index() == 0);
			assert(val == 2);
		}
		assert(val == 123);

		const variant<int, float, long> d { };
		assert(d.index() == 0);
		assert(d.contains<int>());
		assert(d.contains<int>(0));
		assert(d.get<int>().has_value());
		assert(d.get<int>().value() == 0);
		assert(!d.get<float>().has_value());
		assert(!d.get<long>().has_value());
		assert(d.get_or_default<int>() == 0);
		assert(d.get_or_default<float>() == 0.F);
		assert(d.get_or_default<long>() == 0);
		assert(d.get_or<int>(2) == 0);
		assert(d.get_or<float>(3.1F) == 3.1F);
		assert(d.get_or<long>(-100) == -100);
		assert(d.get_or_invoke<int>([]
			{
			++val;
			return 2;
			}) == 0);
		assert(d.get_or_invoke<float>([]
			{
			++val;
			return 2.5F;
			}) == 2.5F);
		assert(d.get_or_invoke<long>([]
			{
			++val;
			return -10;
			}) == -10);
		assert(val == 125);
	}

	std::cout << "All OK!\n";

	return 0;
}
