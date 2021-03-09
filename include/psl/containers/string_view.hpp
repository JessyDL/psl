#pragma once
#include <algorithm>

#include <psl/span.hpp>

namespace psl
{
	// todo move to common functionality when unicode is implemented
	enum class string_encoding
	{
		ASCII,
		UTF8,
	};

	inline namespace details
	{
		template <string_encoding Encoding>
		class string_view_impl;

		template <typename T>
		struct is_string_view : std::false_type
		{};

		template <string_encoding Encoding>
		struct is_string_view<string_view_impl<Encoding>> : std::true_type
		{};


	} // namespace details

	template <typename T>
	concept IsStringView = is_string_view<std::remove_cvref_t<T>>::value;

	template <string_encoding Encoding = string_encoding::ASCII>
	struct string_encoding_character
	{
		using type = char;
	};

	template <string_encoding Encoding = string_encoding::ASCII>
	using string_encoding_character_t = string_encoding_character<Encoding>::type;


	template <string_encoding Encoding>
	constexpr inline size_t length(const string_encoding_character_t<Encoding>* data) noexcept
	{
		size_t res{0};
		while(*(data++) != '\0')
		{
			++res;
		}
		return res;
	}

	constexpr inline auto length(IsStringView auto target) noexcept { return target.length(); }

	constexpr bool starts_with(auto target, auto value) noexcept
	{
		if(target.size() >= value.size())
		{
			using std::begin, std::end;
			return std::equal(begin(value), end(value), begin(target));
		}
		return false;
	}

	constexpr bool ends_with(auto target, auto value) noexcept
	{
		if(target.size() >= value.size())
		{
			using std::rbegin, std::rend;
			return std::equal(rbegin(value), rend(value), rbegin(target));
		}
		return false;
	}

