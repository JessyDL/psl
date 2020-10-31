#include <catch2/catch_template_test_macros.hpp>
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

TEMPLATE_TEST_CASE("optional suite", "[psl::optional]", int, copy_only_object)
{
	using T = TestType;
	SECTION("constraints") { STATIC_REQUIRE(psl::IsOptional<psl::optional<T>>); }
	SECTION("empty_state")
	{
		psl::optional<T> opt = psl::nullopt;
		REQUIRE_FALSE(opt);
		opt = {5};
		REQUIRE(opt);
		opt.reset();
		REQUIRE_FALSE(opt);
		opt = {5};
		REQUIRE(opt);
		opt = {};
		REQUIRE_FALSE(opt);
	}
	SECTION("basic_continuation")
	{
		psl::optional<T> opt = 5;
		int success_value	 = 0;
		int fail_value		 = 0;

		auto res = opt.then(test_fn<T>, success_value, fail_value, true)
					   .then(test_fn2<T>, success_value, fail_value, true)
					   .then(test_fn2<T>, success_value, fail_value, false)
					   .then(test_fn3<T>, success_value, fail_value, true);
		REQUIRE(success_value == 3);
		REQUIRE(fail_value == 1);
		REQUIRE(res.value() == 8);
	}
}