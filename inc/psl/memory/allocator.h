#pragma once
#include <cstdint>
#include <optional>
//#include "psl/memory/region.h"
#include "psl/config.h"
#include "psl/array.h"
#include "psl/exception.h"


namespace psl::memory
{
	using rsize_t = std::uintptr_t;

	template <typename T>
	struct alloc_results
	{
		T* data;
		rsize_t begin;
		rsize_t end;
		rsize_t alignment;

		constexpr operator bool() const noexcept { return reinterpret_cast<rsize_t>(data) != end; }
	};

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

		[[nodiscard]] alloc_results<void> allocate(rsize_t size, std::optional<rsize_t> alignment = std::nullopt) {
			PSL_CONTRACT_EXCEPT_IF(alignment.value_or(1) == 0, "alignment value of 0 is not allowed, 1 is the minimum");
			auto res = do_allocate(size, alignment);
			PSL_CONTRACT_EXCEPT_IF(res.end % m_Alignment != 0,
								   "implementation of abstract region does not satisfy the requirements");

			PSL_CONTRACT_EXCEPT_IF(alignment.value_or(1) != 0, "alignment value of 0 is not allowed, 1 is the minimum");
			return res;
		}

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

	class allocator final
	{
	  public:
		allocator() = default;
		allocator(abstract_region* region) noexcept : m_Region(region){};

		template <typename T>
		alloc_results<T> allocate(std::optional<rsize_t> alignment = std::nullopt)
		{
			auto res = m_Region->allocate(sizeof(T), alignment);
			return alloc_results<T>{(T*)res.data, res.begin, res.end, res.alignment};
		}

		template <typename T>
		bool deallocate(const T& target)
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
} // namespace psl::memory