	inline namespace details
	{
		template <string_encoding Encoding>
		class string_view_impl
		{
			using storage_t = span<string_encoding_character_t<Encoding>>;

		  public:
			using char_t		 = string_encoding_character_t<Encoding>;
			using size_type		 = storage_t::size_type;
			using iterator		 = storage_t::iterator;
			using const_iterator = storage_t::const_iterator;

			constexpr string_view_impl() noexcept = default;
			constexpr string_view_impl(const char_t* data) : m_Data(data, psl::length<Encoding>(data)) {}
			constexpr string_view_impl(const char_t* data, size_type size) : m_Data(data, size) {}
			explicit constexpr string_view_impl(const_iterator begin, const_iterator end)
				: m_Data(&(*begin), end - begin)
			{}

			constexpr string_view_impl(const string_view_impl& other) noexcept : m_Data(other.m_Data) {}

			constexpr string_view_impl& operator=(const string_view_impl& other)
			{
				if(this != &other)
				{
					m_Data = other.m_Data;
				}
				return *this;
			}

			constexpr auto size() const noexcept { return m_Data.size(); }
			constexpr auto length() const noexcept { return psl::length<Encoding>(m_Data.data()); }

			constexpr bool starts_with(string_view_impl value) const noexcept
			{
				return psl::starts_with(m_Data, value);
			}
			constexpr bool ends_with(string_view_impl value) const noexcept { return psl::ends_with(m_Data, value); }

			template <size_t N>
			constexpr auto find_first_not_of(const std::array<char_t, N>& values, const_iterator offset) const
				noexcept(!config::exceptions) -> const_iterator
			{
				// todo this is not utf8 safe.
				using std::begin, std::end;
				const auto csize = this->end();
				for(auto i = offset; i != csize; ++i)
				{
					if(std::none_of(begin(values), end(values), [c = *i](auto v) { return c == v; })) return i;
				}
				return this->end();
			}

			template <size_t N>
			constexpr auto find_first_not_of(const std::array<char_t, N>& values) const noexcept(!config::exceptions)
			{
				return find_first_not_of(values, begin());
			}

			template <size_t N>
			constexpr auto find_first_not_of(const std::array<string_view_impl, N>& values, const_iterator offset) const
				noexcept(!config::exceptions) -> const_iterator
			{
				// todo this is not utf8 safe.
				auto remaining_size = m_Data.size() - (offset - this->begin());
				auto csize			= end();
				using std::begin, std::end, std::next;
				for(auto i = offset; i != csize; ++i)
				{
					if(std::none_of(begin(values), end(values), [i, remaining_size](string_view_impl v) {
						   return v.size() <= remaining_size && std::equal(i, next(i, v.size()), begin(v));
					   }))
						return i;
					--remaining_size;
				}
				return this->end();
			}
			template <size_t N>
			constexpr auto find_first_not_of(const std::array<string_view_impl, N>& values) const
				noexcept(!config::exceptions) -> const_iterator
			{
				return find_first_not_of(values, begin());
			}

			constexpr auto find_first_not_of(string_view_impl value, const_iterator offset) const noexcept
				-> const_iterator
			{
				return find_first_not_of(std::array{value}, offset);
			}

			constexpr auto find_first_not_of(string_view_impl value) const noexcept -> const_iterator
			{
				return find_first_not_of(std::array{value}, begin());
			}

			template <size_t N>
			constexpr auto find_first_of(const std::array<string_view_impl, N>& values, const_iterator offset) const
				noexcept(!config::exceptions) -> const_iterator
			{
				auto remaining_size = m_Data.size() - (offset - this->begin());
				auto csize			= end();
				using std::begin, std::end, std::next;
				for(auto i = offset; i != csize; ++i)
				{
					if(std::any_of(begin(values), end(values), [i, remaining_size](string_view_impl v) {
						   return v.size() <= remaining_size && std::equal(i, next(i, v.size()), begin(v));
					   }))
						return i;
					--remaining_size;
				}
				return this->end();
			}

			template <size_t N>
			constexpr auto find_first_of(const std::array<string_view_impl, N>& values) const
				noexcept(!config::exceptions) -> const_iterator
			{
				return find_first_of(values, begin());
			}

			constexpr auto find_first_of(string_view_impl value, const_iterator offset) const noexcept -> const_iterator
			{
				return find_first_of(std::array{value}, offset);
			}

			constexpr auto find_first_of(string_view_impl value) const noexcept -> const_iterator
			{
				return find_first_of(std::array{value}, begin());
			}

			constexpr auto find(string_view_impl value, const_iterator offset) const noexcept -> const_iterator
			{
				return find_first_of(std::array{value}, offset);
			}

			constexpr auto find(string_view_impl value) const noexcept -> const_iterator
			{
				return find_first_of(std::array{value}, begin());
			}

			constexpr auto operator[](size_type index) noexcept { return m_Data[index]; }
			constexpr auto operator[](size_type index) const noexcept { return m_Data[index]; }

			constexpr auto begin() noexcept { return m_Data.begin(); }
			constexpr auto begin() const noexcept { return m_Data.begin(); }
			constexpr auto end() noexcept { return m_Data.end(); }
			constexpr auto end() const noexcept { return m_Data.end(); }
			constexpr auto cbegin() const noexcept { return m_Data.cbegin(); }
			constexpr auto cend() const noexcept { return m_Data.cend(); }

			constexpr auto rbegin() noexcept { return m_Data.rbegin(); }
			constexpr auto rbegin() const noexcept { return m_Data.rbegin(); }
			constexpr auto rend() noexcept { return m_Data.rend(); }
			constexpr auto rend() const noexcept { return m_Data.rend(); }
			constexpr auto crbegin() const noexcept { return m_Data.crbegin(); }
			constexpr auto crend() const noexcept { return m_Data.crend(); }

			constexpr auto data() const noexcept { return m_Data.data(); }
			constexpr auto front() noexcept { return m_Data.front(); }
			constexpr auto front() const noexcept { return m_Data.front(); }
			constexpr auto back() noexcept { return m_Data.back(); }
			constexpr auto back() const noexcept { return m_Data.back(); }

			constexpr auto substr(const_iterator begin, const_iterator end) const noexcept
			{
				return string_view_impl{begin, end};
			}

			constexpr auto substr(const_iterator begin) const noexcept { return string_view_impl{begin, end()}; }

			constexpr bool operator==(string_view_impl rhs) const noexcept
			{
				using std::begin, std::next;
				return size() == rhs.size() && std::equal(begin(m_Data), next(begin(m_Data),size()), begin(rhs));
			}

			constexpr bool operator!=(string_view_impl rhs) const noexcept
			{
				using std::begin, std::next;
				return size() != rhs.size() || !std::equal(begin(m_Data), next(begin(m_Data), size()), begin(rhs));
			}

			constexpr size_t occurrence(string_view_impl value) const noexcept
			{
				size_t result{0};
				const auto e   = end();
				auto it		   = begin();
				auto remainder = size();
				while(it != e && remainder >= value.size())
				{
					if(string_view_impl{it, it + value.size()} == value)
					{
						it += value.size();
						++result;
						remainder -= value.size();
					}
					else
					{
						++it;
						--remainder;
					}
				}
				return result;
			}

		  private:
			storage_t m_Data{};
		};
	} // namespace details

	using string_view = string_view_impl<string_encoding::UTF8>;

	constexpr string_view operator"" _sv(const char* data, [[maybe_unused]] std::size_t dsize)
	{
		return string_view{data};
	}

	constexpr string_view operator"" _utf8sv(const char* data, [[maybe_unused]] std::size_t size)
	{
		return string_view{data};
	}

	/**
	 * \brief Specialization of `string_view` which is guaranteed to be null-terminated (at creation)
	 * \warning Modifying the backing memory which this cstring was created from will lead to undefined behaviour. Same
	 * behaviour as a normal `string_view`.
	 */
	class cstring_view
	{
	  public:
	  private:
		// span<char_t> m_Data;
	};
} // namespace psl
