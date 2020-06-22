#pragma once
#include <optional>

#include "psl/ecs/entity.h"
#include "psl/allocator.h"
#include "psl/array.h"
#include "psl/static_array.h"
#include "psl/enum.h"

#include "psl/algorithms.h"
#include "psl/zip.h"

#include "psl/details/empty.h"

namespace psl::ecs
{
	inline namespace details
	{
		enum class stage
		{
			STALE   = 1 << 0,
			ADDED   = 1 << 1,
			REMOVED = 1 << 2,

			ADD_REM = 1 << 3
		};

		struct stage_stale_t
		{
			static constexpr auto value		  = stage::STALE;
			static constexpr size_t beg_index = 0;
			static constexpr size_t end_index = 1;
		};
		struct stage_added_t
		{
			static constexpr auto value		  = stage::ADDED;
			static constexpr size_t beg_index = 1;
			static constexpr size_t end_index = 3;
		};
		struct stage_removed_t
		{
			static constexpr auto value		  = stage::REMOVED;
			static constexpr size_t beg_index = 3;
			static constexpr size_t end_index = 4;
		};
		struct stage_add_rem_t
		{
			static constexpr auto value		  = stage::ADD_REM;
			static constexpr size_t beg_index = 2;
			static constexpr size_t end_index = 3;
		};
	} // namespace details
} // namespace psl::ecs

namespace psl::config
{
	template <>
	constexpr auto enable_enum_ops<psl::ecs::details::stage> =
		enum_ops_t::BIT | enum_ops_t::ARITHMETIC | enum_ops_t::LOGICAL;
}

