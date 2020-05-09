#include "psl/allocator.h"
#include "gtest/gtest.h"
#include "psl/exception.h"
#include "psl/array.h"

struct TerribleMemoryResource : public psl::default_abstract_resource_t
{
  public:
	TerribleMemoryResource() : psl::default_abstract_resource_t(8){};
	// psl::rsize_t size() const noexcept override { return 0; };


	psl::alloc_results<void> do_allocate([[maybe_unused]] size_t size, [[maybe_unused]] size_t count,
										 [[maybe_unused]] size_t alignment) override
	{
		return psl::alloc_results<void>{nullptr, (void*)1, (void*)2, 3};
	}

	bool do_deallocate([[maybe_unused]] void* item, [[maybe_unused]] size_t alignment) override { return true; }
};

TEST(allocator, implementation_error)
{
	ASSERT_THROW(std::ignore = TerribleMemoryResource{}.allocate(12, 1, 1), psl::implementation_error)
		<< "This poor allocator does not satisfy alignment requirements";
}

TEST(resource, default_resource_t)
{
	psl::default_resource_t resource{sizeof(char)};
	psl::default_allocator_t alloc{&resource};

	psl::array<int> test{alloc};
	test.emplace_back(0);
	test.emplace_back(5);
	// ASSERT_GE(resource.size(), sizeof(int) * 2);
	ASSERT_EQ(resource.alignment(), sizeof(char));
}