#include "psl/concepts.h"

namespace psl
{
	template <IsIntegral T>
	constexpr inline auto align_to(T value, T alignment) noexcept
	{
		if(alignment <= T{1}) return value;
		auto remainder = value % alignment;
		return (remainder) ? value + (alignment - remainder) : value;
	}
} // namespace psl