#pragma once
#include <cstdint>
#include <string_view>

namespace psl
{
	inline namespace details
	{
		template <size_t N>
		struct fixed_ascii_string;

		template <typename T>
		struct is_fixed_ascii_string : std::false_type
		{};
		template <size_t N>
		struct is_fixed_ascii_string<fixed_ascii_string<N>> : std::true_type
		{};

		template <typename T>
		concept IsFixedAsciiString = is_fixed_ascii_string<std::remove_cvref_t<T>>::value;

		template <size_t N>
		struct fixed_ascii_string
		{
			char buf[N + 1]{};
			consteval fixed_ascii_string(char const *s)
			{
				for(size_t i = 0; i != N; ++i) buf[i] = s[i];
			}
			auto operator<=>(const fixed_ascii_string &) const = default;

			constexpr char operator[](size_t index) const noexcept { return buf[index]; }

			constexpr operator std::string_view() const noexcept { return std::string_view{buf, N}; }
			constexpr operator char const *() const { return buf; }

			constexpr size_t size() const noexcept { return N; }

			template <size_t start, size_t end>
			consteval fixed_ascii_string<end - start> substr() const noexcept
			{
				static_assert(start <= end);
				static_assert(end <= N + 1);
				return fixed_ascii_string<end - start>{&buf[start]};
			}
		};
		template <unsigned N>
		fixed_ascii_string(char const (&)[N]) -> fixed_ascii_string<N - 1>;
	} // namespace details
} // namespace psl