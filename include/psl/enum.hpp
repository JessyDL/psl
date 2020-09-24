#pragma once
#include <type_traits>

namespace psl
{
	template <typename T>
	concept IsEnumClass = std::is_enum_v<T> && !std::is_convertible_v<T, std::underlying_type_t<T>>;

	/**
	 * \brief Defines the set of operations that can be applied to the given enum
	 *
	 */
	enum class enum_ops_t : unsigned char
	{
		/**
		 * \brief No operations permitted
		 *
		 */
		NONE = 0,
		/**
		 * \brief bit operators are permitted
		 * \details operators `&`, `|`, `^`, `~`, `&=`, `|=`, and `^=`
		 *
		 */
		BIT = 1 << 0,
		/**
		 * \brief shift related operators are permitted
		 * \details operators `<<`, `>>`, `<<=`, and `>>=`
		 *
		 */
		SHIFT = 1 << 1,
		/**
		 * \brief remaining arithmetic operators that do not fall in the previous categories
		 * \details operators `+`, `-`, `/`, `*`, `%`, `+=`, `-=`, `/=`, `*=`, and `%=`, for both the enum's underlaying
		 * type as well as any type that can normally do these actions with the underlaying type
		 *
		 */
		ARITHMETIC = 1 << 2,
		/**
		 * \brief defines the logical `bool operator&&` (AND) to return true when the left-hand side satisfies the flag
		 * of the right-hand side
		 * \warning do take special care when enabling this as it changes the behaviour of the
		 * logical and operator, which might be confusing
		 *
		 */
		LOGICAL = 1 << 3
	};
	namespace config
	{
		/**
		 * \brief Customization point, override for your given enum to allow those specific operations to be applied
		 *
		 * \tparam T Any enum class
		 */
		template <IsEnumClass T>
		constexpr enum_ops_t enable_enum_ops = enum_ops_t::NONE;

		template <>
		constexpr auto enable_enum_ops<enum_ops_t> = enum_ops_t::BIT;
	} // namespace config

	/**
	 * \brief True for any enum class that has bit arithmetic operations
	 *
	 * \tparam T
	 */
	template <typename T>
	concept HasEnumBitOps = IsEnumClass<T>&& static_cast<enum_ops_t>(
								static_cast<std::underlying_type_t<enum_ops_t>>(config::enable_enum_ops<T>) &
								static_cast<std::underlying_type_t<enum_ops_t>>(enum_ops_t::BIT)) == enum_ops_t::BIT;
	template <typename T>
	concept HasEnumShiftOps =
		IsEnumClass<T>&& static_cast<enum_ops_t>(
			static_cast<std::underlying_type_t<enum_ops_t>>(config::enable_enum_ops<T>) &
			static_cast<std::underlying_type_t<enum_ops_t>>(enum_ops_t::SHIFT)) == enum_ops_t::SHIFT;
	template <typename T>
	concept HasEnumArithmeticOps =
		IsEnumClass<T>&& static_cast<enum_ops_t>(
			static_cast<std::underlying_type_t<enum_ops_t>>(config::enable_enum_ops<T>) &
			static_cast<std::underlying_type_t<enum_ops_t>>(enum_ops_t::ARITHMETIC)) == enum_ops_t::ARITHMETIC;
	template <typename T>
	concept HasEnumLogicalOps =
		IsEnumClass<T>&& static_cast<enum_ops_t>(
			static_cast<std::underlying_type_t<enum_ops_t>>(config::enable_enum_ops<T>) &
			static_cast<std::underlying_type_t<enum_ops_t>>(enum_ops_t::LOGICAL)) == enum_ops_t::LOGICAL;

} // namespace psl

template <psl::HasEnumBitOps T>
[[nodiscard]] constexpr T operator|(T const a, T const b) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) | static_cast<I>(b));
}

template <psl::HasEnumBitOps T>
[[nodiscard]] constexpr T operator&(T const a, T const b) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) & static_cast<I>(b));
}

template <psl::HasEnumBitOps T>
[[nodiscard]] constexpr T operator^(T const a, T const b) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) ^ static_cast<I>(b));
}

