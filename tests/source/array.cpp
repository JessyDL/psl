#include <catch2/catch_test_macros.hpp>
#include <psl/array.hpp>
#include <tests/types.hpp>

using namespace psl;

TEST_CASE("internal storage constraints", "[psl::sbo_storage]")
{
	int results[]{
		sbo_storage<int, 4, sbo_alias<true>>::SBO,	   sbo_storage<int, 4, sbo_alias<false>>::SBO,
		sbo_storage<int, 0, sbo_alias<true>>::SBO,	   sbo_storage<int, 0, sbo_alias<false>>::SBO,
		sbo_storage<int, 1, sbo_alias<true>>::SBO,	   sbo_storage<int, 1, sbo_alias<false>>::SBO,
		sbo_storage<int8_t, 12, sbo_alias<true>>::SBO, sbo_storage<int8_t, 12, sbo_alias<false>>::SBO,
		sbo_storage<int8_t, 0, sbo_alias<true>>::SBO,  sbo_storage<int8_t, 0, sbo_alias<false>>::SBO,
		sbo_storage<int8_t, 1, sbo_alias<true>>::SBO,  sbo_storage<int8_t, 1, sbo_alias<false>>::SBO,
	};
	REQUIRE(results[0] == 4);
	REQUIRE(results[1] == 2);
	REQUIRE(results[2] == 0);
	REQUIRE(results[3] == 0);
	REQUIRE(results[4] == 1);
	REQUIRE(results[5] == 0);
	REQUIRE(results[6] == 12);
	REQUIRE(results[7] == 4);
	REQUIRE(results[8] == 0);
	REQUIRE(results[9] == 0);
	REQUIRE(results[10] == 1);
	REQUIRE(results[11] == 0);
}

// TYPED_TEST_CASE(array_t, array_test_types);

// TYPED_TEST(array_t, expectations)
// {
// 	EXPECT_TRUE(is_array_v<const array<TypeParam>>);
// 	EXPECT_TRUE(IsArray<array<TypeParam>>);
// }

// TYPED_TEST(array_t, emplace)
// {
// 	psl::array<TypeParam> arr{};
// 	EXPECT_TRUE(arr.is_stored_inlined() || arr.sbo_size() == 0);
// 	EXPECT_EQ(arr.size(), 0);
// 	int count = arr.sbo_size();
// 	for(int i = 0; i < count; ++i)
// 	{
// 		arr.emplace_back(i);
// 	}
// 	EXPECT_TRUE(arr.is_stored_inlined() || arr.sbo_size() == 0);
// 	EXPECT_EQ(arr.size(), count);
// 	arr.emplace_back(arr.size());
// 	EXPECT_FALSE(arr.is_stored_inlined());
// 	EXPECT_EQ(arr.size(), count + 1);
// 	for(int i = 0; i < count + 1; ++i)
// 	{
// 		EXPECT_EQ(static_cast<int>(arr[i]), i);
// 	}
// 	arr.erase(arr.begin());
// 	count = arr.size();
// 	for(int i = 0; i < count; ++i)
// 	{
// 		int value = i;
// 		if(i == 0) value = count;
// 		auto res = static_cast<int>(arr[i]);
// 		EXPECT_EQ(res, value);
// 	}
// 	arr.erase(arr.begin() + 1);
// 	count = arr.size();
// 	for(int i = 0; i < count; ++i)
// 	{
// 		int value = i;
// 		if(i == 0) value = count + 1;
// 		if(i == 1) value = count;
// 		auto res = static_cast<int>(arr[i]);
// 		EXPECT_EQ(res, value);
// 	}
// }