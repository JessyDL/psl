#pragma once
#include <type_traits>
#include <psl/types.hpp>
#include <psl/fwd/allocator.hpp>
#include <psl/allocator_traits.hpp>
#include <psl/config.hpp>
#include <psl/types.hpp>
#include <psl/exceptions.hpp>

namespace psl
{
	/**
	 * \brief Abstract memory resource interface.
	 * \details Used to describe the intended API for implementing a custom memory resource.
	 * The Traits can be used to extend the API of the abstract class.
	 */
	template <typename... Traits>
	class abstract_memory_resource : public Traits...
	{
	  public:
		abstract_memory_resource(const size_t alignment) noexcept : m_Alignment(alignment) {}
		virtual ~abstract_memory_resource()									   = default;
		abstract_memory_resource(const abstract_memory_resource& rhs) noexcept = default;
		abstract_memory_resource(abstract_memory_resource&& rhs) noexcept	  = default;

		abstract_memory_resource& operator=(const abstract_memory_resource& rhs) noexcept = default;
		abstract_memory_resource& operator=(abstract_memory_resource&& rhs) noexcept = default;

		size_t alignment() const noexcept { return m_Alignment; }

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
		allocate(size_t size, size_t alignment, size_t count = 1) noexcept
		{
			PSL_CONTRACT_EXCEPT_IF(alignment == 0, "alignment value of 0 is not allowed, 1 is the minimum");
			auto res = do_allocate(size, alignment, count);
			PSL_CONTRACT_EXCEPT_IF(res && (size_t)res.tail % m_Alignment != 0,
								   "implementation of abstract region does not satisfy the requirements");
			return res;
		}

		/**
		 * \brief Deallocates the given item, this is the '.data' member from alloc_results<T>
		 *
		 * \param[in] item
		 * \param[in] size Original size of the allocation
		 * \param[in] alignment Original alignment of the allocation
		 * \returns true when the deallocation succeeds
		 */
		bool deallocate(void* item, size_t size, size_t alignment) { return do_deallocate(item, size, alignment); }

		template <typename T>
		static consteval bool has_trait()
		{
			if constexpr(sizeof...(Traits) == 0)
				return false;
			else
				return (std::is_same_v<T, Traits> || ...);
		}

	  protected:
		virtual alloc_results<void> do_allocate(size_t size, size_t alignment, size_t count) = 0;
		virtual bool do_deallocate(void* ptr, size_t size, size_t alignment)				 = 0;

	  private:
		size_t m_Alignment;
	};


	template <typename... Traits>
	class allocator final
	{
	  public:
		using abstract_memory_resource_t = abstract_memory_resource<Traits...>;

		allocator() = default;
		allocator(abstract_memory_resource_t* memoryResource) noexcept : m_MemoryResource(memoryResource){};
		~allocator()							   = default;
		allocator(const allocator& other) noexcept = default;
		allocator(allocator&& other) noexcept	  = default;
		allocator& operator=(const allocator& other) noexcept = default;
		allocator& operator=(allocator&& other) noexcept = default;

		template <typename T>
		alloc_results<T> allocate(size_t bytes = sizeof(T), size_t alignment = alignof(T))
		{
			return static_cast<alloc_results<T>>(m_MemoryResource->allocate(bytes, alignment, 1));
		}

		template <typename T>
		bool deallocate(T* object, size_t bytes = sizeof(T), size_t alignment = alignof(T))
		{
			return m_MemoryResource->deallocate(object, bytes, alignment);
		}

	  private:
		abstract_memory_resource_t* m_MemoryResource{nullptr};
	};

	class new_resource : public config::default_abstract_memory_resource_t
	{
		using base_type = config::default_abstract_memory_resource_t;

	  public:
		new_resource(size_t alignment) : base_type(alignment) {}

	  private:
		alloc_results<void> do_allocate(size_t size, size_t alignment, size_t count) override;

		bool do_deallocate(void* ptr, size_t size, size_t alignment) override;
	};
} // namespace psl
