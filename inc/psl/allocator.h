#pragma once
#include <cstdint>
#include <optional>
#include <numeric>

#include "psl/config.h"
#include "psl/exception.h"
#include "psl/algorithms.h"
#include "psl/range.h"

/**
 * \brief Contains everything related to memory management, such as allocators, memory resources, etc..
 *
 */
namespace psl::memory
{
	using rsize_t = std::uintptr_t;

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
		T* data{nullptr};
		psl::range<rsize_t> range{0, 0};
		rsize_t alignment{0};

		constexpr operator bool() const noexcept { return reinterpret_cast<rsize_t>(data) != range.end; }
	};

	/**
	 * \brief Abstract memory region interface
	 * \details used to describe the intended API for implementing a custom memory resource
	 */
	class abstract_region
	{
	  public:
		abstract_region(rsize_t alignment) noexcept : m_Alignment(alignment) {}

		virtual ~abstract_region()							 = default;
		abstract_region(const abstract_region& rhs) noexcept = default;
		abstract_region(abstract_region&& rhs) noexcept		 = default;

		abstract_region& operator=(const abstract_region& rhs) noexcept = default;
		abstract_region& operator=(abstract_region&& rhs) noexcept = default;

		virtual rsize_t size() const noexcept = 0;

		virtual void* data()					  = 0;
		virtual const void* data() const noexcept = 0;

		rsize_t alignment() const noexcept { return m_Alignment; }

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
		[[nodiscard]] alloc_results<void> allocate(rsize_t size, std::optional<rsize_t> alignment = std::nullopt) {
			PSL_CONTRACT_EXCEPT_IF(alignment.value_or(1) == 0, "alignment value of 0 is not allowed, 1 is the minimum");
			auto res = do_allocate(size, alignment);
			PSL_CONTRACT_EXCEPT_IF(res.range.end % m_Alignment != 0,
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
		bool deallocate(void* item)
		{
			return do_deallocate(item);
		}

		virtual bool clear() { return true; }

	  protected:
		virtual alloc_results<void> do_allocate(rsize_t size, std::optional<rsize_t> alignment = std::nullopt) = 0;
		virtual bool do_deallocate(void* item)																   = 0;

		rsize_t m_Alignment;
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
		alloc_results<T> allocate(std::optional<rsize_t> alignment = std::nullopt)
		{
			auto res = m_Region->allocate(sizeof(T), alignment);
			return alloc_results<T>{(T*)res.data, res.range, res.alignment};
		}

		template <typename T>
		alloc_results<T> allocate_n(size_t count, std::optional<rsize_t> alignment = std::nullopt)
		{
			auto res = m_Region->allocate(sizeof(T) * count, alignment);
			return alloc_results<T>{(T*)res.data, res.range, res.alignment};
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

	class malloc_resource
	{
	  public:
		~malloc_resource() { free(previous); }

		rsize_t resize(rsize_t size)
		{
			previous = realloc(previous, size);
			m_Size   = size;
			return size;
		}

		const void* data() const noexcept { return previous; };
		void* data() noexcept { return previous; };
		rsize_t size() const noexcept { return m_Size; }

		void* previous{nullptr};
		rsize_t m_Size{0};
	};

	class no_resource
	{
	  public:
		no_resource(rsize_t size) : m_Size(size){};

		const void* data() const noexcept { return nullptr; };
		void* data() noexcept { return nullptr; };
		rsize_t size() const noexcept { return m_Size; }

	  private:
		rsize_t m_Size{};
	};

	template <typename Resource, typename Strategy>
	class region final : public abstract_region
	{
	  public:
		constexpr static bool growable = IsGrowable<Resource>;
		region(rsize_t alignment) : abstract_region(alignment), resource(), strategy()
		{
			this->strategy.resize(this->resource.size());
		}
		template <typename Resource2 = Resource, typename Strategy2 = Strategy>
		region(rsize_t alignment, Resource2&& resource, Strategy2&& strategy)
			: abstract_region(alignment), resource(std::forward<Resource2>(resource)),
			  strategy(std::forward<Strategy2>(strategy))
		{
			this->strategy.resize(this->resource.size());
		}

		alloc_results<void> do_allocate(rsize_t size, std::optional<rsize_t> alignment = std::nullopt) override
		{
			auto align		  = std::lcm(alignment.value_or(1), m_Alignment);
			auto aligned_size = psl::align_to<rsize_t>(size, m_Alignment);

			auto res = strategy.allocate(aligned_size, align);
			if constexpr(growable)
			{
				if(!res)
				{
					strategy.resize(resource.resize(resource.size() + psl::align_to<rsize_t>(size, align)));
					res = strategy.allocate(aligned_size, align);
				}
			}
			if(res) res.data = (void*)((rsize_t)res.data + (rsize_t)resource.data());
			return res;
		}

		bool do_deallocate(void* item) override { return strategy.deallocate(item); }
		rsize_t size() const noexcept override { return resource.size(); }
		void* data() override { return resource.data(); }
		const void* data() const noexcept override { return resource.data(); }

		bool clear() override
		{
			if constexpr(requires(Strategy s) { s.clear(); })
				return strategy.clear();
			else
				return false;
		}

	  private:
		Resource resource;
		Strategy strategy;
	};

	struct monotonic
	{
	  public:
		void resize(rsize_t size) { m_Size = size; };
		alloc_results<void> allocate(rsize_t size, rsize_t alignment)
		{
			auto data = psl::align_to<rsize_t>(m_Offset, alignment);
			if(data + size > m_Size) return {};
			alloc_results<void> res{};
			res.range	 = {m_Offset, data + size};
			m_Offset	  = res.range.end;
			res.data	  = reinterpret_cast<void*>(data);
			res.alignment = alignment;
			return res;
		};
		bool deallocate([[maybe_unused]] void* item) { return true; };

	  private:
		rsize_t m_Offset{0};
		rsize_t m_Size{0};
	};
} // namespace psl::memory

namespace psl::config::specialization
{
	template <typename T>
	struct default_region_t
	{
		using type = psl::memory::region<psl::memory::malloc_resource, psl::memory::monotonic>;
	};
} // namespace psl::config::specialization

namespace psl::memory
{
	using default_region_t =
		typename psl::config::specialization::default_region_t<psl::config::default_setting_t>::type;

	static inline default_region_t default_region{alignof(char)};
	static inline allocator default_allocator{&default_region};
} // namespace psl::memory