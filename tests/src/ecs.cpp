#include "gtest/gtest.h"
#include "psl/ecs/details/staged_sparse_component_array.h"

using namespace psl::ecs;
using namespace psl::ecs::details;

template <typename T>
struct staged_sparse_component_array_fixture : public ::testing::Test
{
	staged_sparse_component_array<T> data;
};

using trivial_staged_sparse_component_array = staged_sparse_component_array_fixture<int>;
using empty_staged_sparse_component_array   = staged_sparse_component_array_fixture<psl::ecs::flag_t>;

TEST_F(trivial_staged_sparse_component_array, ops)
{
	EXPECT_EQ(data.size(), 0);
	data.emplace(0, 10);
	EXPECT_EQ(data.size(), 0);
	EXPECT_EQ(data.size<stage::ADDED>(), 1);

	data.promote();
	EXPECT_EQ(data.size(), 1);
	EXPECT_EQ(data.size<stage::ADDED>(), 0);

	EXPECT_EQ(data.at(0), 10);
	EXPECT_EQ(data.emplace(10000, 3), 3);
	EXPECT_EQ(data.size(), 1);
	EXPECT_EQ(data.size<stage::ADDED>(), 1);

	EXPECT_EQ(data.at(0), 10);
	EXPECT_EQ(data.at(10000), 3);

	ASSERT_TRUE(data.contains(0));
	data.erase(0);
	ASSERT_FALSE(data.contains(0));
}

TEST_F(empty_staged_sparse_component_array, ops)
{
	EXPECT_EQ(data.size(), 0);
	data.emplace(0);
	EXPECT_EQ(data.size(), 0);
	EXPECT_EQ(data.size<stage::ADDED>(), 1);

	data.promote();
	EXPECT_EQ(data.size(), 1);
	EXPECT_EQ(data.size<stage::ADDED>(), 0);

	data.emplace(10000);
	EXPECT_EQ(data.size(), 1);
	EXPECT_EQ(data.size<stage::ADDED>(), 1);

	ASSERT_TRUE(data.contains(0));
	data.erase(0);
	ASSERT_FALSE(data.contains(0));
}