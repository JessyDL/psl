#pragma once

/*
	replacement std::source_location stub for when the target platform lacks an implementation.
*/
#if __has_include(<source_location>)
#include <source_location>
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
#endif


namespace psl
{
	inline namespace details
	{
#if __has_include(<source_location>)
		using source_location = std::source_location;
#elif __has_include(<experimental/source_location>)
		using source_location = std::experimental::source_location;
#else
		struct source_location
		{
			static consteval source_location current() noexcept { return source_location{}; };
			constexpr const char* function_name() const noexcept { return ""; }
			constexpr const char* file_name() const noexcept { return ""; }
			constexpr std::uint_least32_t column() const noexcept { return 0; }
			constexpr std::uint_least32_t line() const noexcept { return 0; }
		};
#endif
	} // namespace details
} // namespace psl