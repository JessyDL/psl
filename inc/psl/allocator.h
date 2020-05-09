#pragma once
#include <cstdint>
#include <optional>
#include <numeric>

#include "psl/config.h"
#include "psl/exception.h"
#include "psl/algorithms.h"
#include "psl/range.h"
#include <vector>
#include "psl/enum.h"


/**
 * \brief Contains everything related to memory management, such as allocators, memory resources, etc..
 *
 */
namespace psl
{
	using rsize_t = std::uintptr_t;

	enum class alloc_type : uint8_t
	{
		// allows for an allocation to occur
		allocate = 1 << 0,

		// allows the tail/end of the allocation to move, this can be used for grow/shrink commands
		shift_tail = 1 << 1,

		// allows the head/begin of the allocation to move, this can be used for grow/shrink commands
		shift_head = 1 << 2
	};

	namespace config
	{
		template <>
		inline constexpr auto enable_enum_ops<alloc_type> = enum_ops_t::BIT | enum_ops_t::LOGICAL;
	}


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

	namespace traits
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

		struct reallocate_able_t
		{
			virtual alloc_results<void> reallocate(void* location, size_t size, size_t count, size_t alignment) = 0;
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

	} // namespace traits


	/**
	 * \brief Abstract memory region interface
	 * \details used to describe the intended API for implementing a custom memory resource
	 */
	template <typename... Traits>
	class abstract_region : public Traits...
	{
	  public:
		abstract_region(size_t alignment) noexcept : m_Alignment(alignment) {}

		virtual ~abstract_region()							 = default;
		abstract_region(const abstract_region& rhs) noexcept = default;
		abstract_region(abstract_region&& rhs) noexcept		 = default;

		abstract_region& operator=(const abstract_region& rhs) noexcept = default;
		abstract_region& operator=(abstract_region&& rhs) noexcept = default;

		// virtual size_t size() const noexcept = 0;

		size_t alignment() const noexcept { return m_Alignment; }

		/**
		 * \brief Attempts to allocate the given size, and returns the information of said allocation
		 *
		 * \param[in] size Amount you wish the allocate (this will be the minimum allocated size)
		 * \param[in] alignment Special alignment override for the given allocation
		 * \returns alloc_results<void>
		 * \note the alignment override cannot override the backing resource's own requirements, instead the allocation
		 * will try to satisfy both (or fail the allocation). For example, if the resource requires 4 byte alignment,
		 * and the user requests 6 byte alignment, the least common alignment they both satisfy is 12 bytes.
		 */
		[[nodiscard]] alloc_results<void> allocate(size_t size, size_t count, size_t alignment) {
			PSL_CONTRACT_EXCEPT_IF(alignment == 0, "alignment value of 0 is not allowed, 1 is the minimum");
			auto res = do_allocate(size, count, alignment);
			PSL_CONTRACT_EXCEPT_IF(res && (size_t)res.tail % m_Alignment != 0,
								   "implementation of abstract region does not satisfy the requirements");
			return res;
		}

		// 	[[nodiscard]] alloc_results<void> reallocate(void* location, size_t size, size_t count, size_t alignment)
		// {
		// 	PSL_CONTRACT_EXCEPT_IF(alignment == 0, "alignment value of 0 is not allowed, 1 is the minimum");
		// 	auto res = do_reallocate(location, size, count, alignment);
		// 	PSL_CONTRACT_EXCEPT_IF(res && (size_t)res.tail % m_Alignment != 0,
		// 						   "implementation of abstract region does not satisfy the requirements");
		// 	return res;
		// }

		/**
		 * \brief Deallocates the given item, this is the '.data' member from alloc_results<T>
		 *
		 * \param[in] item
		 * \returns true when the deallocation succeeds
		 * \returns false when the deallocation failed
		 */
		bool deallocate(void* item, size_t alignment)
		{
			return do_deallocate(item, alignment);
		}

		// constexpr static T traits{};

