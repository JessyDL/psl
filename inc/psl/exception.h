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

	template <typename T>
	struct meta_t
	{
		using type = T;
	};
} // namespace psl

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
