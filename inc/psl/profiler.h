#pragma once

#if __has_include(<source_location>)
#include <source_location>
#else
#include <experimental/source_location>
#endif

#include <chrono>
#include "psl/log.h"
#include "psl/array.h"
#include "psl/sinks/stream.h"

namespace psl
{
	class profile_scope;

	class profiler
	{
#if __has_include(<source_location>)
		using source_location = std::source_location;
#else
		using source_location = std::experimental::source_location;
#endif
	  public:
		profiler()
		{
			profile_log.add<psl::ostream>(std::cout);
			profile_log.formatter([this](const details::log_message& message) -> psl::string {
				return fmt::format(u8"{:<{}}{}", u8' ', m_Depth * 4, message.message);
			});
		}
		static profiler& current()
		{
			static profiler prof{};
			return prof;
		}

		profile_scope& begin_scope(profile_scope& scope);
		void end_scope(const profile_scope& scope);

		size_t m_Depth{0};
		psl::logger profile_log{};
	};


	class profile_scope
	{
		friend class profiler;
#if __has_include(<source_location>)
		using source_location = std::source_location;
#else
		using source_location = std::experimental::source_location;
#endif
	  public:
		profile_scope(profiler& prof			 = profiler::current(),
					  const source_location& loc = source_location::current()) noexcept
			: m_Profiler(prof), m_Location(loc), m_Timestamp(std::chrono::high_resolution_clock::now())
		{
			m_Profiler.begin_scope(*this);
		};

		~profile_scope() { m_Profiler.end_scope(*this); }

	  private:
		profiler& m_Profiler;
		source_location m_Location;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Timestamp;
	};


	profile_scope& profiler::begin_scope(profile_scope& scope)
	{
		m_Depth++;
		return scope;
	}
	void profiler::end_scope(const profile_scope& scope)
	{
		--m_Depth;
		auto duration = std::chrono::high_resolution_clock::now() - scope.m_Timestamp;
		profile_log.log<u8"profiler">(u8"{} {} - {}:{}", psl::string{(const char8_t*)scope.m_Location.function_name()},
									  duration, psl::string{(const char8_t*)scope.m_Location.file_name()},
									  scope.m_Location.line());
	}
} // namespace psl