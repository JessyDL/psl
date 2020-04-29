
#include "gtest/gtest.h"

#include "psl/uid.h"
#include "psl/range.h"
#include "psl/allocator.h"
#include "psl/array.h"

#include <iostream>

#include "psl/enum.h"


enum class memory_type
{
	device_local  = 1 << 0,
	host_visible  = 1 << 1,
	host_coherent = 1 << 2
};

namespace psl::config
{
	template <>
	inline constexpr auto enable_enum_ops<memory_type> = enum_ops_t::BIT | enum_ops_t::ARITHMETIC;
}

int main(int argc, char** argv)
{
	// constexpr psl::range<size_t> r{0, 64};
	// psl::default_region_t region{4};
	// psl::allocator alloc{&region};

	// psl::array<int> test{alloc};

	// test.emplace_back(0);
	// test.emplace_back(5);

	// for(const auto& element : test)
	// {
	// 	std::cout << element << std::endl;
	// }

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}