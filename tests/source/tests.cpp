#include <gtest/gtest.h>

TEST(QuickTest, Add)
{
	ASSERT_EQ(2, 1 + 1);
	ASSERT_EQ(5, 3 + 2);
	ASSERT_EQ(10, 7 + 3);
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	::testing::FLAGS_gtest_death_test_style = "fast";
	return RUN_ALL_TESTS();
}
