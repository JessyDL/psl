#include "psl/uid.h"
#include "psl/range.h"
//#include "psl/memory/region.h"
#include "psl/memory/allocator.h"

#include <iostream>

#include <optional>
#include <numeric>
#include "psl/algorithms.h"
namespace psl::memory
{
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
			res.begin	 = m_Offset;
			res.end		  = data + size;
			m_Offset	  = res.end;
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
int main()
{
	constexpr psl::range<size_t> r{0, 64};

	psl::memory::region<psl::memory::no_resource, psl::memory::monotonic> test{8, {1024}, {}};
	psl::memory::allocator alloc{&test};
	auto res = alloc.allocate<int>();
	//*(res.data) = 10000000;

	auto res2 = alloc.allocate<int>(12);
	//*(res2.data) = 9999;
	std::cout << *(res.data) << *(res2.data) << std::endl;

	return 0;
}