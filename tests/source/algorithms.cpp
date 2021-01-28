#include <psl/algorithms.hpp>
#include <tests/types.hpp>

#include <litmus/suite.hpp>
#include <litmus/section.hpp>
#include <litmus/expect.hpp>

#include <litmus/generator/range.hpp>

using namespace psl;

using namespace litmus;
using namespace litmus::generator;

auto align_to_test_fn = []<typename T>(T value, T alignment)
{
	auto res = align_to(value, alignment);

	static_assert(std::is_same_v<decltype(res), T>);
	expect(res % alignment) == T{0};
	expect(res) >= value;
	expect((value % alignment != 0 || res == value)) == true;
};

auto ralign_to_test_fn = []<typename T>(T value, T alignment)
{
	auto res = ralign_to(value, alignment);
	static_assert(std::is_same_v<decltype(res), T>);

	expect(res % alignment) == T{0};
	expect(res) <= value;
	expect((value % alignment != 0 || res == value)) == true;
};


auto test0 = suite<"align_to", "psl", "psl::algorithms">(array<-6, -5, -3, 1, 0, 1, 2, 4, 6, 8, 9>{},
														 array<-2, -4, 2, 4, 6, 3>{})
				 .templates<tpack<i8, i16, i32, i64>>() = align_to_test_fn;

auto test1 = suite<"align_to", "psl", "psl::algorithms">(array<0, 1, 2, 4, 6, 8, 9>{}, array<2, 4, 6, 3>{})
				 .templates<tpack<ui8, ui16, ui32, ui64>>() = align_to_test_fn;

auto test2 = suite<"ralign_to", "psl", "psl::algorithms">(array<-6, -5, -3, 1, 0, 1, 2, 4, 6, 8, 9>{},
														  array<-2, -4, 2, 4, 6, 3>{})
				 .templates<tpack<i8, i16, i32, i64>>() = ralign_to_test_fn;

auto test3 = suite<"ralign_to", "psl", "psl::algorithms">(array<0, 1, 2, 4, 6, 8, 9>{}, array<2, 4, 6, 3>{})
				 .templates<tpack<ui8, ui16, ui32, ui64>>() = ralign_to_test_fn;

auto test4 =
	suite<"greatest_contained_count", "psl", "psl::algorithms">(array<1, 2, 4, 6, 8, 9>{}, array<2, 4, 6, 3, 12>{})
		.templates<tpack<ui8, ui16, ui32, ui64>>() = []<typename T>(T value, T size)
{
	auto res = greatest_contained_count(value, size);
	expect(res * value) <= size;
	expect(size - res * value) < value;
};
