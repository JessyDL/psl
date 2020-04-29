#include "psl/algorithms.h"
#include "gtest/gtest.h"

using namespace psl;

TEST(algorithms, align_to)
{
	EXPECT_EQ(align_to(1, 8), 8);
	EXPECT_EQ(align_to(1, 16), 16);
	EXPECT_EQ(align_to(6, 8), 8);
	EXPECT_EQ(align_to(22, 8), 24);
}

TEST(algorithms, ralign_to)
{
	EXPECT_EQ(ralign_to(1, 8), 0);
	EXPECT_EQ(ralign_to(15, 16), 0);
	EXPECT_EQ(ralign_to(16, 16), 16);
	EXPECT_EQ(ralign_to(10, 8), 8);
	EXPECT_EQ(ralign_to(22, 8), 16);
}