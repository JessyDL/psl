#pragma once
#include <cstdint>
#include <cstddef>

namespace psl
{
	using i8  = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;

	using ui8  = std::uint8_t;
	using ui16 = std::uint16_t;
	using ui32 = std::uint32_t;
	using ui64 = std::uint64_t;

	using uint = unsigned int;

	inline constexpr std::size_t dynamic_extent = -1;


	struct default_value_t
	{};

	inline constexpr default_value_t default_value{};
} // namespace psl