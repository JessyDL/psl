#include <catch2/catch_template_test_macros.hpp>
#include <psl/iterators.hpp>
#include <psl/types.hpp>

void test(auto begin, auto end)
{
	REQUIRE(begin.is_valid_pair(end));
	auto stride	 = begin.stride();
	auto data_it = begin.ptr();
	for(auto it = begin; it != end; ++it, data_it += stride)
	{
		REQUIRE(*it == *data_it); // << "data is invalid";
	}
	data_it = begin.ptr();
	for(auto i = 0u; begin + i != end; ++i, data_it += stride)
	{
		REQUIRE(begin[i] == *data_it); // << "data is invalid";
	}

	// comparison operators
	REQUIRE(begin != end);

	REQUIRE(begin >= begin);
	REQUIRE(end >= begin);

	REQUIRE(begin <= end);
	REQUIRE(end <= end);

	REQUIRE(begin < end);
	REQUIRE(end > begin);

	// arithmetic
	{
		auto it	 = begin++;
		auto it2 = begin;
		REQUIRE(it == ++it2);
		REQUIRE(begin == --it2);
		REQUIRE(it == it2++);
		REQUIRE(begin == it2);
		REQUIRE(it-- == begin);
	}

	// ASSERT_EQ(end - begin, data.size());
}

TEMPLATE_TEST_CASE("contiguous iterator suite", "[psl::contiguous_range_iterator]",
				   (psl::contiguous_range_iterator<int, 1>), (psl::contiguous_range_iterator<int, -1>),
				   (psl::contiguous_range_iterator<int, 3>), (psl::contiguous_range_iterator<int, -3>))
{
	int data[]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	constexpr size_t data_size = sizeof(data) / sizeof(data[0]);
	using iterator			   = TestType;

	constexpr auto max_elements = (data_size - (data_size % iterator::abs_stride())) / iterator::abs_stride();
	bool forwards				= iterator::stride() > 0;

	iterator begin{(forwards) ? &data[0] : &data[0] + iterator::abs_stride() * max_elements};
	iterator end{(!forwards) ? &data[0] : &data[0] + iterator::abs_stride() * max_elements};
	test(begin, end);

	REQUIRE(begin + max_elements == end);
	REQUIRE(end - begin == max_elements);

	if(iterator::abs_stride() > 1)
	{
		REQUIRE_FALSE(iterator{&data[1]}.is_valid_pair(end));
	}
	else
	{
		REQUIRE(iterator{&data[1]}.is_valid_pair(end));
	}
}