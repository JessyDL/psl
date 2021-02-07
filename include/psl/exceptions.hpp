#pragma once
#include <stdexcept>
#include <string>
#include <assert.h>
#include <psl/details/source_location.hpp>
#include <psl/details/fixed_ascii_string.hpp>
#include <psl/config.hpp>

namespace psl
{
	class exception : public std::exception
	{
	  public:
		exception(std::string_view message, const source_location& location = source_location::current())
		{
			m_What.append("at: ");
			m_What.append(location.file_name());
			m_What.append(":");
			m_What.append(std::to_string(location.line()));
			m_What.append("\n");
			m_What.append(message);
		}

		exception(const exception& other) noexcept = default;
		exception(exception&& other) noexcept	  = default;
		exception& operator=(const exception& other) noexcept = default;
		exception& operator=(exception&& other) noexcept = default;
		virtual ~exception()							 = default;

		const char* what() const noexcept override { return m_What.c_str(); };

	  private:
		std::string m_What{};
	};

	template <fixed_ascii_string Message>
	class static_exception : public exception
	{
	  public:
		static_exception(const source_location& location = source_location::current()) : exception(Message, location) {}
		virtual ~static_exception() = default;
	};

	/**
	 * \brief Exception class to notify the user of implementation errors.
	 * \details Gets thrown in instances where the user has implemented an extension or feature incorrectly.
	 * When you see this exception being raised, read the documentation, or inspect the code surrounding the
	 * exception location for further information on how to resolve the issue.
	 *
	 */
	class implementation_error : exception
	{
	  public:
		implementation_error(const std::string& message, const source_location& location = source_location::current())
			: exception(message, location)
		{}
		implementation_error(const implementation_error& other) noexcept = default;
		implementation_error(implementation_error&& other) noexcept		 = default;
		implementation_error& operator=(const implementation_error& other) noexcept = default;
		implementation_error& operator=(implementation_error&& other) noexcept = default;
		virtual ~implementation_error()										   = default;
	};

	/**
	 * @brief Exception class signifying a feature that is not yet implemented.
	 * @details Debug exception type to signify a work-in-progress feature/codepath that should not be used yet.
	 * Optionally will contain an issue number that links to the project management tool.
	 */
	template <size_t issue>
	class not_implemented : exception
	{
	  public:
		not_implemented(const source_location& location = source_location::current())
			: exception("https://github.com/JessyDL/psl/issues/" + std::to_string(issue), location)
		{}
		not_implemented(const not_implemented& other) noexcept = default;
		not_implemented(not_implemented&& other) noexcept	  = default;
		not_implemented& operator=(const not_implemented& other) noexcept = default;
		not_implemented& operator=(not_implemented&& other) noexcept = default;
		virtual ~not_implemented()									 = default;
	};

	/**
	 * \brief bad access exception for specific types.
	 *
	 * \tparam T
	 * \todo second template argument should be a message string
	 */
	template <typename T, fixed_ascii_string Message>
	class bad_access : exception
	{
	  public:
		bad_access(const source_location& location = source_location::current()) : exception(Message, location){};
		virtual ~bad_access() = default;
	};

	namespace _priv
	{
		template <typename Exception, typename... Ts>
		requires std::is_same_v<std::conditional_t<config::exceptions, void, int>, void> constexpr void
		throw_if_needed(bool condition, Ts&&... args) noexcept(false)
		{
			throw_exception<Exception>(condition, std::forward<Ts>(args)...);
		}

		template <typename Exception, typename... Ts>
		constexpr void throw_exception(bool condition, Ts&&... args) noexcept(false)
		{
			if(condition)
			{
				throw Exception{std::forward<Ts>(args)...};
			}
		}

		template <typename Exception, typename... Ts>
		requires std::is_same_v<std::conditional_t<config::exceptions, void, int>, int> constexpr void
		throw_if_needed(bool condition, Ts&&... args) noexcept(true)
		{
			if(std::is_constant_evaluated()) throw_exception<Exception>(condition, std::forward<Ts>(args)...);

			if constexpr(config::exceptions_as_asserts || psl::config::asserts)
			{
				assert(condition && (args && ...));
			}
		}
	} // namespace _priv
} // namespace psl

#define PSL_ASSERT(expr, ...)                                                                                          \
	if constexpr(psl::config::exceptions_as_asserts || psl::config::asserts) assert(!!(expr)__VA_OPT__(&&) __VA_ARGS__)

#define PSL_EXCEPT(exception_type, ...) psl::_priv::throw_if_needed<exception_type>(true __VA_OPT__(, ) __VA_ARGS__)


#define PSL_EXCEPT_IF(expr, exception_type, ...)                                                                       \
	psl::_priv::throw_if_needed<exception_type>(expr __VA_OPT__(, ) __VA_ARGS__)

#define PSL_CONTRACT_EXCEPT_IF(expr, ...)                                                                              \
	if constexpr(psl::config::implementation_exceptions)                                                               \
	{                                                                                                                  \
		if(!!(expr)) throw psl::implementation_error(__VA_ARGS__);                                                     \
	}                                                                                                                  \
	else                                                                                                               \
	{                                                                                                                  \
		if(std::is_constant_evaluated() && !!(expr)) throw psl::implementation_error(__VA_ARGS__);                     \
	}                                                                                                                  \
	if constexpr(psl::config::implementation_asserts) assert(!(expr)__VA_OPT__(&&) __VA_ARGS__)

#define __STRINGIFY(TEXT) #TEXT
#define __WARNING(TEXT) __STRINGIFY(GCC warning TEXT)
#define WARNING(VALUE) __WARNING(__STRINGIFY(N = VALUE))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define PSL_NOT_IMPLEMENTED_(issue) "NotImplemented: https://github.com/JessyDL/psl/issues/" STR(issue)

#define PSL_NOT_IMPLEMENTED(issue)                                                                                     \
	if constexpr(psl::config::exceptions)                                                                              \
	{                                                                                                                  \
		throw psl::not_implemented<issue>();                                                                           \
	}                                                                                                                  \
	else                                                                                                               \
	{                                                                                                                  \
		if(std::is_constant_evaluated()) throw psl::not_implemented<issue>();                                          \
	}                                                                                                                  \
	if constexpr(psl::config::exceptions_as_asserts || psl::config::asserts)                                           \
		assert(!"NotImplemented: https://github.com/JessyDL/psl/issues/" #issue);                                      \
	std::exit(issue)

#undef STR
#undef STR_HELPER
#undef __WARNING
#undef __STRINGIFY