template <psl::HasEnumBitOps T>
constexpr T& operator|=(T& a, T const b) noexcept
{
	return a = a | b;
}

template <psl::HasEnumBitOps T>
constexpr T& operator&=(T& a, T const b) noexcept
{
	return a = a & b;
}

template <psl::HasEnumBitOps T>
constexpr T& operator^=(T& a, T const b) noexcept
{
	return a = a ^ b;
}

template <psl::HasEnumLogicalOps T>
constexpr bool operator&&(T const a, T const b) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) & static_cast<I>(b)) == b;
}

template <psl::HasEnumBitOps T>
[[nodiscard]] constexpr T operator~(T const a) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(~static_cast<I>(a));
}

template <psl::HasEnumShiftOps T>
[[nodiscard]] constexpr T operator<<(T const a, size_t pos) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) << pos);
}

template <psl::HasEnumShiftOps T>
[[nodiscard]] constexpr T operator>>(T const a, size_t pos) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) >> pos);
}

template <psl::HasEnumShiftOps T>
constexpr T& operator<<=(T& a, size_t pos) noexcept
{
	return a = a << pos;
}

template <psl::HasEnumShiftOps T>
constexpr T& operator>>=(T& a, size_t pos) noexcept
{
	return a = a >> pos;
}

template <psl::HasEnumArithmeticOps T>
[[nodiscard]] constexpr T operator+(T const a, T const b) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) + static_cast<I>(b));
}

template <psl::HasEnumArithmeticOps T>
[[nodiscard]] constexpr T operator-(T const a, T const b) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) - static_cast<I>(b));
}

template <psl::HasEnumArithmeticOps T>
[[nodiscard]] constexpr T operator/(T const a, T const b) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) / static_cast<I>(b));
}

template <psl::HasEnumArithmeticOps T>
[[nodiscard]] constexpr T operator*(T const a, T const b) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) * static_cast<I>(b));
}

template <psl::HasEnumArithmeticOps T>
[[nodiscard]] constexpr T operator%(T const a, T const b) noexcept
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) % static_cast<I>(b));
}

template <psl::HasEnumArithmeticOps T, typename Y>
[[nodiscard]] constexpr T operator+(T const a, Y const b) noexcept requires requires(std::underlying_type_t<T> a, Y b)
{
	a + b;
}
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) + b);
}

template <psl::HasEnumArithmeticOps T, typename Y>
[[nodiscard]] constexpr T operator-(T const a, Y const b) noexcept requires requires(std::underlying_type_t<T> a, Y b)
{
	a - b;
}
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) - b);
}

template <psl::HasEnumArithmeticOps T, typename Y>
[[nodiscard]] constexpr T operator/(T const a, Y const b) noexcept requires requires(std::underlying_type_t<T> a, Y b)
{
	a / b;
}
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) / b);
}

template <psl::HasEnumArithmeticOps T, typename Y>
[[nodiscard]] constexpr T operator*(T const a, Y const b) noexcept requires requires(std::underlying_type_t<T> a, Y b)
{
	a* b;
}
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) * b);
}

template <psl::HasEnumArithmeticOps T, typename Y>
[[nodiscard]] constexpr T operator%(T const a, Y const b) noexcept requires requires(std::underlying_type_t<T> a, Y b)
{
	a % b;
}
{
	using I = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<I>(a) % b);
}

template <psl::HasEnumArithmeticOps T, typename Y>
constexpr T operator+=(T& a, Y const b) noexcept
{
	return a = a + b;
}

template <psl::HasEnumArithmeticOps T, typename Y>
constexpr T operator-=(T& a, Y const b) noexcept
{
	return a = a - b;
}

template <psl::HasEnumArithmeticOps T, typename Y>
constexpr T operator/=(T& a, Y const b) noexcept
{
	return a = a / b;
}

template <psl::HasEnumArithmeticOps T, typename Y>
constexpr T operator*=(T& a, Y const b) noexcept
{
	return a = a * b;
}

template <psl::HasEnumArithmeticOps T, typename Y>
constexpr T operator%=(T& a, Y const b) noexcept
{
	return a = a % b;
}