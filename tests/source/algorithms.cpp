#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <psl/algorithms.hpp>
#include <tests/types.hpp>
#include <fmt/format.h>

using namespace psl;

TEMPLATE_TEST_CASE("align_to", "[psl::algorithms]", i8, i16, i32, i64, ui8, ui16, ui32, ui64)
{
	using T = TestType;
	T value{};
	T alignment{};
	if constexpr(IsSignedIntegral<T>)
	{
		value	  = static_cast<T>(GENERATE(-6, -2, -3, 1, 0, 1, 2, 4, 6, 8, 9));
		alignment = static_cast<T>(GENERATE(-2, -4, 2, 4, 6, 3));
	}
	else
	{
		value	  = static_cast<T>(GENERATE(0, 1, 2, 4, 6, 8, 9));
		alignment = static_cast<T>(GENERATE(2, 4, 6, 3));
	}

	auto res = align_to(value, alignment);

	STATIC_REQUIRE(std::is_same_v<decltype(res), T>);
	INFO(fmt::format("value = {} alignment = {} result = {}", value, alignment, res));
	CHECK((res % alignment == 0));
	CHECK((res >= value));
	CHECK((value % alignment != 0 || res == value));
}

TEMPLATE_TEST_CASE("ralign_to", "[psl::algorithms]", i8, i16, i32, i64, ui8, ui16, ui32, ui64)
{
	using T = TestType;
	T value{};
	T alignment{};
	if constexpr(IsSignedIntegral<T>)
	{
		value	  = static_cast<T>(GENERATE(-6, -2, -3, 1, 0, 1, 2, 4, 6, 8, 9));
		alignment = static_cast<T>(GENERATE(-2, -4, 2, 4, 6, 3));
	}
	else
	{
		value	  = static_cast<T>(GENERATE(0, 1, 2, 4, 6, 8, 9));
		alignment = static_cast<T>(GENERATE(2, 4, 6, 3));
	}

	auto res = ralign_to(value, alignment);
	STATIC_REQUIRE(std::is_same_v<decltype(res), T>);
	INFO(fmt::format("value = {} alignment = {} result = {}", value, alignment, res));

	CHECK((res % alignment == 0));
	CHECK((res <= value));
	CHECK((value % alignment != 0 || res == value));
}

TEMPLATE_TEST_CASE("greatest_contained_count", "[psl::algorithms]", ui8, ui16, ui32, ui64)
{
	using T = TestType;
	T value{};
	T size{};
	value = static_cast<T>(GENERATE(1, 2, 4, 6, 8, 9));
	size  = static_cast<T>(GENERATE(2, 4, 6, 3, 12));

	auto res = greatest_contained_count(value, size);
	STATIC_REQUIRE(std::is_same_v<decltype(res), T>);
	INFO(fmt::format("value = {} size = {} result = {}", value, size, res));

	CHECK((res * value <= size));
	CHECK((size - res * value < value));
}