#include <psl/random.hpp>

#include <litmus/expect.hpp>
#include <litmus/section.hpp>
#include <litmus/suite.hpp>

using namespace litmus;
using namespace psl;


auto rand_test0 = suite<"random", "psl">().templates<tpack<int, float, double, size_t>>() = []<typename T>() {
	for(auto i = 0u; i < 10u; ++i) {
		auto res = random_range(T {0}, T {10});
		static_assert(std::is_same_v<decltype(res), T>);
		expect(res) >= T {0};
		expect(res) < T {10};
	}
};
