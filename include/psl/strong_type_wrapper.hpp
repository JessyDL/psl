#pragma once
#include <type_traits>

namespace psl {
inline namespace details {
	template <typename T, typename Y, typename Op>
	concept SupportsOperation = requires() {
		{ Op {}(T {}, Y {}) } -> std::same_as<T>;
	};

	template <typename T, typename Y>
	concept SupportsAddition = SupportsOperation<T, Y, decltype([](auto&& lhs, auto&& rhs) { return lhs + rhs; })>;
	template <typename T, typename Y>
	concept SupportsAdditionAssignment = SupportsOperation<T, Y, decltype([](auto&& lhs, auto&& rhs) {
															   lhs += rhs;
															   return lhs;
														   })>;
	template <typename T, typename Y>
	concept SupportsSubtraction = SupportsOperation<T, Y, decltype([](auto&& lhs, auto&& rhs) { return lhs - rhs; })>;
	template <typename T, typename Y>
	concept SupportsSubtractionAssignment = SupportsOperation<T, Y, decltype([](auto&& lhs, auto&& rhs) {
																  lhs -= rhs;
																  return lhs;
															  })>;
	template <typename T, typename Y>
	concept SupportsMultiplication =
	  SupportsOperation<T, Y, decltype([](auto&& lhs, auto&& rhs) { return lhs * rhs; })>;
	template <typename T, typename Y>
	concept SupportsMultiplicationAssignment = SupportsOperation<T, Y, decltype([](auto&& lhs, auto&& rhs) {
																	 lhs *= rhs;
																	 return lhs;
																 })>;
	template <typename T, typename Y>
	concept SupportsDivision = SupportsOperation<T, Y, decltype([](auto&& lhs, auto&& rhs) { return lhs / rhs; })>;
	template <typename T, typename Y>
	concept SupportsDivisionAssignment = SupportsOperation<T, Y, decltype([](auto&& lhs, auto&& rhs) {
															   lhs /= rhs;
															   return lhs;
														   })>;
}	 // namespace details

/**
 * @brief Traits object for `psl::strong_type_wrapper_t`, use this as a blueprint for custom operator behaviours.
 *
 * @tparam T
 */
template <typename T>
struct traits_strong_type_wrapper_t {
	constexpr static bool supports_addition					 = SupportsAddition<T, T>;
	constexpr static bool supports_addition_assignment		 = SupportsAdditionAssignment<T, T>;
	constexpr static bool supports_subtraction				 = SupportsSubtraction<T, T>;
	constexpr static bool supports_subtraction_assignment	 = SupportsSubtractionAssignment<T, T>;
	constexpr static bool supports_multiplication			 = SupportsMultiplication<T, T>;
	constexpr static bool supports_multiplication_assignment = SupportsMultiplicationAssignment<T, T>;
	constexpr static bool supports_division					 = SupportsDivision<T, T>;
	constexpr static bool supports_division_assignment		 = SupportsDivisionAssignment<T, T>;
};


template <typename T, typename Traits = traits_strong_type_wrapper_t<T>, typename Tag = decltype([] {})>
class strong_type_wrapper_t;

inline namespace details {
	template <typename T>
	struct is_strong_wrapper_t : std::false_type {};
	template <typename T, typename Tag>
	struct is_strong_wrapper_t<strong_type_wrapper_t<T, Tag>> : std::true_type {};
}	 // namespace details

template <typename T>
concept IsStrongTypeWrapper = is_strong_wrapper_t<std::remove_cvref_t<T>>::value;
/**
 * @brief Allows you to wrap another type as a "strong" type. Useful for generating safe interfaces for code.
 * @details Strong types are types that don't easily decay/convert into other types. This is most useful to
 * guarantee the programmer has less chance of unintentionally doing operations that are deemed "less" safe
 * implicitly.
 * @tparam T The type to wrap.
 * @tparam Traits The traits object (see `psl::traits_strong_type_wrapper_t` for more details) that can
 * override what operations can be done.
 * @tparam Tag Special tag to guarantee unique instances, ignore this.
 */
template <typename T, typename Traits, typename Tag>
class strong_type_wrapper_t {
  public:
	template <typename... Ys>
	constexpr strong_type_wrapper_t(Ys&&... values) : _m_Value(std::forward<Ys>(values)...) {};
	constexpr strong_type_wrapper_t(strong_type_wrapper_t const&)			 = default;
	constexpr strong_type_wrapper_t& operator=(strong_type_wrapper_t const&) = default;
	constexpr strong_type_wrapper_t(strong_type_wrapper_t&&)				 = default;
	constexpr strong_type_wrapper_t& operator=(strong_type_wrapper_t&&)		 = default;
	constexpr auto operator*() const noexcept -> T const& { return _m_Value; }
	constexpr auto operator*() noexcept -> T { return _m_Value; }
	constexpr auto weak_value() const noexcept -> T const& { return _m_Value; }
	constexpr auto weak_value() noexcept -> T { return _m_Value; }

	constexpr strong_type_wrapper_t operator+(strong_type_wrapper_t const& rhs) const noexcept
		requires Traits::supports_addition
	{
		return strong_type_wrapper_t {_m_Value + *rhs};
	};
	constexpr strong_type_wrapper_t& operator+=(strong_type_wrapper_t const& rhs) noexcept
		requires Traits::supports_addition_assignment
	{
		_m_Value += *rhs;
		return *this;
	};

	constexpr strong_type_wrapper_t operator-(strong_type_wrapper_t const& rhs) const noexcept
		requires Traits::supports_subtraction
	{
		return strong_type_wrapper_t {_m_Value - *rhs};
	};
	constexpr strong_type_wrapper_t& operator-=(strong_type_wrapper_t const& rhs) noexcept
		requires Traits::supports_assignment
	{
		_m_Value -= *rhs;
		return *this;
	};

	constexpr strong_type_wrapper_t operator*(strong_type_wrapper_t const& rhs) const noexcept
		requires Traits::supports_multiplication
	{
		return strong_type_wrapper_t {_m_Value * *rhs};
	};
	constexpr strong_type_wrapper_t& operator*=(strong_type_wrapper_t const& rhs) noexcept
		requires Traits::supports_multiplication_assignment
	{
		_m_Value *= *rhs;
		return *this;
	};

	constexpr strong_type_wrapper_t operator/(strong_type_wrapper_t const& rhs) const noexcept
		requires Traits::supports_division
	{
		return strong_type_wrapper_t {_m_Value / *rhs};
	};
	constexpr strong_type_wrapper_t& operator/=(strong_type_wrapper_t const& rhs) noexcept
		requires Traits::supports_division_assignment
	{
		_m_Value /= *rhs;
		return *this;
	};

	T _m_Value;
};
}	 // namespace psl