#pragma once
#include <type_traits>

namespace psl::metaprogramming
{
	template <typename... Fs>
	constexpr inline bool any_of(Fs&&... f) requires(std::is_invocable_v<Fs>&&...)
	{
		return (f() || ...);
	}

	template <typename... Fs>
	constexpr inline bool all_of(Fs&&... f) requires(std::is_invocable_v<Fs>&&...)
	{
		return (f() && ...);
	}

	template <typename... Fs>
	constexpr inline bool none_of(Fs&&... f) requires(std::is_invocable_v<Fs>&&...)
	{
		return (!f() && ...);
	}
} // namespace psl::metaprogramming