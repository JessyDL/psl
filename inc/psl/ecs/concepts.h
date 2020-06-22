#pragma once
#include <type_traits>

namespace psl::ecs
{
	template <typename T>
	concept IsComponent = true;

	template <typename T>
	concept IsTagComponent = is_empty_v<T>;
} // namespace psl::ecs