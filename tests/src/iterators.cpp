#include "psl/iterators.h"
#include "gtest/gtest.h"
#include <array>

using contiguous_range_iterator = psl::contiguous_range_iterator<int>;

TEST(contiguous_range_iterator, operations)
{
	std::array data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	contiguous_range_iterator begin{&data[0]};
	contiguous_range_iterator end{&data[0] + data.size()};

	auto data_it = data.begin();
	for(auto it = begin; it != end; ++it, ++data_it)
	{
		ASSERT_EQ(*it, *data_it) << "data is invalid";
	}

	// comparison operators
	ASSERT_EQ(begin + data.size(), end);
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

	ASSERT_EQ(end - begin, data.size());
}