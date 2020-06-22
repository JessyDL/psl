#pragma once
#include <type_traits>

namespace psl
{
	inline namespace metaprogramming
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
	} // namespace metaprogramming
} // namespace psl