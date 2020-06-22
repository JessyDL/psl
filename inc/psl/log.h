#pragma once
#include <vector>
#include <chrono>
#include <thread>

#if __has_include(<source_location>)
#include <source_location>
#else
#include <experimental/source_location>
#endif

#include "fmt/format.h"
#include <fmt/chrono.h>
#include "psl/sinks/sink.h"
#include "psl/thread.h"
#include "psl/config.h"
#include "psl/string.h"

namespace psl
{
	namespace details
	{
		struct log_message
		{
#if __has_include(<source_location>)
			using source_location = std::source_location;
#else
			using source_location = std::experimental::source_location;
#endif
			std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
			psl::string_view thread;
			psl::string message;
			const source_location& location;
			psl::string_view level;
			psl::string preformat;
		};
	} // namespace details

	class logger
	{
#if __has_include(<source_location>)
		using source_location = std::source_location;
#else
		using source_location = std::experimental::source_location;
#endif
	  public:
		logger(size_t flush_message_treshold					 = 1,
			   std::chrono::duration<double> flush_time_treshold = std::chrono::duration<double>{1.0}) noexcept
			: m_FlushTimeThreshold(flush_time_treshold),
			  m_MessageTreshold(std::max<size_t>(flush_message_treshold, 1u)){};
		~logger() noexcept { flush(); };
		logger(const logger& other) noexcept = delete;
		logger(logger&& other) noexcept		 = default;
		logger& operator=(const logger& other) noexcept = delete;
		logger& operator=(logger&& other) noexcept = default;

		template <psl::details::fixed_string Level, IsStringifyAble Arg0>
		void log(Arg0&& arg0, const source_location& location = source_location::current())
		{
			log_impl<Level, Arg0>(std::forward<Arg0>(arg0), location);
		}

