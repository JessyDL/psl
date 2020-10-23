#include <gtest/gtest.h>
#include <psl/iterators.hpp>
#include <array>
#include <numeric>
#include <psl/types.hpp>
using contiguous_range_iterator					= psl::contiguous_range_iterator<int, 1>;
using reverse_contiguous_range_iterator			= psl::contiguous_range_iterator<int, -1>;
using strided_contiguous_range_iterator			= psl::contiguous_range_iterator<int, 3>;
using reverse_strided_contiguous_range_iterator = psl::contiguous_range_iterator<int, -3>;

void test(auto begin, auto end)
{
	ASSERT_TRUE(begin.is_valid_pair(end));
	auto stride  = begin.stride();
	auto data_it = begin.ptr();
	for(auto it = begin; it != end; ++it, data_it += stride)
	{
		ASSERT_EQ(*it, *data_it) << "data is invalid";
	}
	data_it = begin.ptr();
	for(auto i = 0u; begin + i != end; ++i, data_it += stride)
	{
		ASSERT_EQ(begin[i], *data_it) << "data is invalid";
	}

	// comparison operators
	ASSERT_NE(begin, end);

	ASSERT_GE(begin, begin);
	ASSERT_GE(end, begin);

	ASSERT_LE(begin, end);
	ASSERT_LE(end, end);

	ASSERT_LT(begin, end);
	ASSERT_GT(end, begin);

	// arithmetic
	{
		auto it  = begin++;
		auto it2 = begin;
		ASSERT_EQ(it, ++it2);
		ASSERT_EQ(begin, --it2);
		ASSERT_EQ(it, it2++);
		ASSERT_EQ(begin, it2);
		ASSERT_EQ(it--, begin);
	}

	// ASSERT_EQ(end - begin, data.size());
}

TEST(contiguous_range_iterator, operations)
{
	std::array data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	contiguous_range_iterator begin{&data[0]};
	contiguous_range_iterator end{&data[0] + data.size()};

	ASSERT_EQ(begin + data.size(), end);
	ASSERT_EQ(end - begin, data.size());
	test(begin, end);
}

TEST(reverse_contiguous_range_iterator, operations)
{
	std::array data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	reverse_contiguous_range_iterator end{&data[0]};
	reverse_contiguous_range_iterator begin{&data[0] + data.size()};

	ASSERT_EQ(begin + data.size(), end);
	ASSERT_EQ(end - begin, data.size());
	test(begin, end);
}

TEST(strided_contiguous_range_iterator, operations)
{
	std::array data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	strided_contiguous_range_iterator begin{&data[0]};
	constexpr auto max_elements = (data.size() - (data.size() % begin.abs_stride())) / begin.abs_stride();
	strided_contiguous_range_iterator end{&data[0] + begin.abs_stride() * max_elements};
	test(begin, end);

	ASSERT_EQ(begin + max_elements, end);
	ASSERT_EQ(end - begin, max_elements);

	begin = strided_contiguous_range_iterator{&data[1]};
	ASSERT_FALSE(begin.is_valid_pair(end));
}

TEST(reverse_strided_contiguous_range_iterator, operations)
{
	std::array data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	reverse_strided_contiguous_range_iterator end{&data[0]};
	constexpr auto max_elements = (data.size() - (data.size() % end.abs_stride())) / end.abs_stride();
	reverse_strided_contiguous_range_iterator begin{&data[0] + end.abs_stride() * max_elements};
	test(begin, end);

	ASSERT_EQ(begin + max_elements, end);
	ASSERT_EQ(end - begin, max_elements);

	begin = reverse_strided_contiguous_range_iterator{&data[1]};
	ASSERT_FALSE(begin.is_valid_pair(end));
}