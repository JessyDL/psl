#pragma once
#include <cstdint>
#include "psl/config.h"

namespace psl::config
{
	template <typename T>
	using entity_type = uint32_t;
}
namespace psl::ecs
{
	using entity = psl::config::entity_type<psl::config::default_setting_t>;

	/// \brief checks if an entity is valid or not
	/// \param[in] e the entity to check
	constexpr bool valid(entity e) noexcept { return e != static_cast<entity>(0); }

	/**
	 * \brief component flag, to be used when no data needs to be stored just a "presence" check
	 *
	 */
	struct flag_t
	{};
} // namespace psl::ecs