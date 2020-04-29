#include "psl/array.h"
#include "gtest/gtest.h"

class ArrayTest : public ::testing::Test
{
  public:
	void SetUp() override
	{
		_arr1.emplace_back(0);
		_arr1.emplace_back(0);
		_arr2.emplace_back(1);
		_arr2.emplace_back(2);
	}

	psl::array<int> _arr0;
	psl::array<int> _arr1;
	psl::array<int> _arr2;
};

using array = ArrayTest;

TEST_F(array, empty) { EXPECT_TRUE(_arr0.empty()); }

TEST_F(array, size) { EXPECT_EQ(_arr1.size(), 2); }

TEST_F(array, emplace)
{
	auto& res0 = _arr0.emplace_back(0);
	EXPECT_EQ(res0, 0);
	EXPECT_EQ(_arr0.size(), 1);
}

TEST_F(array, resize)
{
	_arr0.resize(10);
	EXPECT_EQ(_arr0.size(), 10);
}