#include <psl/uid.hpp>
#include <iostream>

using namespace psl;

auto uuidv_test0 = suite<"uuidv4", "psl">() = [] {
	std::random_device r;
	std::mt19937_64 mersene{r()};

	[[maybe_unused]] auto res = uuidv4::generate(mersene);
	std::cout << res.to_string() << std::endl;
};