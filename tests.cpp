/*
	MIT License

	Copyright (c) 2021 Mario Sieg <pinsrq> mt3000@gmx.de

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
		static_assert(std::is_same_v<variant<bool>::detail::discriminator<std::numeric_limits<std::uint8_t>::max()>::type, std::uint8_t>);
		static_assert(std::is_same_v<variant<bool>::detail::discriminator<std::numeric_limits<std::uint8_t>::max() + 1>::type, std::uint16_t>);
		static_assert(std::is_same_v<variant<bool>::detail::discriminator<std::numeric_limits<std::uint16_t>::max()>::type, std::uint16_t>);
		static_assert(std::is_same_v<variant<bool>::detail::discriminator<std::numeric_limits<std::uint16_t>::max() + 1>::type, std::uint32_t>);
		static_assert(std::is_same_v<variant<bool>::detail::discriminator<std::numeric_limits<std::uint32_t>::max()>::type, std::uint32_t>);
		static_assert(std::is_same_v<variant<bool>::detail::discriminator<static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) + 1>::type, std::size_t>);
		static_assert(std::is_same_v<variant<bool>::detail::discriminator<std::numeric_limits<std::size_t>::max()>::type, std::size_t>);
	};
}

using stdex::variant;

auto main() -> int
{

	/* constructing: */
	{
		constexpr variant<int, float> x{};
		assert(x.index() == decltype(x)::index_of<int>());
		assert(x.index() == 0);

		constexpr variant<short, int, float> y{};
		assert(y.index() == decltype(y)::index_of<short>());
		assert(y.index() == 0);
	}

	std::cout << "All OK!\n";
	
	return 0;
}