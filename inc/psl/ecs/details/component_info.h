#pragma once
#include <span>
#include "psl/ecs/entity.h"
#include "psl/ecs/details/staged_sparse_component_array.h"

#include "psl/ecs/concepts.h"

namespace psl::ecs
{
	inline namespace details
	{
		/**
		 * \brief type erased storage container for components
		 *
		 */
		class component_storage_base
		{
		  public:
		  private:
			virtual bool do_has()
		};

		template <typename T>
		class component_storage final
		{
		  public:
			void add(std::span<entity> entities);
			void destroy(std::span<entity> entities);

			template <stage FromStage = stage::STALE, stage ToStage = BeginStage>
			bool has(entity entity) const noexcept;

			template <stage FromStage = stage::STALE, stage ToStage = BeginStage>
			std::span<entity> entities() const noexcept;

			template <stage FromStage = stage::STALE, stage ToStage = BeginStage>
			size_t size() const noexcept;

			void copy_into(void* location) const noexcept(std::is_nothrow_copy_constructible_v<T>);
			void move_into(void* location) noexcept(std::is_nothrow_move_constructible_v<T>);

		  private:
			staged_sparse_component_array<T> m_Data;
		};

		template <IsTagComponent T>
		class component_storage<T> final
		{
		  public:
			void add(std::span<entity> entities);
			void destroy(std::span<entity> entities);

			template <stage FromStage = stage::STALE, stage ToStage = BeginStage>
			bool has(entity entity) const noexcept;

			template <stage FromStage = stage::STALE, stage ToStage = BeginStage>
			std::span<entity> entities() const noexcept;

			template <stage FromStage = stage::STALE, stage ToStage = BeginStage>
			size_t size() const noexcept;

		  private:
			staged_sparse_component_array<flag_t> m_Data;
		};
	} // namespace details
} // namespace psl::ecs