		virtual alloc_results<void> do_allocate(size_t size, size_t count, size_t alignment) = 0;
		// virtual alloc_results<void> do_reallocate(void* location, size_t size, size_t count, size_t alignment) = 0;
		virtual bool do_deallocate(void* item, size_t alignment) = 0;

		template <typename T>
		static consteval bool has_trait()
		{
			if constexpr(sizeof...(Traits) == 0)
				return false;
			else
				return (std::is_same_v<T, Traits> || ...);
		}

	  private:
		size_t m_Alignment;
	};

	template <typename... Traits>
	class allocator final
	{
	  public:
		using abstract_region_t = abstract_region<Traits...>;

		allocator() = default;
		allocator(abstract_region_t* region) noexcept : m_Region(region){};
		~allocator()							   = default;
		allocator(const allocator& other) noexcept = default;
		allocator(allocator&& other) noexcept	  = default;
		allocator& operator=(const allocator& other) noexcept = default;
		allocator& operator=(allocator&& other) noexcept = default;
		template <typename T>
		alloc_results<T> allocate(size_t alignment = alignof(T))
		{
			return static_cast<alloc_results<T>>(m_Region->allocate(sizeof(T), 1, alignment));
		}

		template <typename T>
		alloc_results<T> allocate_n(size_t count, size_t repeat = 1, size_t alignment = alignof(T))
		{
			return static_cast<alloc_results<T>>(m_Region->allocate(sizeof(T) * count, repeat, alignment));
		}

		template <typename T>
		alloc_results<T> reallocate(T* location,
									size_t alignment = alignof(T)) requires traits::IsReallocateAble<abstract_region_t>
		{
			return static_cast<alloc_results<T>>(m_Region->reallocate(location, sizeof(T), 1, alignment));
		}

		template <typename T>
		alloc_results<T>
		reallocate_n(T* location, size_t count, size_t repeat = 1,
					 size_t alignment = alignof(T)) requires traits::IsReallocateAble<abstract_region_t>
		{
			return static_cast<alloc_results<T>>(m_Region->reallocate(location, sizeof(T) * count, repeat, alignment));
		}

		template <typename T>
		bool deallocate(T& target, size_t alignment = alignof(std::remove_pointer_t<std::remove_cvref_t<T>>))
		{
			if constexpr(std::is_pointer_v<T>)
			{
				if(!target) return false;
				return m_Region->deallocate((void*)target, alignment);
			}
			else
				return deallocate(std::addressof(target));
		}

		template <typename T>
		static consteval bool has_trait()
		{
			return abstract_region_t::template has_trait<T>();
		}

	  private:
		abstract_region_t* m_Region{nullptr};
	};

	template <typename... Traits>
	allocator(abstract_region<Traits...>*)->allocator<Traits...>;

	template <typename T>
	concept IsLazyAllocation = std::is_invocable_v<T>;

	template <typename T>
	concept IsGrowable = requires(T t)
	{
		t.resize(std::declval<rsize_t>());
	};

	template <typename T>
	concept IsAlignedGrowable = requires(T t)
	{
		t.resize(std::declval<rsize_t>(), std::declval<rsize_t>());
	};

	class new_resource : public abstract_region<traits::shareable_t<true>>
	{
	  public:
		new_resource(size_t alignment)
			: abstract_region(alignment){

			  };

		alloc_results<void> do_allocate(size_t bytes, size_t count, size_t alignment) override
		{
			auto align		   = std::lcm(alignment, this->alignment());
			auto stride		   = psl::align_to<size_t>(bytes, align);
			auto aligned_bytes = psl::align_to<size_t>(bytes, this->alignment());

			auto requested = aligned_bytes + (stride * (count - 1));

			auto res = operator new(requested, std::align_val_t{align});

			PSL_EXCEPT_IF(!res, "no allocation happened", std::runtime_error);

			alloc_results<void> result{};
			result.data   = res;
			result.head   = res;
			result.tail   = (void*)((size_t)result.data + requested);
			result.stride = stride;
			return result;
		}

