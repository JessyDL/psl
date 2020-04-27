#pragma once
#include <string>
#include <string_view>
#include <type_traits>

namespace psl
{
	template <typename T>
	concept IsString = requires(T t)
	{
		std::begin(t);
		std::end(t);
		std::is_convertible_v<std::remove_cvref_t<decltype(t[0])>, char8_t>;
	};

	template <typename T>
	concept IsStringView = IsString<T> && !requires(T t)
	{
		t.reserve(0);
	};

	using string	   = std::u8string;
	using string_view  = std::u8string_view;
	using pstring	  = std::string;
	using pstring_view = std::string_view;
} // namespace psl