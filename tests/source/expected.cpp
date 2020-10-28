#include <gtest/gtest.h>
#include <psl/expected.hpp>

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

class move_only_object
{
  public:
	move_only_object() = default;
	move_only_object(int value) : m_Value(value){};
	move_only_object(const move_only_object&) = delete;
	move_only_object& operator=(const move_only_object&) = delete;

	move_only_object(move_only_object&&) noexcept = default;
	move_only_object& operator=(move_only_object&&) noexcept = default;

	operator int() { return m_Value; }

	move_only_object& operator++()
	{
		++m_Value;
		return *this;
	}
	move_only_object operator++(int) = delete;

	bool operator==(int rhs) const noexcept { return m_Value == rhs; }

	int m_Value{0};
};

class copy_only_object
{
	struct disable_move
	{
		disable_move()				 = default;
		disable_move(disable_move&&) = delete;
		disable_move& operator=(disable_move&&) = delete;
	};

  public:
	copy_only_object() noexcept = default;
	copy_only_object(int value) noexcept : m_Value(value) {}
	copy_only_object(const copy_only_object& other) : m_Value(other.m_Value){};
	copy_only_object& operator=(const copy_only_object& other) noexcept
	{
		if(this != &other) m_Value = other.m_Value;
		return *this;
	};

	operator int() { return m_Value; }

	copy_only_object& operator++()
	{
		++m_Value;
		return *this;
	}
	copy_only_object operator++(int)
	{
		auto copy{*this};
		return ++copy;
	};

	bool operator==(int rhs) const noexcept { return m_Value == rhs; }

	int m_Value{0};
	[[no_unique_address]] disable_move beh{};
};

template <typename T>
class foo
{
  public:
	template <typename... Args>
	constexpr foo(Args&&... args) : value(std::forward<Args>(args)...)
	{}

	T value;
};

template <typename T>
class expected : public ::testing::Test
{};
using expected_test_types = ::testing::Types<int, move_only_object, copy_only_object>;

TYPED_TEST_CASE(expected, expected_test_types);

TYPED_TEST(expected, expectations) { EXPECT_TRUE(psl::IsExpected<psl::expected<TypeParam>>); }

TYPED_TEST(expected, basic_continuation)
{
	using T = TypeParam;


	int success_val = 0;
	int error_val   = 0;
	auto total		= test_fn<T>(0, success_val, error_val, true)	 // 1 : 0
					 .then(test_fn<T>, success_val, error_val, true)  // 2 : 0
					 .then(test_fn<T>, success_val, error_val, true); // 3 : 0

	EXPECT_TRUE(total);
	EXPECT_EQ(total.consume(), success_val);
	EXPECT_FALSE(total);
	EXPECT_EQ(error_val, 0);
}

TYPED_TEST(expected, continuation_with_recover)
{
	using T			  = TypeParam;
	int error_fn_runs = 0;

	int success_val = 0;
	int error_val   = 0;
	auto error_fn   = [&success_val, &error_val, &error_fn_runs]([[maybe_unused]] auto error) { ++error_fn_runs; };
	psl::expected<T> total{test_fn<T>(0, success_val, error_val, true)			// 1 : 0
							   .then(test_fn<T>, success_val, error_val, true)  // 2 : 0
							   .then(test_fn<T>, success_val, error_val, true)  // 3 : 0
							   .then(test_fn<T>, success_val, error_val, false) // 3 : 1
							   .then(test_fn<T>, success_val, error_val, true)  // did not run
							   .on_error(error_fn)								// increments error_fn_runs to 1
							   .recover(-1)};									// recover

	EXPECT_TRUE(total);
	EXPECT_EQ(total.consume(), -1);
	EXPECT_FALSE(total);
	EXPECT_EQ(error_val, 1);
	EXPECT_EQ(error_fn_runs, 1);
	EXPECT_EQ(success_val, 3);
}

TYPED_TEST(expected, complex_continuation)
{
	using T			  = TypeParam;
	int error_fn_runs = 0;
	int success_val   = 0;
	int error_val	 = 0;
	auto error_fn	 = [&success_val, &error_val, &error_fn_runs]([[maybe_unused]] auto error) { ++error_fn_runs; };

	auto total = test_fn<T>(0, success_val, error_val, true)		  // 1 : 0
					 .then(test_fn<T>, success_val, error_val, true)  // 2 : 0
					 .then(test_fn<T>, success_val, error_val, true)  // 3 : 0
					 .then(test_fn<T>, success_val, error_val, false) // 3 : 1
					 .then(test_fn<T>, success_val, error_val, true)  // did not run
					 .on_error(error_fn)							  // increments error_fn_runs to 1
					 .recover(success_val)							  // recover
					 .then(test_fn<T>, success_val, error_val, true)  // 4 : 1
					 .then(test_fn<T>, success_val, error_val, true)  // 5 : 1
					 .then(test_fn<T>, success_val, error_val, false) // 5 : 2
					 .then(test_fn<T>, success_val, error_val, true)  // did not run
					 .then(test_fn<T>, success_val, error_val, true)  // did not run
					 .on_error(error_fn)							  // increments error_fn_runs to 2
					 .on_error(error_fn);							  // increments error_fn_runs to 3
	EXPECT_FALSE(total);
	EXPECT_EQ(success_val, 5);
	EXPECT_EQ(error_val, 2) << "error value should run twice, ignoring success untill recovery";
	EXPECT_EQ(error_fn_runs, 3) << "error function should have run 3 times";
}