#pragma once
#include <type_traits>
#include <psl/fwd/allocator.hpp>

namespace psl::traits
{
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

	/**
	 * @brief Readability of data from the context of the host.
	 * @info host refers to the context that is executing the code.
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

} // namespace psl::traits