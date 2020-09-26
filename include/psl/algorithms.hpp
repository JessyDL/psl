#pragma once
#include <psl/concepts.hpp>

namespace psl
{
	/**
	 * \brief Aligns a value to the next possible value that satisfies the alignment
	 *
	 * \tparam T
	 * \param[in] value
	 * \param[in] alignment
	 * \returns constexpr auto
	 */
	template <IsIntegral T>
	constexpr inline auto align_to(T value, T alignment) noexcept
	{
		if(alignment <= T{1}) return value;
		auto remainder = value % alignment;
		return (remainder) ? value + (alignment - remainder) : value;
	}

	/**
	 * \brief Aligns a value to the previous possible value that satisfies the alignment (including the seed value)
	 *
	 * \tparam T
	 * \param[in] value
	 * \param[in] alignment
	 * \returns constexpr auto
	 */
	template <IsIntegral T>
	constexpr inline auto ralign_to(T value, T alignment) noexcept
	{
		if(alignment <= T{1}) return value;
		auto remainder = value % alignment;
		return (remainder) ? value - remainder : value;
	}
} // namespace psl