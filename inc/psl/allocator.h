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

	// namespace traits
	// {
	// 	struct movable_t
	// 	{};
	// 	struct nontrivial_types_support_t
	// 	{};
	// } // namespace traits

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


	/**
	 * \brief Abstract memory region interface
	 * \details used to describe the intended API for implementing a custom memory resource
	 */
	class abstract_region
	{
	  public:
		abstract_region(size_t alignment) noexcept : m_Alignment(alignment) {}

		virtual ~abstract_region()							 = default;
		abstract_region(const abstract_region& rhs) noexcept = default;
		abstract_region(abstract_region&& rhs) noexcept		 = default;

		abstract_region& operator=(const abstract_region& rhs) noexcept = default;
		abstract_region& operator=(abstract_region&& rhs) noexcept = default;

		virtual size_t size() const noexcept = 0;

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

			[[nodiscard]] alloc_results<void> reallocate(void* location, size_t size, size_t count, size_t alignment)
		{
			PSL_CONTRACT_EXCEPT_IF(alignment == 0, "alignment value of 0 is not allowed, 1 is the minimum");
			auto res = do_reallocate(location, size, count, alignment);
			PSL_CONTRACT_EXCEPT_IF(res && (size_t)res.tail % m_Alignment != 0,
								   "implementation of abstract region does not satisfy the requirements");
			return res;
		}

		/**
		 * \brief Deallocates the given item, this is the '.data' member from alloc_results<T>
		 *
		 * \param[in] item
		 * \returns true when the deallocation succeeds
		 * \returns false when the deallocation failed
		 */
		bool deallocate(void* item) { return do_deallocate(item); }

		// constexpr static T traits{};

		virtual alloc_results<void> do_allocate(size_t size, size_t count, size_t alignment)				   = 0;
		virtual alloc_results<void> do_reallocate(void* location, size_t size, size_t count, size_t alignment) = 0;
		virtual bool do_deallocate(void* item)																   = 0;

	  private:
		size_t m_Alignment;
	};


	class allocator
	{
	  public:
		allocator() = default;
		allocator(abstract_region* region) noexcept : m_Region(region){};
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
		alloc_results<T> reallocate(T* location, size_t alignment = alignof(T))
		{
			return static_cast<alloc_results<T>>(m_Region->reallocate(location, sizeof(T), 1, alignment));
		}

		template <typename T>
		alloc_results<T> reallocate_n(T* location, size_t count, size_t repeat = 1, size_t alignment = alignof(T))
		{
			return static_cast<alloc_results<T>>(m_Region->reallocate(location, sizeof(T) * count, repeat, alignment));
		}

		template <typename T>
		bool deallocate(T& target)
		{
			return m_Region->deallocate(std::addressof(target));
		}

		template <typename T>
		bool deallocate(const T* target)
		{
			return m_Region->deallocate((void*)target);
		}

	  private:
		abstract_region* m_Region{nullptr};
	};

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


	/**
	 * \brief Allocates a single region using malloc (can be resized)
	 *
	 */
	// class mono_malloc_resource
	// {
	//   public:
	// 	~mono_malloc_resource() { free(m_Data); }

	// 	rsize_t resize(rsize_t size)
	// 	{
	// 		m_Data = realloc(m_Data, size);
	// 		m_Size = size;
	// 		return size;
	// 	}

	// 	const void* data() const noexcept { return m_Data; };
	// 	void* data() noexcept { return m_Data; };
	// 	rsize_t size() const noexcept { return m_Size; }

	// 	bool allocate(rsize_t size, [[maybe_unused]] rsize_t alignment)
	// 	{
	// 		m_Data = realloc(m_Data, size);
	// 		m_Size = size;
	// 		return size;
	// 	}

	// 	bool deallocate([[maybe_unused]] void* allocation) { return true; }

	// 	void* translate(rsize_t location)
	// 	{
	// 		PSL_EXCEPT_IF(m_Data == nullptr, "no memory was allocated", std::runtime_error);
	// 		PSL_EXCEPT_IF(location >= m_Size, "asked for an offset into the region beyond its allocated size",
	// 					  std::runtime_error);
	// 		return (void*)((std::uintptr_t)m_Data + location);
	// 	}

	// 	void* m_Data{nullptr};
	// 	rsize_t m_Size{0};
	// };

	// class new_resource
	// {
	// 	struct allocation
	// 	{
	// 		void* data;
	// 		rsize_t size;
	// 	};

	//   public:
	// 	~new_resource() {}

	// 	rsize_t size() const noexcept { return m_Size; }

	// 	bool allocate(rsize_t size, rsize_t alignment)
	// 	{
	// 		m_Allocations.emplace_back(::operator new(size, static_cast<std::align_val_t>(alignment), std::nothrow),
	// 								   size);
	// 		m_Size += size;
	// 		return true;
	// 	}

	// 	bool deallocate(void* allocation)
	// 	{
	// 		if(auto it = std::find_if(std::begin(m_Allocations), std::end(m_Allocations),
	// 								  [allocation](const auto& alloc) { return alloc.data == allocation; });
	// 		   it != std::end(m_Allocations))
	// 		{
	// 			::operator delete(it->data);
	// 			m_Allocations.erase(it);
	// 			return true;
	// 		}
	// 		return false;
	// 	}

	// 	void* translate(rsize_t location)
	// 	{
	// 		PSL_EXCEPT_IF(location >= m_Size, "asked for an offset into the region beyond its allocated size",
	// 					  std::runtime_error);
	// 		rsize_t aggregate_size{0};
	// 		if(auto it = std::find_if(std::begin(m_Allocations), std::end(m_Allocations),
	// 								  [location, &aggregate_size](const auto& alloc) {
	// 									  if(location >= aggregate_size && location < aggregate_size + alloc.size)
	// 										  return true;
	// 									  aggregate_size += alloc.size;
	// 									  return false;
	// 								  });
	// 		   it != std::end(m_Allocations))
	// 		{
	// 			return (void*)((std::uintptr_t)it->data + (location - aggregate_size));
	// 		}
	// 		PSL_EXCEPT_IF(true, "asked for an offset into the region that it doesn't have", std::runtime_error);
	// 		return nullptr;
	//} // namespace psl

	// std::vector<allocation> m_Allocations;
	// rsize_t m_Size{0};
	// }
	// ;

	/**
	 * \brief Does no actual allocation, only simulates it
	 *
	 */
	// class no_resource
	// {
	//   public:
	// 	no_resource(rsize_t size) : m_Size(size){};
	// 	rsize_t resize(rsize_t size) noexcept
	// 	{
	// 		m_Size = size;
	// 		return m_Size;
	// 	}
	// 	const void* data() const noexcept { return nullptr; };
	// 	void* data() noexcept { return nullptr; };
	// 	rsize_t size() const noexcept { return m_Size; }

	//   private:
	// 	rsize_t m_Size{};
	// };

	class monotonic_new_resource : public abstract_region
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
		alloc_results<void> do_reallocate([[maybe_unused]] void* location, size_t bytes, size_t count,
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

		bool do_deallocate([[maybe_unused]] void* item) override { return true; }
		size_t size() const noexcept override { return m_Buffers[m_Buffers.size() - 1].size(); }

	  private:
		std::vector<buffer> m_Buffers;
	};

	// template <typename Resource, typename Strategy>
	// class region final : public abstract_region
	// {
	//   public:
	// 	constexpr static bool growable = IsGrowable<Resource>;
	// 	region(rsize_t alignment) : abstract_region(alignment), resource(), strategy()
	// 	{
	// 		this->strategy.resize(this->resource.size());
	// 	}
	// 	template <typename Resource2 = Resource, typename Strategy2 = Strategy>
	// 	region(rsize_t alignment, Resource2&& resource, Strategy2&& strategy)
	// 		: abstract_region(alignment), resource(std::forward<Resource2>(resource)),
	// 		  strategy(std::forward<Strategy2>(strategy))
	// 	{
	// 		this->strategy.resize(this->resource.size());
	// 	}

	// 	alloc_results<void> do_reallocate(void* location, rsize_t size, rsize_t count,
	// 									  std::optional<rsize_t> alignment = std::nullopt) override
	// 	{
	// 		auto align		  = std::lcm(alignment.value_or(m_Alignment), m_Alignment);
	// 		auto stride		  = psl::align_to<rsize_t>(size, align);
	// 		auto aligned_size = psl::align_to<rsize_t>(size, m_Alignment);

	// 		auto res = strategy.reallocate(aligned_size + (stride * (count - 1)), align);

	// 		return alloc_results<void>{};
	// 	}

	// 	alloc_results<void> do_allocate(rsize_t size, rsize_t count,
	// 									std::optional<rsize_t> alignment = std::nullopt) override
	// 	{

	// 		auto align		  = std::lcm(alignment.value_or(m_Alignment), m_Alignment);
	// 		auto stride		  = psl::align_to<rsize_t>(size, align);
	// 		auto aligned_size = psl::align_to<rsize_t>(size, m_Alignment);

	// 		auto res = strategy.allocate(aligned_size + (stride * (count - 1)), align);
	// 		if constexpr(growable)
	// 		{
	// 			if(!res)
	// 			{
	// 				strategy.resize(resource.resize(resource.size() + (stride * count)));
	// 				res = strategy.allocate(aligned_size + (stride * (count - 1)), align);
	// 			}
	// 		}
	// 		if(res)
	// 		{
	// 			res.data   = resource.translate((rsize_t)res.data);
	// 			res.stride = stride;
	// 		}
	// 		return res;
	// 	}

	// 	bool do_deallocate(void* item) override { return strategy.deallocate(item); }
	// 	rsize_t size() const noexcept override { return resource.size(); }
	// 	void* data() override { return resource.data(); }
	// 	const void* data() const noexcept override { return resource.data(); }

	// 	bool clear() override
	// 	{
	// 		if constexpr(requires(Strategy s) { s.clear(); })
	// 			return strategy.clear();
	// 		else
	// 			return false;
	// 	}

	//   private:
	// 	Resource resource;
	// 	Strategy strategy;
	// };

	/**
	 * \brief monotonic allocation strategy
	 * \details Allocates in a monotonic fashion, ever increasing, never decreasing even when deallocating.
	 *
	 */
	// struct monotonic
	// {
	//   public:
	// 	void resize(rsize_t size) { m_Size = size; };
	// 	alloc_results<void> allocate(rsize_t size, rsize_t alignment)
	// 	{
	// 		auto data = psl::align_to<rsize_t>(m_Offset, alignment);
	// 		if(data + size > m_Size) return {};
	// 		alloc_results<void> res{};
	// 		res.range	 = {m_Offset, data + size};
	// 		m_Offset	  = res.range.end;
	// 		res.data	  = reinterpret_cast<void*>(data);
	// 		res.alignment = alignment;
	// 		return res;
	// 	};
	// 	bool deallocate([[maybe_unused]] void* item) { return true; };

	//   private:
	// 	rsize_t m_Offset{0};
	// 	rsize_t m_Size{0};
	// };
} // namespace psl

namespace psl::config::specialization
{
	template <typename T>
	struct default_resource_t
	{
		using type = psl::monotonic_new_resource;
	};
} // namespace psl::config::specialization

namespace psl
{
	using default_resource_t =
		typename psl::config::specialization::default_resource_t<psl::config::default_setting_t>::type;

	static inline default_resource_t default_resource{alignof(char)};
	static inline allocator default_allocator{&default_resource};
} // namespace psl