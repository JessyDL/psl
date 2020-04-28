#pragma once

#include "psl/config.h"
#include "psl/exception.h"

#include "psl/concepts.h"

namespace psl
{
	/**
	 * \brief describes a range from [begin, end]
	 *
	 * \tparam T any arithmetic type
	 */
	template <psl::IsArithmetic T>
	class range
	{
	  public:
		/**
		 * \param[in] begin beginning of the range
		 * \param[in] end end of the range, this needs to be equal to, or higher than begin
		 */
		constexpr range(T begin, T end) noexcept(!psl::config::exceptions) : begin(begin), end(end)
		{
			PSL_EXCEPT_IF(end < begin, "end is smaller than begin", std::logic_error);
		};
		constexpr ~range()							 = default;
		constexpr range(const range& other) noexcept = default;
		constexpr range(range&& other) noexcept		 = default;
		constexpr range& operator=(const range& other) noexcept = default;
		constexpr range& operator=(range&& other) noexcept = default;

		/**
		 * \returns constexpr T the size of the range
		 */
		constexpr T size() const noexcept { return end - begin; }

		/**
		 * \brief shifts the range to start at the new starting location
		 *
		 * \param[in] newBegin value of the new 'begin'
		 */
		constexpr void shift(T newBegin) noexcept
		{
			if(newBegin < begin)
			{
				end -= begin - newBegin;
			}
			else
			{
				end += newBegin - begin;
			}
			begin = newBegin;
		}

		T begin, end;
	};
} // namespace psl