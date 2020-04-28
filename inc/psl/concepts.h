#pragma once
#include <type_traits>

namespace psl
{
	template <typename T>
	concept IsIntegral = std::is_integral_v<T>;

	template <typename T>
	concept IsArithmetic = std::is_arithmetic_v<T>;

	template <typename T>
	concept IsRange = requires(T t)
	{
		t.begin();
		t.end();
	};
} // namespace psl