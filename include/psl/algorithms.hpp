#pragma once
#include <psl/type_concepts.hpp>

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
	constexpr auto align_to(T value, T alignment) noexcept
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
	constexpr auto ralign_to(T value, T alignment) noexcept
	{
		if(alignment <= T{1}) return value;
		auto remainder = value % alignment;
		return (remainder) ? value - remainder : value;
	}

	/**
	 * \brief Calculates the max amount value fits in target without overflowing
	 *
	 * \param value
	 * \param target
	 * \return constexpr auto
	 */
	constexpr auto greatest_contained_count(auto value, auto target) noexcept
	{
		return (target - (target % value)) / value;
	}

	/**
	 * \brief Helper to construct the reverse of a given input.
	 *
	 * \param range Input to convert to its reverse
	 * \return constexpr auto
	 */
	constexpr auto reverse(IsRange auto& range) noexcept { return range.reverse(); }
} // namespace psl