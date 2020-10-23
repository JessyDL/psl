#pragma once

namespace psl
{
	template <typename T>
	concept IsIterator = requires
	{
		T::iterator_category;
	};

	template <typename T>
	concept IsIntegral = std::is_integral_v<T>;

	template <typename T>
	concept IsSignedIntegral = IsIntegral<T>&& std::is_signed_v<T>;

	template <typename T>
	concept IsUnsignedIntegral = IsIntegral<T>&& std::is_unsigned_v<T>;

	template <typename T>
	concept IsArithmetic = std::is_arithmetic_v<T>;

	template <typename T>
	concept IsRange = requires(T t)
	{
		t.begin();
		t.end();
		{
			t.size()
		}
		->IsIntegral;
	};
} // namespace psl