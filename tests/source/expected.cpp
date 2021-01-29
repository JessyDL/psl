#include <psl/expected.hpp>
#include <tests/types.hpp>

#include <litmus/suite.hpp>
#include <litmus/section.hpp>
#include <litmus/expect.hpp>
#include <litmus/generator/range.hpp>

using namespace psl;
using namespace litmus;


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

auto expected_test0 =
	suite<"expected", "psl">()
		.templates<tpack<complex_destruct<true>, complex_destruct<false>, int, copy_only_object, requires_init>>() =
		[]<typename T>()
{
	section<"constraints">() = [&] { expect(psl::IsExpected<psl::expected<T>>) == true; };

	section<"basic_success">() = [&] {
		T seed{0};
		{
			int success_val = 0;
			int error_val   = 0;
			auto total		= test_fn<T>(seed, success_val, error_val, true);
			expect(total.has_value()) == true;
			if constexpr(is_complex_destruct_v<T>)
			{
				expect(seed.references()) == 2;
			}
			expect(static_cast<int>(total.consume())) == success_val;
			expect([&] { total.consume(); }) == throws<>();
			if constexpr(is_complex_destruct_v<T>)
			{
				expect(seed.references()) == 1;
			}
			expect(success_val) == 1;
			expect(error_val) == 0;
		}
		if constexpr(is_complex_destruct_v<T>)
		{
			expect(seed.references()) == 1;
		}
	};

	section<"basic_error">() = [&] {
		T seed{0};
		{
			int success_val = 0;
			int error_val   = 0;
			auto total		= test_fn<T>(seed, success_val, error_val, false);
			if constexpr(is_complex_destruct_v<T>)
			{
				expect(seed.references()) == 1;
			}
			expect(total.has_value()) == false;
			expect([&] { total.consume(); }) == throws<>();
			if constexpr(is_complex_destruct_v<T>)
			{
				expect(seed.references()) == 1;
			}
			expect(1) == error_val;
			expect(success_val) == 0;
		}
		if constexpr(is_complex_destruct_v<T>)
		{
			expect(seed.references()) == 1;
		}
	};

	section<"basic_continuation">() = [&] {
		T seed{0};
		{
			int success_val = 0;
			int error_val   = 0;
			auto total		= test_fn<T>(seed, success_val, error_val, true)  // 1 : 0
							 .then(test_fn<T>, success_val, error_val, true)  // 2 : 0
							 .then(test_fn<T>, success_val, error_val, true); // 3 : 0

			expect(total.has_value()) == true;
			if constexpr(is_complex_destruct_v<T>)
			{
				expect(seed.references()) == 2;
			}

			expect(static_cast<int>(total.consume()) == success_val);
			expect([&] { total.consume(); }) == throws<>();
			expect(total.has_value()) == false;
			expect(error_val) == 0;
		}
	};
	section<"continuation_with_recover">() = [&] {
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

		expect(total.has_value()) == true;
		expect(total.consume()) == recovery;
		expect(total.has_value()) == false;
		expect([&] { total.consume(); }) == throws<>();
		expect(error_val) == 1;
		expect(error_fn_runs) == 1;
		expect(success_val) == 3;
	};
	section<"complex_continuation">() = [&] {
		T seed{0};
		int error_fn_runs = 0;
		int success_val   = 0;
		int error_val	 = 0;
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
		expect(total.has_value()) == false;
		expect([&] { total.consume(); }) == throws<>();

		if constexpr(is_complex_destruct_v<T>)
		{
			expect(seed.references()) == 1;
		}

		expect(success_val) == 5;
		expect(error_val) == 2;		// << "error value should run twice, ignoring success untill recovery";
		expect(error_fn_runs) == 3; // << "error function should have run 3 times";
	};
};
