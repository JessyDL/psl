#include <gtest/gtest.h>
#include <psl/array.hpp>
#include <tests/types.hpp>

using namespace psl;

template <typename T>
class array_t : public ::testing::Test
{};
using array_test_types =
	::testing::Types<complex_destruct<true>, complex_destruct<false>, int, copy_only_object, requires_init>;


TEST(sbo_storage, size_test)
{
	int results[]{
		sbo_storage<int, 4, sbo_alias::yes>::SBO,	 sbo_storage<int, 4, sbo_alias::no>::SBO,
		sbo_storage<int, 0, sbo_alias::yes>::SBO,	 sbo_storage<int, 0, sbo_alias::no>::SBO,
		sbo_storage<int, 1, sbo_alias::yes>::SBO,	 sbo_storage<int, 1, sbo_alias::no>::SBO,
		sbo_storage<int8_t, 12, sbo_alias::yes>::SBO, sbo_storage<int8_t, 12, sbo_alias::no>::SBO,
		sbo_storage<int8_t, 0, sbo_alias::yes>::SBO,  sbo_storage<int8_t, 0, sbo_alias::no>::SBO,
		sbo_storage<int8_t, 1, sbo_alias::yes>::SBO,  sbo_storage<int8_t, 1, sbo_alias::no>::SBO,
	};
	EXPECT_EQ(results[0], 4);
	EXPECT_EQ(results[1], 2);
	EXPECT_EQ(results[2], 0);
	EXPECT_EQ(results[3], 0);
	EXPECT_EQ(results[4], 1);
	EXPECT_EQ(results[5], 0);
	EXPECT_EQ(results[6], 12);
	EXPECT_EQ(results[7], 4);
	EXPECT_EQ(results[8], 0);
	EXPECT_EQ(results[9], 0);
	EXPECT_EQ(results[10], 1);
	EXPECT_EQ(results[11], 0);
}

TYPED_TEST_CASE(array_t, array_test_types);

TYPED_TEST(array_t, expectations)
{
	EXPECT_TRUE(is_array_v<const array<TypeParam>>);
	EXPECT_TRUE(IsArray<array<TypeParam>>);
}

namespace psl
{
	class test
	{
	  public:
		int item;
	};
} // namespace psl

TYPED_TEST(array_t, emplace)
{
	[[maybe_unused]] test t{};
	array<TypeParam> arr{};
	EXPECT_TRUE(arr.is_stored_inlined() || arr.sbo_size() == 0);
	EXPECT_EQ(arr.size(), 0);
	int count = arr.sbo_size();
	for(int i = 0; i < count; ++i)
	{
		arr.emplace_back(i);
	}
	EXPECT_TRUE(arr.is_stored_inlined() || arr.sbo_size() == 0);
	EXPECT_EQ(arr.size(), count);
	arr.emplace_back(arr.size());
	EXPECT_FALSE(arr.is_stored_inlined());
	EXPECT_EQ(arr.size(), count + 1);
	for(int i = 0; i < count + 1; ++i)
	{
		EXPECT_EQ(static_cast<int>(arr[i]), i);
	}
	arr.erase(arr.begin());
	count = arr.size();
	for(int i = 0; i < count; ++i)
	{
		int value = i;
		if(i == 0) value = count;
		auto res = static_cast<int>(arr[i]);
		EXPECT_EQ(res, value);
	}
	arr.erase(arr.begin() + 1);
	count = arr.size();
	for(int i = 0; i < count; ++i)
	{
		int value = i;
		if(i == 0) value = count + 1;
		if(i == 1) value = count;
		auto res = static_cast<int>(arr[i]);
		EXPECT_EQ(res, value);
	}
}