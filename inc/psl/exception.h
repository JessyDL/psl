#pragma once
#include <stdexcept>
#include <string>
#include <assert.h>
#include <experimental/source_location>
#include "psl/config.h"

namespace psl
{
	/**
	 * \brief Exception class to notify the user of implementation errors.
	 * \details Gets thrown in instances where the user has implemented an extension or feature incorrectly.
	 * When you see this exception being raised, read the documentation, or inspect the code surrounding the
	 * exception location for further information on how to resolve the issue.
	 *
	 */
	class implementation_error : std::exception
	{
	  public:
		implementation_error(const std::string& message, const std::experimental::source_location& location =
															 std::experimental::source_location::current())
			: m_What()
		{
			m_What.append("at: ");
			m_What.append(location.file_name());
			m_What.append(":");
			m_What.append(std::to_string(location.line()));
			m_What.append("\n");
			m_What.append(message);
		}
		implementation_error(const implementation_error& other) noexcept = default;
		implementation_error(implementation_error&& other) noexcept		 = default;
		implementation_error& operator=(const implementation_error& other) noexcept = default;
		implementation_error& operator=(implementation_error&& other) noexcept = default;
		~implementation_error()												   = default;

		const char* what() const noexcept override { return m_What.c_str(); };

	  private:
		std::string m_What;
	};

	class not_implemented : std::exception
	{
	  public:
		not_implemented(size_t issue, const std::experimental::source_location& location =
										  std::experimental::source_location::current())
			: m_What(to_message(issue, location))
		{}
		not_implemented(const not_implemented& other) noexcept = default;
		not_implemented(not_implemented&& other) noexcept	  = default;
		not_implemented& operator=(const not_implemented& other) noexcept = default;
		not_implemented& operator=(not_implemented&& other) noexcept = default;
		~not_implemented()											 = default;

		static std::string
		to_message(size_t issue,
				   const std::experimental::source_location& location = std::experimental::source_location::current())
		{
			std::string what{};
			what.append("NotImplemented\nat: ");
			what.append(location.file_name());
			what.append(":");
			what.append(std::to_string(location.line()));
			if(issue != 0)
			{
				what.append("\nhttps://github.com/JessyDL/psl/issues/");
				what.append(std::to_string(issue));
			}
			return what;
		}

		const char* what() const noexcept override { return m_What.c_str(); };

	  private:
		std::string m_What;
	};
} // namespace psl

#define PSL_ASSERT(condition, ...)                                                                                     \
	if constexpr(psl::config::exceptions_as_asserts || psl::config::asserts) assert(!!(false)__VA_OPT__(&&) __VA_ARGS__)

#define PSL_EXCEPT(exception_type, ...)                                                                                \
	if constexpr(psl::config::exceptions)                                                                              \
	{                                                                                                                  \
		throw exception_type(__VA_ARGS__);                                                                             \
	}                                                                                                                  \
	else                                                                                                               \
	{                                                                                                                  \
		if(std::is_constant_evaluated()) throw exception_type(__VA_ARGS__);                                            \
	}                                                                                                                  \
	PSL_ASSERT(false, __VA_ARGS__)


#define PSL_EXCEPT_IF(expr, message, exception_type)                                                                   \
	if constexpr(psl::config::exceptions)                                                                              \
	{                                                                                                                  \
		if(!!(expr)) throw exception_type(message);                                                                    \
	}                                                                                                                  \
	else                                                                                                               \
	{                                                                                                                  \
		if(std::is_constant_evaluated() && !!(expr)) throw exception_type(message);                                    \
	}                                                                                                                  \
	if constexpr(psl::config::exceptions_as_asserts || psl::config::asserts) assert(!(expr) && message)

#define PSL_CONTRACT_EXCEPT_IF(expr, message)                                                                          \
	if constexpr(psl::config::implementation_exceptions)                                                               \
	{                                                                                                                  \
		if(!!(expr)) throw psl::implementation_error(message);                                                         \
	}                                                                                                                  \
	else                                                                                                               \
	{                                                                                                                  \
		if(std::is_constant_evaluated() && !!(expr)) throw psl::implementation_error(message);                         \
	}                                                                                                                  \
	if constexpr(psl::config::implementation_asserts) assert(!(expr) && message)

#define __STRINGIFY(TEXT) #TEXT
#define __WARNING(TEXT) __STRINGIFY(GCC warning TEXT)
#define WARNING(VALUE) __WARNING(__STRINGIFY(N = VALUE))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define PSL_NOT_IMPLEMENTED_(issue) "NotImplemented: https://github.com/JessyDL/psl/issues/" STR(issue)

#define PSL_NOT_IMPLEMENTED(issue)                                                                                     \
	if constexpr(psl::config::exceptions)                                                                              \
	{                                                                                                                  \
		throw psl::not_implemented(issue);                                                                             \
	}                                                                                                                  \
	else                                                                                                               \
	{                                                                                                                  \
		if(std::is_constant_evaluated()) throw psl::not_implemented(issue);                                            \
	}                                                                                                                  \
	if constexpr(psl::config::exceptions_as_asserts || psl::config::asserts)                                           \
		assert(!"NotImplemented: https://github.com/JessyDL/psl/issues/" #issue);                                      \
	std::exit(issue)
