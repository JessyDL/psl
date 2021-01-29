#pragma once
#include <psl/type_concepts.hpp>

namespace psl
{
	/**
	 * \brief Aligns a value to the next possible value that satisfies the alignment
	 * \note alignment will be transformed to its absolute value.
	 *
	 * \tparam T
	 * \param[in] value
	 * \param[in] alignment
	 * \returns constexpr auto
	 */
	template <IsIntegral T>
	constexpr T align_to(T value, T alignment) noexcept
	{
		if constexpr(IsSignedIntegral<T>)
		{
			if(alignment < 0) alignment = -alignment;
		}
		if(alignment <= T{1}) return value;
		T remainder = value % alignment;
		if(remainder != 0)
			[[likely]] return value + (alignment - remainder);
		else
			[[unlikely]] return value;
	}

	/**
	 * \brief Aligns a value to the previous possible value that satisfies the alignment (including the seed value)
	 *
	 * \tparam T
	 * \param[in] value
	 * \param[in] alignment
	 * \returns constexpr auto
	 */
	template <IsUnsignedIntegral T>
	constexpr T ralign_to(T value, T alignment) noexcept
	{
		if(alignment <= T{1}) return value;
		T remainder = value % alignment;

		if(remainder != 0)
			[[likely]] return value - remainder;
		else
			[[unlikely]] return value;
	}

	template <IsSignedIntegral T>
	constexpr T ralign_to(T value, T alignment) noexcept
	{
		if(alignment < 0 && value < 0)
			return -align_to(-value, -alignment);
		else if(alignment > 0 && value < 0)
			return -align_to<T>(-value, alignment);
		else if(alignment < 0)
			alignment = -alignment;
		else if(alignment <= T{1})
			return value;

		T remainder = value % alignment;

		if(remainder != 0)
			[[likely]] return value - remainder;
		else
			[[unlikely]] return value;
	}

	/**
	 * \brief Calculates the max amount value fits in target without overflowing
	 *
	 * \param value
	 * \param target
	 * \return constexpr auto
	 */
	template <IsUnsignedIntegral T>
	constexpr T greatest_contained_count(T value, T target) noexcept
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
