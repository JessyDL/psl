#include <catch2/catch_template_test_macros.hpp>
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

TEMPLATE_TEST_CASE("expected suite", "[psl::expected]", complex_destruct<true>, complex_destruct<false>, int,
				   copy_only_object, requires_init)
{
	using T = TestType;
	SECTION("constraints") { REQUIRE(psl::IsExpected<psl::expected<T>>); }
	SECTION("basic_success")
	{
		T seed{0};
		{
			int success_val = 0;
			int error_val	= 0;
			auto total		= test_fn<T>(seed, success_val, error_val, true);
			REQUIRE(total);
			if constexpr(is_complex_destruct_v<T>)
			{
				REQUIRE(seed.references() == 2);
			}
			REQUIRE(static_cast<int>(total.consume()) == success_val);
			REQUIRE_THROWS(total.consume());
			if constexpr(is_complex_destruct_v<T>)
			{
				REQUIRE(seed.references() == 1);
			}
			REQUIRE(1 == success_val);
			REQUIRE(error_val == 0);
		}
		if constexpr(is_complex_destruct_v<T>)
		{
			REQUIRE(seed.references() == 1);
		}
	}
	SECTION("basic_error")
	{
		T seed{0};
		{
			int success_val = 0;
			int error_val	= 0;
			auto total		= test_fn<T>(seed, success_val, error_val, false);
			if constexpr(is_complex_destruct_v<T>)
			{
				REQUIRE(seed.references() == 1);
			}
			REQUIRE_FALSE(total);
			REQUIRE_THROWS(total.consume());
			if constexpr(is_complex_destruct_v<T>)
			{
				REQUIRE(seed.references() == 1);
			}
			REQUIRE(1 == error_val);
			REQUIRE(success_val == 0);
		}
		if constexpr(is_complex_destruct_v<T>)
		{
			REQUIRE(seed.references() == 1);
		}
	}
	SECTION("basic_continuation")
	{
		T seed{0};
		{
			int success_val = 0;
			int error_val	= 0;
			auto total		= test_fn<T>(seed, success_val, error_val, true)  // 1 : 0
							 .then(test_fn<T>, success_val, error_val, true)  // 2 : 0
							 .then(test_fn<T>, success_val, error_val, true); // 3 : 0

			REQUIRE(total);
			if constexpr(is_complex_destruct_v<T>)
			{
				REQUIRE(seed.references() == 2);
			}

			REQUIRE(static_cast<int>(total.consume()) == success_val);
			REQUIRE_THROWS(total.consume());
			REQUIRE_FALSE(total);
			REQUIRE(error_val == 0);
		}
	}
	SECTION("continuation_with_recover")
	{
		int error_fn_runs = 0;

		int recovery = -1;
		T seed{0};
		int success_val = 0;
		int error_val	= 0;
		auto error_fn	= [&success_val, &error_val, &error_fn_runs]([[maybe_unused]] auto error) { ++error_fn_runs; };
		psl::expected<T> total{test_fn<T>(seed, success_val, error_val, true)		// 1 : 0
								   .then(test_fn<T>, success_val, error_val, true)	// 2 : 0
								   .then(test_fn<T>, success_val, error_val, true)	// 3 : 0
								   .then(test_fn<T>, success_val, error_val, false) // 3 : 1
								   .then(test_fn<T>, success_val, error_val, true)	// did not run
								   .on_error(error_fn)								// increments error_fn_runs to 1
								   .recover(recovery)};								// recover

		REQUIRE(total);
		REQUIRE(total.consume() == recovery);
		REQUIRE_FALSE(total);
		REQUIRE_THROWS(total.consume());
		REQUIRE(error_val == 1);
		REQUIRE(error_fn_runs == 1);
		REQUIRE(success_val == 3);
	}
	SECTION("complex_continuation")
	{
		T seed{0};
		int error_fn_runs = 0;
		int success_val	  = 0;
		int error_val	  = 0;
		auto error_fn = [&success_val, &error_val, &error_fn_runs]([[maybe_unused]] auto error) { ++error_fn_runs; };

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
		REQUIRE_FALSE(total);
		REQUIRE_THROWS(total.consume());

		if constexpr(is_complex_destruct_v<T>)
		{
			REQUIRE(seed.references() == 1);
		}

		REQUIRE(success_val == 5);
		REQUIRE(error_val == 2);	 // << "error value should run twice, ignoring success untill recovery";
		REQUIRE(error_fn_runs == 3); // << "error function should have run 3 times";
	}
}