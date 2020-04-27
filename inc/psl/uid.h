#pragma once
#include <cstdint>
#include <stdexcept>

#include "psl/static_array.h"
#include "psl/string.h"
#include "psl/details/metaprogramming.h"
#include "psl/details/fixed_string.h"

namespace std
{
	template <typename T>
	struct hash;
} // namespace std

namespace psl
{
	class uuidv4 final
	{
	  public:
		using value_type = psl::static_array<uint8_t, 16>;

		friend struct std::hash<uuidv4>;

		constexpr uuidv4() noexcept					   = default;
		constexpr ~uuidv4()							   = default;
		constexpr uuidv4(const uuidv4& other) noexcept = default;
		constexpr uuidv4(uuidv4&& other) noexcept	  = default;
		constexpr uuidv4& operator=(const uuidv4& other) noexcept = default;
		constexpr uuidv4& operator=(uuidv4&& other) noexcept = default;

		template <IsStringView T>
		constexpr uuidv4(T uid) noexcept : m_Data({0})
		{
			static_assert(sizeof(decltype(uid[0])) == sizeof(char8_t), "can only work with 8 bit wide characters");
			if(!is_convertible(uid.data(), uid.size()))
			{
				return;
			}

			auto* text = uid.data();

			constexpr auto parse = [](const auto* text) {
				uint8_t result{};
				for(size_t i = 0; i < 2; ++i)
				{
					auto character = text[i];
					int res{};
					if('0' <= character && character <= '9')
						res = character - '0';
					else if('a' <= character && character <= 'f')
						res = 10 + character - 'a';
					else
						res = 10 + character - 'A';

					result |= res << (4 * (1 - i));
				}
				return result;
			};

			auto res_offset = 0;
			for(size_t i = 0; i < 4; ++i)
			{
				m_Data[res_offset++] = parse(text);
				text += 2;
			}
			text += 1;
			for(size_t i = 0; i < 2; ++i)
			{
				m_Data[res_offset++] = parse(text);
				text += 2;
			}
			text += 1;
			for(size_t i = 0; i < 2; ++i)
			{
				m_Data[res_offset++] = parse(text);
				text += 2;
			}
			text += 1;
			for(size_t i = 0; i < 2; ++i)
			{
				m_Data[res_offset++] = parse(text);
				text += 2;
			}
			text += 1;

			for(size_t i = 0; i < 6; ++i)
			{
				m_Data[res_offset++] = parse(text);
				text += 2;
			}
		}

		inline constexpr bool operator==(const uuidv4& rhs) const noexcept { return m_Data == rhs.m_Data; }
		inline constexpr bool operator!=(const uuidv4& rhs) const noexcept { return m_Data != rhs.m_Data; }
		inline constexpr bool operator<(const uuidv4& rhs) const noexcept { return m_Data < rhs.m_Data; }
		inline constexpr bool operator<=(const uuidv4& rhs) const noexcept { return m_Data <= rhs.m_Data; }
		inline constexpr bool operator>(const uuidv4& rhs) const noexcept { return m_Data > rhs.m_Data; }
		inline constexpr bool operator>=(const uuidv4& rhs) const noexcept { return m_Data >= rhs.m_Data; }

		/// \brief checks if the held uuidv4 is valid.
		/// \returns true in case the held uuidv4 is valid.
		constexpr explicit operator bool() const
		{
			for(const auto& element : m_Data)
				if(element != 0) return true;
			return false;
		}

		/// \brief generates a uuidv4.
		/// \returns a valid uuidv4.
		static uuidv4 generate() noexcept;

	  private:
		template <auto T, size_t index, size_t length>
		consteval bool is_valid_character() noexcept
		{
			if(length != 38 && length != 36)
				return false;
			else if(length == 38)
			{
				if(index == 0) return T == '{';
				if(index == length - 1) return T == '}';

				if(index == 9 || index == 14 || index == 19 || index == 24) return T == '-';
			}
			else
			{
				if(index == 8 || index == 13 || index == 18 || index == 23) return T == '-';
			}
			return ('0' <= T && T <= '9') || ('a' <= T && T <= 'f') || ('A' <= T && T <= 'F');
		}

		template <typename T, size_t... Is>
		consteval bool is_valid_character(T uid) noexcept
		{
			return metaprogramming::all_of([&uid]() { return is_valid_character<uid[Is], Is, sizeof...(Is)>(); }...);
		}

	  public:
		template <IsStringView T>
		constexpr static bool is_convertible(T view) noexcept
		{
			return is_convertible(view.data(), view.size());
		}


	  private:
		template <typename T>
		constexpr static bool is_convertible(const T* str, size_t size) noexcept
		{
			constexpr const size_t short_size = 36;
			constexpr const size_t long_size  = 38;

			if(size != short_size && size != long_size)
			{
				// incorrect size
				return false;
			}
			const size_t offset = (str[0] == '{') ? 1 : 0;
			if(size == long_size)
			{
				if(str[0] == '{')
				{
					if(str[size - 1] != '}')
					{
						// expected a closing for the long str form
						return false;
					}
				}
				else
				{
					// expected an opening for the long str form
					return false;
				}
			}


			if((str[8 + offset] != '-') || (str[13 + offset] != '-') || (str[18 + offset] != '-') ||
			   (str[23 + offset] != '-'))
			{
				return false;
			}

			constexpr auto parse = [](const auto character) -> bool {
				return ('0' <= character && character <= '9') || ('a' <= character && character <= 'f') ||
					   ('A' <= character && character <= 'F');
			};
			return std::all_of(std::next(str, offset), std::next((str), 8 + offset), parse) &&
				   std::all_of(std::next(str, 9 + offset), std::next((str), 1 + offset), parse) &&
				   std::all_of(std::next((str), 14 + offset), std::next((str), 18 + offset), parse) &&
				   std::all_of(std::next((str), 19 + offset), std::next((str), 23 + offset), parse) &&
				   std::all_of(std::next((str), 24 + offset), std::prev((str), size - offset), parse);
		}