		template <psl::details::fixed_string Level, IsStringifyAble Arg0, IsStringifyAble Arg1>
		void log(Arg0&& arg0, Arg1&& arg1, const source_location& location = source_location::current())
		{
			log_impl<Level, Arg0, Arg1>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), location);
		}

		template <psl::details::fixed_string Level, IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2>
		void log(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, const source_location& location = source_location::current())
		{
			log_impl<Level, Arg0, Arg1, Arg2>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
											  std::forward<Arg2>(arg2), location);
		}

		template <psl::details::fixed_string Level, IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2,
				  IsStringifyAble Arg3>
		void log(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3,
				 const source_location& location = source_location::current())
		{
			log_impl<Level, Arg0, Arg1, Arg2, Arg3>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
													std::forward<Arg2>(arg2), std::forward<Arg3>(arg3), location);
		}

		template <psl::details::fixed_string Level, IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2,
				  IsStringifyAble Arg3, IsStringifyAble Arg4>
		void log(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4,
				 const source_location& location = source_location::current())
		{
			log_impl<Level, Arg0, Arg1, Arg2, Arg3, Arg4>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
														  std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
														  std::forward<Arg4>(arg4), location);
		}

		template <psl::details::fixed_string Level, IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2,
				  IsStringifyAble Arg3, IsStringifyAble Arg4, IsStringifyAble Arg5>
		void log(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5,
				 const source_location& location = source_location::current())
		{
			log_impl<Level, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), location);
		}

		template <psl::details::fixed_string Level, IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2,
				  IsStringifyAble Arg3, IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6>
		void log(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6,
				 const source_location& location = source_location::current())
		{
			log_impl<Level, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), location);
		}

		template <psl::details::fixed_string Level, IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2,
				  IsStringifyAble Arg3, IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6,
				  IsStringifyAble Arg7>
		void log(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7,
				 const source_location& location = source_location::current())
		{
			log_impl<Level, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), std::forward<Arg7>(arg7),
				location);
		}
		/// -----------------------------------
		/// info
		/// -----------------------------------
		template <IsStringifyAble Arg0>
		void info(Arg0&& arg0, const source_location& location = source_location::current())
		{
			log_impl<u8"info", Arg0>(std::forward<Arg0>(arg0), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1>
		void info(Arg0&& arg0, Arg1&& arg1, const source_location& location = source_location::current())
		{
			log_impl<u8"info", Arg0, Arg1>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), location);
		}
		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2>
		void info(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, const source_location& location = source_location::current())
		{
			log_impl<u8"info", Arg0, Arg1, Arg2>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
												 std::forward<Arg2>(arg2), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3>
		void info(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3,
				  const source_location& location = source_location::current())
		{
			log_impl<u8"info", Arg0, Arg1, Arg2, Arg3>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
													   std::forward<Arg2>(arg2), std::forward<Arg3>(arg3), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4>
		void info(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4,
				  const source_location& location = source_location::current())
		{
			log_impl<u8"info", Arg0, Arg1, Arg2, Arg3, Arg4>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
															 std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
															 std::forward<Arg4>(arg4), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5>
		void info(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5,
				  const source_location& location = source_location::current())
		{
			log_impl<u8"info", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6>
		void info(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6,
				  const source_location& location = source_location::current())
		{
			log_impl<u8"info", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6, IsStringifyAble Arg7>
		void info(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6,
				  Arg7&& arg7, const source_location& location = source_location::current())
		{
			log_impl<u8"info", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), std::forward<Arg7>(arg7),
				location);
		}

		/// -----------------------------------
		/// error
		/// -----------------------------------
		template <IsStringifyAble Arg0>
		void error(Arg0&& arg0, const source_location& location = source_location::current())
		{
			log_impl<u8"error", Arg0>(std::forward<Arg0>(arg0), location);
		}


		template <IsStringifyAble Arg0, IsStringifyAble Arg1>
		void error(Arg0&& arg0, Arg1&& arg1, const source_location& location = source_location::current())
		{
			log_impl<u8"error", Arg0, Arg1>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), location);
		}
		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2>
		void error(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, const source_location& location = source_location::current())
		{
			log_impl<u8"error", Arg0, Arg1, Arg2>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
												  std::forward<Arg2>(arg2), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3>
		void error(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3,
				   const source_location& location = source_location::current())
		{
			log_impl<u8"error", Arg0, Arg1, Arg2, Arg3>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
														std::forward<Arg2>(arg2), std::forward<Arg3>(arg3), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4>
		void error(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4,
				   const source_location& location = source_location::current())
		{
			log_impl<u8"error", Arg0, Arg1, Arg2, Arg3, Arg4>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
															  std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
															  std::forward<Arg4>(arg4), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5>
		void error(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5,
				   const source_location& location = source_location::current())
		{
			log_impl<u8"error", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6>
		void error(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6,
				   const source_location& location = source_location::current())
		{
			log_impl<u8"error", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6, IsStringifyAble Arg7>
		void error(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6,
				   Arg7&& arg7, const source_location& location = source_location::current())
		{
			log_impl<u8"error", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), std::forward<Arg7>(arg7),
				location);
		}


		/// -----------------------------------
		/// critical
		/// -----------------------------------
		template <IsStringifyAble Arg0>
		void critical(Arg0&& arg0, const source_location& location = source_location::current())
		{
			log_impl<u8"critical", Arg0>(std::forward<Arg0>(arg0), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1>
		void critical(Arg0&& arg0, Arg1&& arg1, const source_location& location = source_location::current())
		{
			log_impl<u8"critical", Arg0, Arg1>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), location);
		}
		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2>
		void critical(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2,
					  const source_location& location = source_location::current())
		{
			log_impl<u8"critical", Arg0, Arg1, Arg2>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
													 std::forward<Arg2>(arg2), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3>
		void critical(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3,
					  const source_location& location = source_location::current())
		{
			log_impl<u8"critical", Arg0, Arg1, Arg2, Arg3>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
														   std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
														   location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4>
		void critical(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4,
					  const source_location& location = source_location::current())
		{
			log_impl<u8"critical", Arg0, Arg1, Arg2, Arg3, Arg4>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
																 std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
																 std::forward<Arg4>(arg4), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5>
		void critical(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5,
					  const source_location& location = source_location::current())
		{
			log_impl<u8"critical", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6>
		void critical(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6,
					  const source_location& location = source_location::current())
		{
			log_impl<u8"critical", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6, IsStringifyAble Arg7>
		void critical(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6,
					  Arg7&& arg7, const source_location& location = source_location::current())
		{
			log_impl<u8"critical", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), std::forward<Arg7>(arg7),
				location);
		}

		/// -----------------------------------
		/// debug
		/// -----------------------------------
		template <IsStringifyAble Arg0>
		void debug(Arg0&& arg0, const source_location& location = source_location::current())
		{
			log_impl<u8"debug", Arg0>(std::forward<Arg0>(arg0), location);
		}


		template <IsStringifyAble Arg0, IsStringifyAble Arg1>
		void debug(Arg0&& arg0, Arg1&& arg1, const source_location& location = source_location::current())
		{
			log_impl<u8"debug", Arg0, Arg1>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), location);
		}
		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2>
		void debug(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, const source_location& location = source_location::current())
		{
			log_impl<u8"debug", Arg0, Arg1, Arg2>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
												  std::forward<Arg2>(arg2), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3>
		void debug(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3,
				   const source_location& location = source_location::current())
		{
			log_impl<u8"debug", Arg0, Arg1, Arg2, Arg3>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
														std::forward<Arg2>(arg2), std::forward<Arg3>(arg3), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4>
		void debug(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4,
				   const source_location& location = source_location::current())
		{
			log_impl<u8"debug", Arg0, Arg1, Arg2, Arg3, Arg4>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
															  std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
															  std::forward<Arg4>(arg4), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5>
		void debug(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5,
				   const source_location& location = source_location::current())
		{
			log_impl<u8"debug", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6>
		void debug(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6,
				   const source_location& location = source_location::current())
		{
			log_impl<u8"debug", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), location);
		}

		template <IsStringifyAble Arg0, IsStringifyAble Arg1, IsStringifyAble Arg2, IsStringifyAble Arg3,
				  IsStringifyAble Arg4, IsStringifyAble Arg5, IsStringifyAble Arg6, IsStringifyAble Arg7>
		void debug(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6,
				   Arg7&& arg7, const source_location& location = source_location::current())
		{
			log_impl<u8"debug", Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>(
				std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3),
				std::forward<Arg4>(arg4), std::forward<Arg5>(arg5), std::forward<Arg6>(arg6), std::forward<Arg7>(arg7),
				location);
		}

		void flush()
		{
			for(auto sink : m_Sinks)
			{
				sink->log(m_Messages);
			}

			m_Messages.clear();
			m_LastFlush = std::chrono::high_resolution_clock::now();
		}

		void add(std::shared_ptr<sink> sink) { m_Sinks.emplace_back(sink); }
		template <typename Sink, typename... Args>
		std::shared_ptr<sink> add(Args&&... args)
		{
			return m_Sinks.emplace_back(std::make_shared<Sink>(std::forward<Args>(args)...));
		}
		void remove(sink* sink)
		{
			m_Sinks.erase(std::find_if(std::begin(m_Sinks), std::end(m_Sinks),
									   [&sink](const auto& element) { return sink == element.get(); }),
						  std::end(m_Sinks));
		}

		void formatter(std::function<psl::string(const details::log_message&)> format) { m_Formatter = format; }

	  private:
		template <psl::details::fixed_string Level, IsStringifyAble Message, IsStringifyAble... Args>
		void log_impl(Message&& message, Args&&... args, const source_location& location)
		{
			auto current = std::chrono::high_resolution_clock::now();
			m_Messages.emplace_back(current, psl::thread::NAME, fmt::format(message, args...), location,
									static_cast<psl::string_view>(Level));
			m_Messages.back().preformat = m_Formatter(m_Messages.back());
			if(m_Messages.size() >= m_MessageTreshold || m_LastFlush - current >= m_FlushTimeThreshold) flush();
		}

		std::function<psl::string(const details::log_message&)> m_Formatter =
			[](const details::log_message& message) -> psl::string {
			psl::string_view thread = (message.thread.size() > 0) ? message.thread : u8"???";
			if(message.level == u8"critical" || message.level == u8"debug" || message.level == u8"error")
			{
				return fmt::format(u8"{:<12%H:%M:%S} | {:^12} | {:^8} | {}\n{:38}     at: {}:{}",
								   message.timestamp.time_since_epoch(), thread, message.level, message.message, u8" ",
								   psl::string{(const char8_t*)message.location.file_name()}, message.location.line());
			}
			else
			{
				return fmt::format(u8"{:<12%H:%M:%S} | {:^12} | {:^8} | {}", message.timestamp.time_since_epoch(),
								   thread, message.level, message.message);
			}
		};
		std::vector<details::log_message> m_Messages{};
		std::vector<std::shared_ptr<sink>> m_Sinks{};
		std::chrono::time_point<std::chrono::high_resolution_clock> m_LastFlush{};
		std::chrono::duration<double> m_FlushTimeThreshold{1.0};
		size_t m_MessageTreshold{1};
	};

	namespace config
	{
		template <typename T>
		logger default_logger()
		{
			return logger{.flush_message_treshold = 1024};
		}
	} // namespace config

	static logger main_log{config::default_logger<config::default_setting_t>()};
} // namespace psl