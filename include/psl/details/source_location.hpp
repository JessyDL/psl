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
		struct source_location final
		{
		  private:
#if(__GNUC__ >= 9)
			consteval source_location(const char* file, const char* function, std::uint_least32_t line) noexcept
				: m_Filename(file), m_Function(function), m_Line(line)
			{}
#endif
		  public:
			[[nodiscard]] static consteval source_location current(
#if(__GNUC__ >= 9)
				const char* file = __builtin_FILE(), const char* function = __builtin_FUNCTION(),
				std::uint_least32_t line = __builtin_LINE()
#endif
					) noexcept
			{
#if(__GNUC__ >= 9)
				return source_location{file, function, line};
#else
				return source_location{};
#endif
			};
			constexpr const char* function_name() const noexcept { return m_Function; }
			constexpr const char* file_name() const noexcept { return m_Filename; }
			constexpr std::uint_least32_t column() const noexcept { return 0; }
			constexpr std::uint_least32_t line() const noexcept { return m_Line; }
#if(__GNUC__ >= 9)
			const char* m_Filename{};
			const char* m_Function{};
			std::uint_least32_t m_Line{};
#endif
		};
#endif
	} // namespace details
} // namespace psl