#include <gtest/gtest.h>
#include <psl/allocator.hpp>

using namespace psl;

TEST(allocation, default_allocator_t)
{
	new_resource* memory_resource = new new_resource(alignof(int));
	config::default_allocator_t allocator{memory_resource};

	auto intAlloc = allocator.allocate<int>();
	int* iPtr	 = new(intAlloc.data) int{5};

	ASSERT_EQ(*iPtr, 5);

	allocator.deallocate<int>(intAlloc.data);
}