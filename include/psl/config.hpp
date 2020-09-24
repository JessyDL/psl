#pragma once
#include <type_traits>

/**
 * \brief contains all configurations options for PSL
 *
 * \details see psl::config::specialize to see what to specialize and how
 */
namespace psl::config
{
#if defined(DEBUG)
	constexpr bool is_debug = true;
#else
	constexpr bool is_debug = false;
#endif
	constexpr bool is_release = !is_debug;

	/**
	 * \brief used as the type to force specialization of an option
	 *
	 */
	struct default_setting_t
	{};

	/**
	 * \brief exception handling strategy options
	 *
	 */
	enum class exceptions_handling
	{
		EXCEPTIONS = 0,
		ASSERTS	   = 1,
		NONE	   = 2
	};

	/**
	 * \brief controls exception usage in the library (with exception of constant evaluated contexts)
	 *
	 */
	template <typename T>
	struct errors_strategy_t
	{
		constexpr static exceptions_handling value = exceptions_handling::EXCEPTIONS;
	};

	/**
	 * \brief controls assert usage in the library
	 *
	 */
	template <typename T>
	struct use_asserts_t : std::false_type
	{};

	/**
	 * \brief controls exceptions for validating implementations in the library (such as custom
	 * psl::abstract_region implementations)
	 *
	 * \tparam T
	 */
	template <typename T>
	struct implementation_errors_strategy_t
	{
		constexpr static exceptions_handling value = exceptions_handling::EXCEPTIONS;
	};


	constexpr static exceptions_handling errors_strategy_v = errors_strategy_t<default_setting_t>::value;
	constexpr static bool exceptions					   = errors_strategy_v == exceptions_handling::EXCEPTIONS;
	constexpr static bool exceptions_as_asserts			   = errors_strategy_v == exceptions_handling::ASSERTS;

	constexpr static bool asserts = use_asserts_t<default_setting_t>::value;

	/**
	 * \brief tracks implementation errors on the part of the programmer
	 * \details unlike the previous exception handling, which is meant to protect against runtime errors,
	 * this variation is meant to check for errors in contract implementation. Most commonly this is used
	 * when inheriting from polymorphic base types
	 *
	 */
	constexpr static exceptions_handling implementation_errors_strategy_v =
		implementation_errors_strategy_t<default_setting_t>::value;
	constexpr static bool implementation_exceptions =
		implementation_errors_strategy_v == exceptions_handling::EXCEPTIONS;
	constexpr static bool implementation_asserts = implementation_errors_strategy_v == exceptions_handling::ASSERTS;
} // namespace psl::config