namespace psl::ecs
{
	inline namespace details
	{
		template <stage S>
		consteval size_t to_index()
		{
			if constexpr(S == stage::STALE)
				return stage_stale_t::beg_index;
			else if constexpr(S == stage::ADDED)
				return stage_added_t::beg_index;
			else if constexpr(S == stage::ADD_REM)
				return stage_add_rem_t::beg_index;
			else if constexpr(S == stage::REMOVED)
				return stage_removed_t::beg_index;
			else
				static_assert(S != S, "The stage was defined as a combination, which isn't possible for this method");
			return -1;
		}

		template <stage S>
		consteval size_t to_end_index()
		{
			if constexpr(S == stage::STALE)
				return stage_stale_t::end_index;
			else if constexpr(S == stage::ADDED)
				return stage_added_t::end_index;
			else if constexpr(S == stage::ADD_REM)
				return stage_add_rem_t::end_index;
			else if constexpr(S == stage::REMOVED)
				return stage_removed_t::end_index;
			else
				static_assert(S != S, "The stage was defined as a combination, which isn't possible for this method");
			return -1;
		}

		inline size_t to_index(stage S) noexcept(!config::exceptions)
		{
			if(S == stage::STALE)
				return stage_stale_t::beg_index;
			else if(S == stage::ADDED)
				return stage_added_t::beg_index;
			else if(S == stage::ADD_REM)
				return stage_add_rem_t::beg_index;
			else if(S == stage::REMOVED)
				return stage_removed_t::beg_index;

			PSL_EXCEPT(std::runtime_error,
					   "The stage was defined as a combination, which isn't possible for this method");
			return -1;
		}

		inline size_t to_end_index(stage S) noexcept(!config::exceptions)
		{
			if(S == stage::STALE)
				return stage_stale_t::end_index;
			else if(S == stage::ADDED)
				return stage_added_t::end_index;
			else if(S == stage::ADD_REM)
				return stage_add_rem_t::end_index;
			else if(S == stage::REMOVED)
				return stage_removed_t::end_index;

			PSL_EXCEPT(std::runtime_error,
					   "The stage was defined as a combination, which isn't possible for this method");
			return -1;
		}

		template <typename T, typename Key = psl::ecs::entity, Key chunks_size = 4096,
				  typename Allocator = default_allocator_t>
		class staged_sparse_component_array
		{
			static constexpr bool is_power_of_two{chunks_size && ((chunks_size & (chunks_size - 1)) == 0)};
			static constexpr Key mod_val{(is_power_of_two) ? chunks_size - 1 : chunks_size};
			static constexpr auto INVALID_VALUE = std::numeric_limits<Key>::max();

			using is_empty		= std::is_same<T, flag_t>;
			using internal_type = std::conditional_t<is_empty::value, int, T>;
			using dense_t		= std::conditional_t<is_empty::value, flag_t, psl::array<internal_type>>;

		  public:
			using value_type	 = internal_type;
			using key_type		 = Key;
			using chunk_type	 = psl::array<key_type>;
			using allocator_type = Allocator;
			using size_type		 = size_t;

			auto& operator[](size_type index) requires(!is_empty::value)
			{
				auto [chunk_index, element_index] = chunk_info_for(index);
				if(chunk_index >= m_Sparse.size()) m_Sparse.resize(chunk_index + 1);
				if(m_Sparse[chunk_index].size() == 0)
				{
					m_Sparse[chunk_index].resize(chunks_size + 1);
				}

				if(m_Sparse[chunk_index][element_index] == INVALID_VALUE)
				{
					construct(index);
				}

				return m_Dense[m_Sparse[chunk_index][element_index]];
			}

			auto& at(size_type index) noexcept(!config::exceptions) requires(!is_empty::value)
			{
				auto [chunk_index, element_index] = chunk_info_for(index);

				PSL_EXCEPT_IF(chunk_index >= m_Sparse.size() || m_Sparse[chunk_index].size() == 0 ||
								  m_Sparse[chunk_index][element_index] == INVALID_VALUE,
							  "invalid access", std::range_error);

				return m_Dense[m_Sparse[chunk_index][element_index]];
			}

			const auto& at(size_type index) const noexcept(!config::exceptions) requires(!is_empty::value)
			{
				auto [chunk_index, element_index] = chunk_info_for(index);

				PSL_EXCEPT_IF(chunk_index >= m_Sparse.size() || m_Sparse[chunk_index].size() == 0 ||
								  m_Sparse[chunk_index][element_index] == INVALID_VALUE,
							  "invalid access", std::range_error);

				return m_Dense[m_Sparse[chunk_index][chunks_size]];
			}

			constexpr auto key_begin(stage_stale_t = {}) noexcept
			{
				return std::next(std::begin(m_Reverse), m_StageStart[stage_stale_t::beg_index]);
			}
			constexpr auto key_begin(stage_added_t) noexcept
			{
				return std::next(std::begin(m_Reverse), m_StageStart[stage_added_t::beg_index]);
			}
			constexpr auto key_begin(stage_removed_t) noexcept
			{
				return std::next(std::begin(m_Reverse), m_StageStart[stage_removed_t::beg_index]);
			}
			constexpr auto key_begin(stage_add_rem_t) noexcept
			{
				return std::next(std::begin(m_Reverse), m_StageStart[stage_add_rem_t::beg_index]);
			}

			constexpr auto key_begin(stage_stale_t = {}) const noexcept
			{
				return std::next(std::cbegin(m_Reverse), m_StageStart[stage_stale_t::beg_index]);
			}
			constexpr auto key_begin(stage_added_t) const noexcept
			{
				return std::next(std::cbegin(m_Reverse), m_StageStart[stage_added_t::beg_index]);
			}
			constexpr auto key_begin(stage_removed_t) const noexcept
			{
				return std::next(std::cbegin(m_Reverse), m_StageStart[stage_removed_t::beg_index]);
			}
			constexpr auto key_begin(stage_add_rem_t) const noexcept
			{
				return std::next(std::cbegin(m_Reverse), m_StageStart[stage_add_rem_t::beg_index]);
			}

			constexpr auto key_end(stage_stale_t = {}) noexcept
			{
				return std::next(std::begin(m_Reverse), m_StageStart[stage_stale_t::end_index]);
			}
			constexpr auto key_end(stage_added_t) noexcept
			{
				return std::next(std::begin(m_Reverse), m_StageStart[stage_added_t::end_index]);
			}
			constexpr auto key_end(stage_removed_t) noexcept
			{
				return std::next(std::begin(m_Reverse), m_StageStart[stage_removed_t::end_index]);
			}
			constexpr auto key_end(stage_add_rem_t) noexcept
			{
				return std::next(std::begin(m_Reverse), m_StageStart[stage_add_rem_t::end_index]);
			}

			constexpr auto key_end(stage_stale_t = {}) const noexcept
			{
				return std::next(std::cbegin(m_Reverse), m_StageStart[stage_stale_t::end_index]);
			}
			constexpr auto key_end(stage_added_t) const noexcept
			{
				return std::next(std::cbegin(m_Reverse), m_StageStart[stage_added_t::end_index]);
			}
			constexpr auto key_end(stage_removed_t) const noexcept
			{
				return std::next(std::cbegin(m_Reverse), m_StageStart[stage_removed_t::end_index]);
			}
			constexpr auto key_end(stage_add_rem_t) const noexcept
			{
				return std::next(std::cbegin(m_Reverse), m_StageStart[stage_add_rem_t::end_index]);
			}

			/*
				dense
			*/

			constexpr auto begin(stage_stale_t = {}) noexcept
			{
				return std::next(std::begin(m_Dense), m_StageStart[stage_stale_t::beg_index]);
			}
			constexpr auto begin(stage_added_t) noexcept
			{
				return std::next(std::begin(m_Dense), m_StageStart[stage_added_t::beg_index]);
			}
			constexpr auto begin(stage_removed_t) noexcept
			{
				return std::next(std::begin(m_Dense), m_StageStart[stage_removed_t::beg_index]);
			}
			constexpr auto begin(stage_add_rem_t) noexcept
			{
				return std::next(std::begin(m_Dense), m_StageStart[stage_add_rem_t::beg_index]);
			}

			constexpr auto begin(stage_stale_t = {}) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_stale_t::beg_index]);
			}
			constexpr auto begin(stage_added_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_added_t::beg_index]);
			}
			constexpr auto begin(stage_removed_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_removed_t::beg_index]);
			}
			constexpr auto begin(stage_add_rem_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_add_rem_t::beg_index]);
			}

			constexpr auto cbegin(stage_stale_t = {}) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_stale_t::beg_index]);
			}
			constexpr auto cbegin(stage_added_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_added_t::beg_index]);
			}
			constexpr auto cbegin(stage_removed_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_removed_t::beg_index]);
			}
			constexpr auto cbegin(stage_add_rem_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_add_rem_t::beg_index]);
			}

			constexpr auto end(stage_stale_t = {}) noexcept
			{
				return std::next(std::begin(m_Dense), m_StageStart[stage_stale_t::end_index]);
			}
			constexpr auto end(stage_added_t) noexcept
			{
				return std::next(std::begin(m_Dense), m_StageStart[stage_added_t::end_index]);
			}
			constexpr auto end(stage_removed_t) noexcept
			{
				return std::next(std::begin(m_Dense), m_StageStart[stage_removed_t::end_index]);
			}
			constexpr auto end(stage_add_rem_t) noexcept
			{
				return std::next(std::begin(m_Dense), m_StageStart[stage_add_rem_t::end_index]);
			}

			constexpr auto end(stage_stale_t = {}) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_stale_t::end_index]);
			}
			constexpr auto end(stage_added_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_added_t::end_index]);
			}
			constexpr auto end(stage_removed_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_removed_t::end_index]);
			}
			constexpr auto end(stage_add_rem_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_add_rem_t::end_index]);
			}

			constexpr auto cend(stage_stale_t = {}) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_stale_t::end_index]);
			}
			constexpr auto cend(stage_added_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_added_t::end_index]);
			}
			constexpr auto cend(stage_removed_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_removed_t::end_index]);
			}
			constexpr auto cend(stage_add_rem_t) const noexcept
			{
				return std::next(std::cbegin(m_Dense), m_StageStart[stage_add_rem_t::end_index]);
			}

			/*
				end region
			*/

			template <stage Stage = stage::STALE>
			auto size() const noexcept
			{
				size_t res{0};
				if constexpr(Stage && stage::STALE) res += m_StageSize[to_index<stage::STALE>()];
				if constexpr(Stage && stage::ADDED)
					res += m_StageSize[to_index<stage::ADDED>()] + m_StageSize[to_index<stage::ADD_REM>()];
				if constexpr(Stage && stage::REMOVED) res += m_StageSize[to_index<stage::REMOVED>()];
				return res;
			}

			auto capacity() const noexcept { return m_Reverse.capacity(); }

			void promote()
			{
				// 1. erases the items in the "REMOVED" stage

				// we sort the reverse item list for better coherency
				if constexpr(is_empty::value)
				{
					std::sort(std::next(std::begin(m_Reverse), m_StageStart[to_index<stage::REMOVED>()]),
							  std::next(std::begin(m_Reverse), m_StageStart[to_end_index<stage::REMOVED>()]));
				}
				else
				{
					auto zip = psl::zip(m_Dense, m_Reverse);
					psl::sort(std::next(std::begin(zip), m_StageStart[to_index<stage::REMOVED>()]),
							  std::next(std::begin(zip), m_StageStart[to_end_index<stage::REMOVED>()]),
							  [](const auto& lhs, const auto& rhs) { return psl::get<1>(lhs) < psl::get<1>(rhs); });
				}
				auto next_lookup = key_type{0};
				key_type chunk_index, element_index;
				chunk_type* cached_chunk{};
				for(auto it  = std::next(std::begin(m_Reverse), m_StageStart[to_index<stage::REMOVED>()]),
						 end = std::next(std::begin(m_Reverse), m_StageStart[to_end_index<stage::REMOVED>()]);
					it != end; ++it)
				{
					const auto index = *it;
					if(index >= next_lookup)
					{
						std::tie(chunk_index, element_index) = chunk_info_for(index);
						cached_chunk						 = &m_Sparse[chunk_index];
						next_lookup							 = (chunk_index + 1) * chunks_size;
					}
					else
					{
						element_index = offset_of(index);
					}
					(*cached_chunk)[element_index] = INVALID_VALUE;
					if((*cached_chunk)[chunks_size] == 1)
					{
						cached_chunk->resize(0);
					}
					else
					{
						(*cached_chunk)[chunks_size] -= 1;
					}
				}
				if constexpr(!is_empty::value)
					m_Dense.erase(m_StageStart[to_index<stage::REMOVED>()], m_StageSize[to_index<stage::REMOVED>()]);
				m_Reverse.erase(m_StageStart[to_index<stage::REMOVED>()], m_StageSize[to_index<stage::REMOVED>()]);
				// we move the erased stage start to ADD_REM, and set its size
				m_StageStart[to_index<stage::REMOVED>()] = m_StageStart[to_index<stage::ADD_REM>()];
				m_StageSize[to_index<stage::REMOVED>()]  = m_StageSize[to_index<stage::ADD_REM>()];

				// we update the back and the ADD_REM's size
				m_StageStart.back() =
					m_StageStart[to_index<stage::REMOVED>()] + m_StageSize[to_index<stage::REMOVED>()];
				m_StageSize[to_index<stage::ADD_REM>()] = 0;

				// 2. erases the items in the "ADDED" stage, and adds them to the "STALE" state

				// we sort the stale and added together for lookup coherency
				if(size<stage::ADDED>() > 0)
				{
					if constexpr(is_empty::value)
					{
						std::sort(std::begin(m_Reverse),
								  std::next(std::begin(m_Reverse), m_StageStart[to_end_index<stage::ADDED>()]));
					}
					else
					{
						// todo: this should be inplace merge sort instead
						auto zip = psl::zip(m_Dense, m_Reverse);
						psl::sort(std::begin(zip),
								  std::next(std::begin(zip), m_StageStart[to_end_index<stage::ADDED>()]),
								  [](const auto& lhs, const auto& rhs) { return psl::get<1>(lhs) < psl::get<1>(rhs); });
					}
				}

				// we add the ADDED size to STALE, set ADDED size to 0 and then move the start to the end of STALE
				m_StageSize[to_index<stage::STALE>()] += m_StageSize[to_index<stage::ADDED>()];
				m_StageSize[to_index<stage::ADDED>()]  = 0;
				m_StageStart[to_index<stage::ADDED>()] = m_StageSize[to_index<stage::STALE>()];
			}

			template <typename... Args>
			auto emplace(key_type index, Args&&... args)
			{
				static_assert(!is_empty::value || (is_empty::value && sizeof...(Args) == 0));
				auto [chunk_index, element_index] = chunk_info_for(index);
				if(chunk_index >= m_Sparse.size()) m_Sparse.resize(chunk_index + 1);
				if(m_Sparse[chunk_index].size() == 0)
				{
					m_Sparse[chunk_index].resize(chunks_size + 1);
				}

				m_Sparse[chunk_index][element_index] = m_Reverse.size();
				m_Sparse[chunk_index][chunks_size] += 1;
				if(m_Reverse.size() == m_Reverse.capacity())
				{
					m_Reverse.resize(psl::details::array_growth_for(m_Reverse.size()));
				}

				m_StageSize[to_index<stage::ADDED>()] += 1;

				m_StageStart[to_index<stage::ADD_REM>()] += 1;
				m_StageStart[to_index<stage::REMOVED>()] += 1;
				m_StageStart.back() += 1;

				m_Reverse.emplace(m_StageStart[to_index<stage::ADD_REM>()] - 1, index);
				if constexpr(!is_empty::value)
					return (m_Dense.emplace(m_StageStart[to_index<stage::ADD_REM>()] - 1, std::forward<Args>(args)...));
			}

			void construct(key_type index)
			{
				auto [chunk_index, element_index] = chunk_info_for(index);
				if(chunk_index >= m_Sparse.size()) m_Sparse.resize(chunk_index + 1);
				if(m_Sparse[chunk_index].size() == 0)
				{
					m_Sparse[chunk_index].resize(chunks_size + 1);
				}

				m_Sparse[chunk_index][element_index] = m_Reverse.size();
				m_Sparse[chunk_index][chunks_size] += 1;

				if(m_Reverse.size() == m_Reverse.capacity())
				{
					m_Reverse.resize(m_Reverse.next_growth());
				}

				m_Reverse.emplace(m_StageStart[to_index<stage::ADD_REM>()], index);
				if constexpr(!is_empty::value) m_Dense.emplace(m_StageStart[to_index<stage::ADD_REM>()]);

				m_StageSize[to_index<stage::ADDED>()] += 1;

				m_StageStart[to_index<stage::ADD_REM>()] += 1;
				m_StageStart[to_index<stage::REMOVED>()] += 1;
				m_StageStart.back() += 1;
			}

			void construct(std::span<key_type> indices)
			{
				for(auto index : indices) construct(index);
			}

			bool erase(key_type index) noexcept(!config::exceptions)
			{
				auto [chunk_index, element_index] = chunk_info_for(index);
				PSL_EXCEPT_IF(chunk_index >= m_Sparse.size() || m_Sparse[chunk_index].size() == 0 ||
								  m_Sparse[chunk_index][element_index] == INVALID_VALUE,
							  "invalid access", std::range_error);
				auto dense_index = m_Sparse[chunk_index][element_index];

				auto remove_swap = [this](size_type stage_index, key_type element_index) {
					auto next_index = stage_index + 1;
					m_StageStart[next_index] -= 1;
					m_StageSize[next_index] += 1;
					m_StageSize[stage_index] -= 1;

					auto [old_chunk_index, old_element_index] = chunk_info_for(m_Reverse[element_index]);
					auto [new_chunk_index, new_element_index] = chunk_info_for(m_Reverse[m_StageStart[next_index]]);

					if constexpr(!is_empty::value) std::swap(m_Dense[element_index], m_Dense[m_StageStart[next_index]]);
					std::swap(m_Reverse[element_index], m_Reverse[m_StageStart[next_index]]);
					std::swap(m_Sparse[old_chunk_index][old_element_index],
							  m_Sparse[new_chunk_index][new_element_index]);

					return m_StageStart[next_index];
				};

				if(dense_index < m_StageStart[to_index<stage::ADDED>()])
				{
					for(auto i = 0; i < 3; ++i) dense_index = remove_swap(i, dense_index);
				}
				else if(dense_index < m_StageStart[to_index<stage::ADD_REM>()])
				{
					remove_swap(to_index<stage::ADDED>(), dense_index);
				}
				else
					return false;
				return true;
			}

			template <stage BeginStage = stage::STALE, stage EndStage = stage::ADD_REM>
			bool contains(key_type index) const noexcept
			{
				auto [chunk_index, element_index] = chunk_info_for(index);
				if(chunk_index >= m_Sparse.size() || m_Sparse[chunk_index].size() == 0) return false;

				auto dense_index = m_Sparse[chunk_index][element_index];
				return dense_index != INVALID_VALUE && dense_index >= m_StageStart[to_index<BeginStage>()] &&
					   dense_index < m_StageStart[to_end_index<EndStage>()];
			}

			/*
			todo: erasure could be done as a batched 'command'; where we enqueue erasures to-be
			*/

		  private:
			constexpr key_type offset_of(key_type index) const noexcept
			{
				if constexpr(is_power_of_two)
				{
					return index & (mod_val);
				}
				else
				{
					return index % mod_val;
				}
			}

			constexpr key_type chunk_for(key_type index) const noexcept
			{
				if constexpr(is_power_of_two)
				{
					return (index - (index & (mod_val))) / chunks_size;
				}
				else
				{
					return (index - (index % mod_val)) / chunks_size;
				}
			}

			constexpr std::pair<key_type, key_type> chunk_info_for(key_type index) const noexcept
			{
				if constexpr(is_power_of_two)
				{
					auto element_index = index & (mod_val);
					return {(index - element_index) / chunks_size, element_index};
				}
				else
				{
					return {(index - (index % mod_val)) / chunks_size, index % mod_val};
				}
			}

			[[no_unique_address]] dense_t m_Dense;
			psl::array<key_type> m_Reverse;  // 1:1 with m_Dense, contains the index into the m_Sparse
			psl::array<chunk_type> m_Sparse; // contains the index into the m_Dense for the user facing index

			psl::static_array<key_type, 5> m_StageStart{0, 0, 0, 0, 0};
			psl::static_array<key_type, 4> m_StageSize{0, 0, 0, 0};
		};
	} // namespace details
} // namespace psl::ecs