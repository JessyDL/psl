#pragma once
#include <string>
#include <string_view>
#include <type_traits>

#include "psl/concepts.h"

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

	inline pstring to_pstring(psl::string_view value) noexcept { return {value.begin(), value.end()}; }

	inline bool to_string(const auto& target, psl::string& output)
	{
		static_assert(std::is_same_v<decltype(target), void>);
		return false;
	}

	inline bool to_string(const IsIntegral auto& target, psl::string& output)
	{
		auto res = std::to_string(target);
		output   = psl::string{res.begin(), res.end()};
		return true;
	}

	inline psl::string to_string(const auto& target)
	{
		psl::string res{};
		to_string(target, res);
		return res;
	}

	template <typename T>
	concept IsPStringifyAble = requires(T t)
	{
		std::to_string(t);
	}
	|| std::is_same_v<psl::pstring, T> || std::is_same_v<const char*, T>;

	template <typename T>
	concept IsStringifyAble = requires(T t)
	{
		psl::to_string(t, std::declval<psl::string&>());
	}
	|| std::is_same_v<psl::string, T> || std::is_same_v<const char8_t*, T>;
} // namespace psl