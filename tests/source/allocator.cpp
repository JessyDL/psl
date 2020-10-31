#include <gtest/gtest.h>
#include <psl/allocator.hpp>
#include <psl/span.hpp>
#include <array>

using namespace psl;

TEST(allocation, default_allocator_t)
{
	new_resource* memory_resource = new new_resource(alignof(int));
	config::default_allocator_t allocator{memory_resource};

	static_assert(sizeof(config::default_allocator_t) == sizeof(new_resource*));

	ASSERT_EQ(sizeof(config::default_allocator_t), sizeof(new_resource*));
	auto intAlloc = construct<int>(allocator, 5);

	ASSERT_EQ(*intAlloc.data, 5);

	destroy(allocator, *intAlloc.data);

	auto range = construct_n<int>(allocator, 5, 0);
	psl::span<int, 5> range_span{range.data};

	for(auto i = 0u; auto& it : range_span)
	{
		it = ++i;
		ASSERT_EQ(it, i);
	}
	for(auto i = 0u; auto& it : range_span)
	{
		++i;
		ASSERT_EQ(it, i);
	}
}