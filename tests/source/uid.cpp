#include <psl/uid.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>

using namespace psl;

TEST_CASE("uuidv4 suite", "[psl::uuidv4]")
{
	std::random_device r;
	std::mt19937_64 mersene{r()};

	[[maybe_unused]] auto res = uuidv4::generate(mersene);
	std::cout << res.to_string() << std::endl;
}