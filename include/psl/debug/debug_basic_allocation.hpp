#pragma once
#include <iostream>
#include <psl/allocator_traits.hpp>

namespace psl::traits {
struct debug_basic_allocation {};

template <typename Y>
struct allocator_trait<debug_basic_allocation, Y> : protected allocator_trait<basic_allocation, Y> {
  public:
	template <typename T>
	alloc_results<T> allocate(size_t bytes = sizeof(T)) {
		std::cout << "allocating " << bytes << " bytes" << std::endl;
		auto* memoryResource = ((Y*)(this))->memory_resource();
		auto res			 = static_cast<alloc_results<T>>(memoryResource->allocate(bytes, alignof(T)));


		std::cout << "allocated " << res.size() << " bytes at " << (std::uintptr_t)res.data << std::endl;
		return res;
	}

	template <typename T>
	bool deallocate(T* object, size_t bytes = sizeof(T)) {
		std::cout << "deallocating " << bytes << " bytes at " << (std::uintptr_t)object;
		auto* memoryResource = ((Y*)(this))->memory_resource();
		return memoryResource->deallocate(object, bytes, alignof(T));
	}
};

template <typename Y>
struct memory_resource_trait<debug_basic_allocation, Y> : memory_resource_trait<basic_allocation, Y> {};
}	 // namespace psl::traits