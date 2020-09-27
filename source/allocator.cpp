#include <psl/allocator.hpp>
#include <psl/algorithms.hpp>
#include <psl/exceptions.hpp>
#include <numeric>

using namespace psl;

alloc_results<void> new_resource::do_allocate(size_t size, size_t alignment)
{
	auto align		   = std::lcm(alignment, this->alignment());
	auto stride		   = psl::align_to<size_t>(size, align);
	auto aligned_bytes = psl::align_to<size_t>(size, this->alignment());

	// auto requested = aligned_bytes + (stride * (count - 1));
	auto requested = aligned_bytes;
	auto res	   = operator new(requested, std::align_val_t{align});

	PSL_EXCEPT_IF(!res, std::runtime_error, "no allocation happened");

	alloc_results<void> result{};
	result.data   = res;
	result.head   = res;
	result.tail   = (void*)((size_t)result.data + requested);
	result.stride = stride;
	return result;
}

bool new_resource::do_deallocate(void* ptr, size_t size, size_t alignment)
{
	auto align = std::lcm(alignment, this->alignment());
	operator delete(ptr, std::align_val_t{align});
	return true;
}