		constexpr void clear() noexcept { m_Data = {}; }

	  private:
		template <typename T>
		constexpr auto pack_pair(const T* token)
		{
			uint8_t result{};
			for(size_t i = 0; i < 2; ++i)
			{
				auto character = token[i];
				int res{};
				if('0' <= character && character <= '9')
					res = character - '0';
				else if('a' <= character && character <= 'f')
					res = 10 + character - 'a';
				else
					res = 10 + character - 'A';

				result |= res << (4 * (1 - i));
			}
			return result;
		}
		template <details::fixed_string<36> UID, size_t offset>
		inline static consteval auto pack_pair()
		{
			uint8_t result{};
			for(size_t i = 0; i < 2; ++i)
			{
				auto character = UID[i + offset];
				int res{};
				if('0' <= character && character <= '9')
					res = character - '0';
				else if('a' <= character && character <= 'f')
					res = 10 + character - 'a';
				else
					res = 10 + character - 'A';

				result |= res << (4 * (1 - i));
			}
			return result;
		}

		template <details::fixed_string<36> UID, size_t offset, size_t range = 1>
		inline static constexpr bool valid_pair()
		{
			for(size_t i = offset; i < (2 * range) + offset; ++i)
			{
				if(('0' <= UID[i] && UID[i] <= '9') || ('a' <= UID[i] && UID[i] <= 'f') ||
				   ('A' <= UID[i] && UID[i] <= 'F'))
					continue;
				else
					return false;
			}
			return true;
		}

		template <details::fixed_string<36> UID>
		inline static consteval uuidv4 from_short_format_string()
		{
			static_assert((UID[8] == '-') && (UID[13] == '-') && (UID[18] == '-') && (UID[23] == '-'));
			static_assert(valid_pair<UID, 0, 4>() && valid_pair<UID, 9, 2>() && valid_pair<UID, 14, 2>() &&
						  valid_pair<UID, 19, 2>() && valid_pair<UID, 24, 6>());

			uuidv4 res{};

			res.m_Data[0] = pack_pair<UID, 0>();
			res.m_Data[1] = pack_pair<UID, 2>();
			res.m_Data[2] = pack_pair<UID, 4>();
			res.m_Data[3] = pack_pair<UID, 6>();
			// -
			res.m_Data[4] = pack_pair<UID, 9>();
			res.m_Data[5] = pack_pair<UID, 11>();
			// -
			res.m_Data[6] = pack_pair<UID, 14>();
			res.m_Data[7] = pack_pair<UID, 16>();
			// -
			res.m_Data[8] = pack_pair<UID, 19>();
			res.m_Data[9] = pack_pair<UID, 21>();
			// -
			res.m_Data[10] = pack_pair<UID, 24>();
			res.m_Data[11] = pack_pair<UID, 26>();
			res.m_Data[12] = pack_pair<UID, 28>();
			res.m_Data[13] = pack_pair<UID, 30>();
			res.m_Data[14] = pack_pair<UID, 32>();
			res.m_Data[15] = pack_pair<UID, 34>();

			return res;
		}
		template <details::fixed_string<38> UID>
		inline static consteval uuidv4 from_long_format_string()
		{
			constexpr auto size = UID.size();
			constexpr auto str  = &UID.buf[0];

			static_assert(str[0] == '{' && str[size - 1] == '}',
						  "incorrect format, should open and close with braces '{}'");

			return from_short_format_string<UID.substr<1, 37>()>();
		}


	  public:
		template <details::fixed_string UID>
		inline static consteval uuidv4 from_string()
		{
			static_assert(UID.size() == 36 || UID.size() == 38);

			if constexpr(UID.size() == 36)
				return from_short_format_string<UID>();
			else if constexpr(UID.size() == 38)
				return from_long_format_string<UID>();
			else
				return {};
		}

		psl::string to_string() const noexcept;

	  private:
		value_type m_Data;
	};

	using uid = uuidv4;

	namespace literals
	{
		consteval uid operator"" _uid(const char* text, std::size_t size)
		{
			if(!uuidv4::is_convertible(psl::pstring_view{text, size}))
				throw std::logic_error("the given string is not a valid UID");
			return uid(psl::pstring_view{text, size});
		}

		consteval uid operator"" _uid(const char8_t* text, std::size_t size)
		{
			if(!uuidv4::is_convertible(psl::string_view{text, size}))
				throw std::logic_error("the given string is not a valid UID");
			return uid(psl::string_view{text, size});
		}
	} // namespace literals

	template <details::fixed_string UID>
	inline consteval auto make_uid()
	{
		return psl::uid::from_string<UID>();
	}
} // namespace psl


// support hashing
namespace std
{
	template <>
	struct hash<psl::uuidv4>
	{
		size_t operator()(const psl::uuidv4& x) const noexcept
		{
			const uint64_t* half = reinterpret_cast<const uint64_t*>(&x.m_Data);
			return half[0] ^ half[1];
		}
	};
} // namespace std