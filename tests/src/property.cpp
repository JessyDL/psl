#include "psl/property.h"
#include "gtest/gtest.h"
#include <vector>

TEST(property, generic)
{
	psl::property<char, u8"myname"> c{'1'};
	psl::property<int> i{0};

	ASSERT_NE(c, i);

	psl::property<std::vector<int>> vec{std::initializer_list<int>{1, 2, 3, 4, 5, 6}};

	{
		auto i = 0;
		for(auto element : vec) ASSERT_EQ(++i, element);
		for(i = 0; i < (int)vec->size(); ++i) ASSERT_EQ(i + 1, vec[i]);
		++vec[0];
		ASSERT_EQ(vec[0], 2);
	}

	psl::property<int, u8"heyaa"> i2 = i;

	i2 += 5;
	ASSERT_NE(i, i2);
	i = std::move(i2);


	i = i + i + 5 + i;
	ASSERT_EQ(i, 20);
}