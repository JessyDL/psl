#include "psl/allocator.h"
#include "gtest/gtest.h"
#include "psl/exception.h"
#include "psl/array.h"

struct TerribleMemoryResource : public psl::abstract_region
{
  public:
	TerribleMemoryResource() : psl::abstract_region(8){};
	psl::rsize_t size() const noexcept override { return 0; };

	void* data() override { return nullptr; }
	const void* data() const noexcept override { return nullptr; }


	psl::alloc_results<void> do_allocate([[maybe_unused]] psl::rsize_t size,
										 [[maybe_unused]] std::optional<psl::rsize_t> alignment = std::nullopt) override
	{
		return psl::alloc_results<void>{nullptr, {1, 2}, 3};
	}

	bool clear() override { return true; }
	bool do_deallocate([[maybe_unused]] void* item) override { return true; }
};

TEST(allocator, implementation_error)
{
	ASSERT_THROW(std::ignore = TerribleMemoryResource{}.allocate(12), psl::implementation_error)
		<< "This poor allocator does not satisfy alignment requirements";
}

TEST(region, malloc_resource_monotonic)
{
	psl::region<psl::malloc_resource, psl::monotonic> region{sizeof(char)};
	psl::allocator alloc{&region};

	psl::array<int> test{alloc};
	test.emplace_back(0);
	test.emplace_back(5);
	ASSERT_GE(region.size(), sizeof(int) * 2);
	ASSERT_EQ(region.alignment(), sizeof(char));
	const auto& cregion = region;
	ASSERT_EQ(region.data(), cregion.data());
	ASSERT_FALSE(region.clear());
}