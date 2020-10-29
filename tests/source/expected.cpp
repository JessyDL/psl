#include <gtest/gtest.h>
#include <psl/expected.hpp>
#include <tests/types.hpp>

template <typename T>
psl::expected<T> test_fn(T count, int& success_val, int& error_val, bool success)
{
	if(success)
	{
		++success_val;
		++count;
		return count;
	}
	++error_val;
	return std::error_code{200, std::generic_category()};
}

template <typename T>
class expected : public ::testing::Test
{};
using expected_test_types =
	::testing::Types<complex_destruct<true>, complex_destruct<false>, int, copy_only_object, requires_init>;

TYPED_TEST_CASE(expected, expected_test_types);

TYPED_TEST(expected, expectations) { EXPECT_TRUE(psl::IsExpected<psl::expected<TypeParam>>); }

TYPED_TEST(expected, basic_success)
{
	using T = TypeParam;
	{
		T seed{0};
		{
			int success_val = 0;
			int error_val   = 0;
			auto total		= test_fn<T>(seed, success_val, error_val, true);
			EXPECT_TRUE(total);
			if constexpr(is_complex_destruct_v<T>)
			{
				EXPECT_EQ(seed.references(), 2);
			}
			EXPECT_EQ(static_cast<int>(total.consume()), success_val);
			EXPECT_ANY_THROW(total.consume());
			if constexpr(is_complex_destruct_v<T>)
			{
				EXPECT_EQ(seed.references(), 1);
			}
			EXPECT_EQ(1, success_val);
			EXPECT_EQ(error_val, 0);
		}
		if constexpr(is_complex_destruct_v<T>)
		{
			EXPECT_EQ(seed.references(), 1);
		}
	}
}

TYPED_TEST(expected, basic_error)
{
	using T = TypeParam;

	T seed{0};
	{
		int success_val = 0;
		int error_val   = 0;
		auto total		= test_fn<T>(seed, success_val, error_val, false);
		if constexpr(is_complex_destruct_v<T>)
		{
			EXPECT_EQ(seed.references(), 1);
		}
		EXPECT_FALSE(total);
		EXPECT_ANY_THROW(total.consume());
		if constexpr(is_complex_destruct_v<T>)
		{
			EXPECT_EQ(seed.references(), 1);
		}
		EXPECT_EQ(1, error_val);
		EXPECT_EQ(success_val, 0);
	}
	if constexpr(is_complex_destruct_v<T>)
	{
		EXPECT_EQ(seed.references(), 1);
	}
}

TYPED_TEST(expected, basic_continuation)
{
	using T = TypeParam;

	T seed{0};
	{
		int success_val = 0;
		int error_val   = 0;
		auto total		= test_fn<T>(seed, success_val, error_val, true)  // 1 : 0
						 .then(test_fn<T>, success_val, error_val, true)  // 2 : 0
						 .then(test_fn<T>, success_val, error_val, true); // 3 : 0

		EXPECT_TRUE(total);
		if constexpr(is_complex_destruct_v<T>)
		{
			EXPECT_EQ(seed.references(), 2);
		}

		EXPECT_EQ(static_cast<int>(total.consume()), success_val);
		EXPECT_ANY_THROW(total.consume());
		EXPECT_FALSE(total);
		EXPECT_EQ(error_val, 0);
	}
}

TYPED_TEST(expected, continuation_with_recover)
{
	using T			  = TypeParam;
	int error_fn_runs = 0;

	int recovery = -1;
	T seed{0};
	int success_val = 0;
	int error_val   = 0;
	auto error_fn   = [&success_val, &error_val, &error_fn_runs]([[maybe_unused]] auto error) { ++error_fn_runs; };
	psl::expected<T> total{test_fn<T>(seed, success_val, error_val, true)		// 1 : 0
							   .then(test_fn<T>, success_val, error_val, true)  // 2 : 0
							   .then(test_fn<T>, success_val, error_val, true)  // 3 : 0
							   .then(test_fn<T>, success_val, error_val, false) // 3 : 1
							   .then(test_fn<T>, success_val, error_val, true)  // did not run
							   .on_error(error_fn)								// increments error_fn_runs to 1
							   .recover(recovery)};								// recover

	EXPECT_TRUE(total);
	EXPECT_EQ(total.consume(), recovery);
	EXPECT_FALSE(total);
	EXPECT_ANY_THROW(total.consume());
	EXPECT_EQ(error_val, 1);
	EXPECT_EQ(error_fn_runs, 1);
	EXPECT_EQ(success_val, 3);
}

TYPED_TEST(expected, complex_continuation)
{
	using T = TypeParam;
	T seed{0};
	int error_fn_runs = 0;
	int success_val   = 0;
	int error_val	 = 0;
	auto error_fn	 = [&success_val, &error_val, &error_fn_runs]([[maybe_unused]] auto error) { ++error_fn_runs; };

	auto total = test_fn<T>(seed, success_val, error_val, true)		  // 1 : 0
					 .then(test_fn<T>, success_val, error_val, true)  // 2 : 0
					 .then(test_fn<T>, success_val, error_val, true)  // 3 : 0
					 .then(test_fn<T>, success_val, error_val, false) // 3 : 1
					 .then(test_fn<T>, success_val, error_val, true)  // did not run
					 .on_error(error_fn)							  // increments error_fn_runs to 1
					 .recover(seed)									  // recover
					 .then(test_fn<T>, success_val, error_val, true)  // 4 : 1
					 .then(test_fn<T>, success_val, error_val, true)  // 5 : 1
					 .then(test_fn<T>, success_val, error_val, false) // 5 : 2
					 .then(test_fn<T>, success_val, error_val, true)  // did not run
					 .then(test_fn<T>, success_val, error_val, true)  // did not run
					 .on_error(error_fn)							  // increments error_fn_runs to 2
					 .on_error(error_fn);							  // increments error_fn_runs to 3
	EXPECT_FALSE(total);
	EXPECT_ANY_THROW(total.consume());

	if constexpr(is_complex_destruct_v<T>)
	{
		EXPECT_EQ(seed.references(), 1);
	}

	EXPECT_EQ(success_val, 5);
	EXPECT_EQ(error_val, 2) << "error value should run twice, ignoring success untill recovery";
	EXPECT_EQ(error_fn_runs, 3) << "error function should have run 3 times";
}