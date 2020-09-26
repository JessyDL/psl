#pragma once

namespace psl
{
	template <typename... Traits>
	class abstract_memory_resource;

	template <typename... Traits>
	class allocator;

	class new_resource;

	/**
	 * \brief Result type of an allocation invocation
	 * \details This contains the result of both valid and invalid allocations. Check operator bool() for conditions of
	 * an invalid allocation to distinguish which is which.
	 * Aside from a pointer to the allocated data, it optionally contains both the begin (head) and end (tail) of the
	 * allocation. This can be before and after the actual requested member due to alignment requirements, etc..
	 * Also contains the actual alignment that ended up being used for the 'data' member.
	 *
	 * \tparam T
	 */
	template <typename T>
	struct alloc_results
	{
		template <typename Y>
		explicit operator alloc_results<Y>() noexcept requires std::is_same_v<T, void>
		{
			return alloc_results<Y>{(Y*)data, head, tail, stride};
		}

		T* data{nullptr};
		void* head{0};
		void* tail{0};
		size_t stride{0}; // stride till the next element (if doing a multi alloc), or till the end of the block

		constexpr operator bool() const noexcept { return stride != 0; }
	};
}