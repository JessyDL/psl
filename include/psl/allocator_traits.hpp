#pragma once
#include <type_traits>
#include <psl/exceptions.hpp>
#include <psl/fwd/allocator.hpp>

#pragma region forward

#pragma endregion forward

#pragma region definition
namespace psl::traits
{
	template <typename T, typename Y>
	struct allocator_trait
	{};

	template <typename T, typename Y>
	struct memory_resource_trait
	{};

	/**
	 * \brief Indicates if the resource will physically allocate somewhere.
	 * \details Sometimes you want to simulate resource management, this either to manage a resource you have no
	 * direct access to (such as the GPU), or for other reasons. This property can indicate suitable resources that
	 * support such behaviour.
	 */
	template <bool Value>
	struct physically_allocated_t : std::conditional_t<Value, std::true_type, std::false_type>
	{};

	/**
	 * \brief Indicates if the resource allocates in a way that can be shared with others
	 * \details When a resource is non_shareable it means that it is best suited for a single
	 * object to be used as an allocator, as when it reallocates it can potentiall invalidate others.
	 * This could be used in a resizeable arena-like allocator, where the object is the gatekeeper to the resource
	 * to preserve correct usage/linkage.
	 * Most resource will be shareable_t, which is how the standard (std) allocators also behave.
	 */
	template <bool Value>
	struct shareable_t : std::conditional_t<Value, std::true_type, std::false_type>
	{};

	struct queryable_size_t
	{
		virtual size_t size() const noexcept = 0;
	};

	/**
	 * @brief Support relocatability of previously allocated block.
	 * @details When an allocator supports this trait it means that the allocator can either sometimes, or all of
	 * the times, support the resizing of a previously allocated block.
	 */
	struct reallocate_able_t
	{
		virtual alloc_results<void> reallocate(void* location, size_t size, size_t count, size_t alignment) = 0;
	};

	struct host_reachable_t
	{};

	enum class syncronization
	{
		always_synced = 0, /* no effort needs to be done to access the data. */
		flush		  = 1, /* requires a flush (or flush-like) interaction before the data can be accessed. */
		async		  = 2, /* accessing the data is an async operation. */
		unreachable	  = 3  /* the data can never be reached. */
	};

	struct alignment_override
	{};

	/**
	 * \brief Basic allocation interface.
	 * \details can be compared to a simplistic allocator, that can only allocate in bytes.
	 * The alignment is determined by the alignof of the target object.
	 *
	 */
	struct basic_allocation
	{};

	/**
	 * @brief Readability of data from the context of the host.
	 * @note host refers to the context that is executing the code.
	 */
	template <syncronization Sync = syncronization::always_synced>
	struct host_readability_t
	{
		static constexpr syncronization value{Sync};
	};

	template <syncronization Sync = syncronization::always_synced>
	struct host_writability_t
	{
		static constexpr syncronization value{Sync};
	};

	template <typename T, typename Trait>
	concept HasTrait = T::template has_trait<Trait>();

	template <typename T>
	concept IsShareable = HasTrait<T, shareable_t<true>>;

	template <typename T>
	concept IsVirtuallyAllocated = HasTrait<T, physically_allocated_t<false>>;

	template <typename T>
	concept IsPhysicallyAllocated = !IsVirtuallyAllocated<T>;

	template <typename T>
	concept IsSizeQueryable = HasTrait<T, queryable_size_t>;

	template <typename T>
	concept IsReallocateAble = HasTrait<T, reallocate_able_t>;

	template <typename Y>
	struct allocator_trait<basic_allocation, Y>
	{
	  public:
		template <typename T>
		alloc_results<T> allocate(size_t bytes = sizeof(T));

		template <typename T>
		alloc_results<T> allocate_n(size_t count, size_t bytes = sizeof(T));

		template <typename T>
		bool deallocate(T* object, size_t bytes = sizeof(T));
	};

	template <typename Y>
	struct memory_resource_trait<basic_allocation, Y>
	{
	  public:
		/**
		 * \brief Attempts to allocate the given size, and returns the information of said allocation
		 *
		 * \param[in] size Amount you wish the allocate (this will be the minimum allocated size)
		 * \param[in] alignment Special alignment override for the given allocation
		 * \param[in] count how many items should be allocated
		 * \returns alloc_results<void>
		 * \note the alignment override cannot override the backing resource's own requirements, instead the allocation
		 * will try to satisfy both (or fail the allocation). For example, if the resource requires 4 byte alignment,
		 * and the user requests 6 byte alignment, the least common alignment they both satisfy is 12 bytes.
		 */
		[[nodiscard(
			"discarding this will lead to a memory leak as you won't know what to deallocate")]] alloc_results<void>
		allocate(size_t size, size_t alignment) noexcept(!psl::config::implementation_exceptions);

		/**
		 * \brief Deallocates the given item, this is the '.data' member from alloc_results<T>
		 *
		 * \param[in] item
		 * \param[in] size Original size of the allocation
		 * \param[in] alignment Original alignment of the allocation
		 * \returns true when the deallocation succeeds
		 */
		bool deallocate(void* item, size_t size, size_t alignment) { return do_deallocate(item, size, alignment); }

	  protected:
		virtual alloc_results<void> do_allocate(size_t size, size_t alignment) = 0;
		virtual bool do_deallocate(void* ptr, size_t size, size_t alignment)   = 0;
	};
} // namespace psl::traits
#pragma endregion definition

#pragma region implementation
namespace psl::traits
{
	template <typename Y>
	template <typename T>
	alloc_results<T> allocator_trait<basic_allocation, Y>::allocate(size_t bytes)
	{
		auto* memoryResource = ((Y*)(this))->resource();
		return static_cast<alloc_results<T>>(memoryResource->allocate(bytes, alignof(T)));
	}

	template <typename Y>
	template <typename T>
	alloc_results<T> allocator_trait<basic_allocation, Y>::allocate_n(size_t count, size_t bytes)
	{
		auto* memoryResource = ((Y*)(this))->resource();
		return static_cast<alloc_results<T>>(memoryResource->allocate(bytes * count, alignof(T)));
	}

	template <typename Y>
	template <typename T>
	bool allocator_trait<basic_allocation, Y>::deallocate(T* object, size_t bytes)
	{
		auto* memoryResource = ((Y*)(this))->resource();
		return memoryResource->deallocate(object, bytes, alignof(T));
	}

	template <typename Y>
	alloc_results<void> memory_resource_trait<basic_allocation, Y>::allocate(size_t size, size_t alignment) noexcept(
		!psl::config::implementation_exceptions)
	{
		PSL_CONTRACT_EXCEPT_IF(alignment == 0, "alignment value of 0 is not allowed, 1 is the minimum");
		auto res = do_allocate(size, alignment);
		PSL_CONTRACT_EXCEPT_IF(res && (std::uintptr_t)res.tail % ((const Y*)this)->alignment() != 0,
							   "implementation of abstract region does not satisfy the requirements");
		return res;
	}

} // namespace psl::traits

#pragma endregion implementation