		bool do_deallocate(void* item, size_t alignment) override
		{
			auto align = std::lcm(alignment, this->alignment());
			operator delete(item, std::align_val_t{align});
			return true;
		}
		// size_t size() const noexcept override { return 0; }
	};

	class monotonic_new_resource
		: public abstract_region<traits::shareable_t<false>, traits::queryable_size_t, traits::reallocate_able_t>
	{
		struct buffer
		{
			size_t free() const noexcept { return (size_t)((std::uintptr_t)capacity - (std::uintptr_t)end); }
			size_t size() const noexcept { return (size_t)((std::uintptr_t)capacity - (std::uintptr_t)begin); }
			void* begin{nullptr};
			void* end{nullptr};
			void* capacity{nullptr};
		};

	  public:
		monotonic_new_resource(size_t alignment, size_t initialSize = 0) : abstract_region(alignment)
		{
			if(initialSize == 0) return;
			auto& buffer	= m_Buffers.emplace_back();
			buffer.begin	= operator new(initialSize);
			buffer.end		= buffer.begin;
			buffer.capacity = (void*)((std::uintptr_t)buffer.begin + initialSize);
		}
		~monotonic_new_resource()
		{
			for(auto& buffer : m_Buffers) operator delete(buffer.begin);
		}
		alloc_results<void> reallocate([[maybe_unused]] void* location, size_t bytes, size_t count,
									   size_t alignment) override
		{
			return do_allocate(bytes, count, alignment);
		}

		alloc_results<void> do_allocate(size_t bytes, size_t count, size_t alignment) override
		{
			auto align		   = std::lcm(alignment, this->alignment());
			auto stride		   = psl::align_to<size_t>(bytes, align);
			auto aligned_bytes = psl::align_to<size_t>(bytes, this->alignment());

			auto requested = aligned_bytes + (stride * (count - 1));
			if(m_Buffers.size() == 0 || m_Buffers[m_Buffers.size() - 1].free() < requested)
			{
				auto& buffer	= m_Buffers.emplace_back();
				auto minSize	= std::max(m_Buffers[m_Buffers.size() - 1].size() * 2, requested);
				buffer.begin	= operator new(minSize);
				buffer.end		= buffer.begin;
				buffer.capacity = (void*)((std::uintptr_t)buffer.begin + minSize);
			}

			alloc_results<void> result{};
			result.data   = (void*)psl::align_to<size_t>((size_t)m_Buffers[m_Buffers.size() - 1].end, align);
			result.head   = m_Buffers[m_Buffers.size() - 1].end;
			result.tail   = (void*)((size_t)result.data + requested);
			result.stride = stride;
			return result;
		}

		bool do_deallocate([[maybe_unused]] void* item, [[maybe_unused]] size_t alignment) override { return true; }
		size_t size() const noexcept override { return m_Buffers[m_Buffers.size() - 1].size(); }

	  private:
		std::vector<buffer> m_Buffers;
	};
} // namespace psl

namespace psl::config::specialization
{
	template <typename T>
	struct default_abstract_resource_t
	{
		using type = psl::abstract_region<traits::shareable_t<true>>;
	};

	template <typename T>
	struct default_resource_t
	{
		using type = psl::new_resource;
	};

	template <typename T>
	struct default_allocator_t
	{
		using type = psl::allocator<traits::shareable_t<true>>;
	};
} // namespace psl::config::specialization

namespace psl
{
	using default_resource_t =
		typename psl::config::specialization::default_resource_t<psl::config::default_setting_t>::type;

	using default_allocator_t =
		typename psl::config::specialization::default_allocator_t<psl::config::default_setting_t>::type;

	using default_abstract_resource_t =
		typename psl::config::specialization::default_abstract_resource_t<psl::config::default_setting_t>::type;

	static inline default_resource_t default_resource{alignof(char)};
	static inline default_allocator_t default_allocator{&default_resource};
} // namespace psl