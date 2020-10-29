#include <gtest/gtest.h>
#include <psl/optional.hpp>
#include <tests/types.hpp>

template <typename T>
psl::optional<T> test_fn(psl::optional<T> value, int& success_value, int& fail_value, bool success)
{
	if(value && success)
	{
		++success_value;
		++value.value();
		return value;
	}
	++fail_value;
	return value;
}

template <typename T>
T test_fn2(psl::optional<T> value, int& success_value, int& fail_value, bool success)
{
	if(success)
	{
		++success_value;
		++value.value();
		return value.value();
	}
	++fail_value;
	return value.value();
}


template <typename T>
T test_fn3(T value, int& success_value, int& fail_value, bool success)
{
	if(success)
	{
		++success_value;
		++value;
		return value;
	}
	++fail_value;
	return value;
}

template <typename T>
class optional : public ::testing::Test
{};
using optional_test_types = ::testing::Types<int>;

TYPED_TEST_CASE(optional, optional_test_types);

TYPED_TEST(optional, expectations) { EXPECT_TRUE(psl::IsOptional<psl::optional<TypeParam>>); }
TYPED_TEST(optional, empty_state)
{
	psl::optional<TypeParam> opt = psl::nullopt;
	EXPECT_FALSE(opt) << "expected empty";
	opt = {5};
	EXPECT_TRUE(opt);
	opt.reset();
	EXPECT_FALSE(opt) << "expected empty";
	opt = {5};
	EXPECT_TRUE(opt);
	opt = {};
	EXPECT_FALSE(opt) << "expected empty";
}

TYPED_TEST(optional, basic_continuation)
{
	psl::optional<TypeParam> opt = 5;

	int success_value = 0;
	int fail_value	= 0;

	auto res = opt.then(test_fn<TypeParam>, success_value, fail_value, true)
				   .then(test_fn2<TypeParam>, success_value, fail_value, true)
				   .then(test_fn2<TypeParam>, success_value, fail_value, false)
				   .then(test_fn3<TypeParam>, success_value, fail_value, true);
	EXPECT_EQ(success_value, 3);
	EXPECT_EQ(fail_value, 1);
	EXPECT_EQ(res.value(), 8);
}