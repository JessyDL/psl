#include <litmus/expect.hpp>
#include <litmus/section.hpp>
#include <litmus/suite.hpp>

#include <psl/optional.hpp>
#include <tests/types.hpp>

using namespace psl;
using namespace litmus;

template <typename T>
psl::optional<T> test_fn(psl::optional<T> value, int& success_value, int& fail_value, bool success) {
	if(value && success) {
		++success_value;
		++value.value();
		return value;
	}
	++fail_value;
	return value;
}

template <typename T>
T test_fn2(psl::optional<T> value, int& success_value, int& fail_value, bool success) {
	if(success) {
		++success_value;
		++value.value();
		return value.value();
	}
	++fail_value;
	return value.value();
}


template <typename T>
T test_fn3(T value, int& success_value, int& fail_value, bool success) {
	if(success) {
		++success_value;
		++value;
		return value;
	}
	++fail_value;
	return value;
}

auto optional_test0 = suite<"optional", "psl", "psl::optional">().templates<tpack<int>>() = []<typename T>() {
	static_assert(psl::IsOptional<psl::optional<T>>);
	section<"empty state">() = [] {
		psl::optional<T> opt = psl::nullopt;
		require(opt.has_value()) == false;
		opt = {5};
		require(opt.has_value()) == true;
		opt.reset();
		require(opt.has_value()) == false;
		opt = {5};
		require(opt.has_value()) == true;
		opt = {};
		require(opt.has_value()) == false;
	};
	section<"basic_continuation">() = [] {
		psl::optional<T> opt = 5;
		int success_value	 = 0;
		int fail_value		 = 0;

		auto res = opt.then(test_fn<T>, success_value, fail_value, true)
					 .then(test_fn2<T>, success_value, fail_value, true)
					 .then(test_fn2<T>, success_value, fail_value, false)
					 .then(test_fn3<T>, success_value, fail_value, true);
		expect(success_value) == 3;
		expect(fail_value) == 1;
		expect(res.value()) == 8;
	};
};