#pragma once
#include <type_traits>

namespace std
{
	template <typename T>
	class optional;
}

namespace psl
{
	/**
	 * \brief Used as a meta type to pass around type info
	 *
	 * \tparam T
	 */
	template <typename T>
	struct type_t
	{
		using type = T;
	};
	namespace detail
	{
		template <typename T>
		struct is_optional : std::false_type
		{};

		template <typename T>
		struct is_optional<std::optional<T>> : std::true_type
		{};
	} // namespace detail

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

	template <typename T>
	concept IsOptional = detail::is_optional<T>::value;
} // namespace psl