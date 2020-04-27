#pragma once

#include <stddef.h> // size_t
#include "psl/string.h"

namespace psl::details
{
	template <size_t N>
	struct fixed_string
	{
		char8_t buf[N + 1]{};
		consteval fixed_string(char8_t const *s)
		{
			for(size_t i = 0; i != N; ++i) buf[i] = s[i];
		}
		auto operator<=>(const fixed_string &) const = default;

		constexpr char8_t operator[](size_t index) const noexcept { return buf[index]; }

		constexpr operator psl::string_view() const noexcept { return psl::string_view{buf, N}; }
		constexpr operator char8_t const *() const { return buf; }

		constexpr size_t size() const noexcept { return N; }

		template <size_t start, size_t end>
		consteval fixed_string<end - start> substr() const noexcept
		{
			static_assert(start <= end);
			static_assert(end <= N + 1);
			return fixed_string<end - start>{&buf[start]};
		}
	};
	template <unsigned N>
	fixed_string(char8_t const (&)[N])->fixed_string<N - 1>;
} // namespace psl::details