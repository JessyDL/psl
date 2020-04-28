#pragma once
#include <cstdint>
#include "psl/config.h"
#include "psl/range.h"
#include "psl/algorithms.h"

namespace psl::memory
{
	/**
	 * \brief Encapsulates a region of memory, this can be both physically backed and 'virtual'
	 * \details A basic, non resizeable region of memory. Contains various strategies on how
	 * to allocate (if it allocates at all).
	 *
	 * It will clear up its memory when it goes out of scope, it's up to you to destroy any
	 * created objects before that.
	 */
	class region
	{
		enum class behaviour
		{
			paged,
			malloc,
			none
		};

	  public:
		struct no_alloc_t
		{};
		struct paged_alloc_t
		{};
		struct malloc_t
		{};

		region(size_t size, size_t alignment) noexcept(!psl::config::exceptions);
		explicit region(malloc_t, size_t size, size_t alignment) noexcept(!psl::config::exceptions);
		explicit region(paged_alloc_t, size_t size, size_t alignment) noexcept(!psl::config::exceptions);
		explicit region(no_alloc_t, size_t size, size_t alignment) noexcept
			: m_Alignment(alignment), m_Range(0, align_to(size, alignment)), m_Behaviour(behaviour::none)
		{}

		~region();

		const auto& range() const noexcept { return m_Range; }
		auto begin() const noexcept { return m_Range.begin; }
		auto end() const noexcept { return m_Range.end; }
		auto size() const noexcept { return m_Range.size(); }

		void* data() noexcept { return m_Data; }
		const void* data() const noexcept { return m_Data; }

		auto alignment() const noexcept { return m_Alignment; }

		bool is_physically_back() const noexcept { return m_Data != nullptr; }

	  private:
		size_t m_Alignment;
		psl::range<size_t> m_Range;
		void* m_Data{nullptr};
		behaviour m_Behaviour{behaviour::none};
	};
} // namespace psl::memory

namespace psl::config
{
	namespace specialize
	{
		template <typename T>
		struct region_default_alloc_t
		{
			using type = psl::memory::region::malloc_t;
		};
	} // namespace specialize
} // namespace psl::config

namespace psl::config::specialization
{
	template <typename T>
	struct default_region_t
	{
		using type = psl::memory::region;
	};
} // namespace psl::config::specialization

namespace psl::memory
{
	using default_region_t =
		typename psl::config::specialization::default_region_t<psl::config::default_setting_t>::type;
} // namespace psl::memory