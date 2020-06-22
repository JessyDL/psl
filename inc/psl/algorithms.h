#pragma once

#include "psl/concepts.h"
#include "psl/static_array.h"
#include "psl/string.h"
#include "psl/config.h"
#include "psl/exception.h"
#include <cstddef>
#include <span>
#include <cmath>

namespace psl
{
	template <IsIntegral I, typename T>
	auto get(T&& value)
	{
		using std::get;
		return get<I>(value);
	}

	template <IsIntegral precision_t>
	constexpr static auto log(precision_t value) noexcept requires std::is_convertible_v<precision_t, long double>
	{
		return std::log(static_cast<long double>(value));
	}

	template <IsIntegral precision_t, IsIntegral precision_N_t>
	constexpr static long double log_n(precision_N_t N, precision_t value) noexcept
	{
		return log(value) / log(N);
	}


	template <IsIntegral precision_t, IsIntegral precision_N_t>
	constexpr static precision_t
	next_pow_of(precision_N_t N, precision_t value) noexcept requires std::is_convertible_v<precision_t, long double>
	{
		return static_cast<precision_t>(std::pow<precision_t>(N, static_cast<precision_t>(std::ceil(log_n(N, value)))));
	}


	/**
	 * \brief Aligns a value to the next possible value that satisfies the alignment
	 *
	 * \tparam T
	 * \param[in] value
	 * \param[in] alignment
	 * \returns constexpr auto
	 */
	template <IsIntegral T>
	constexpr inline auto align_to(T value, T alignment) noexcept
	{
		if(alignment <= T{1}) return value;
		auto remainder = value % alignment;
		return (remainder) ? value + (alignment - remainder) : value;
	}

	/**
	 * \brief Aligns a value to the previous possible value that satisfies the alignment (including the seed value)
	 *
	 * \tparam T
	 * \param[in] value
	 * \param[in] alignment
	 * \returns constexpr auto
	 */
	template <IsIntegral T>
	constexpr inline auto ralign_to(T value, T alignment) noexcept
	{
		if(alignment <= T{1}) return value;
		auto remainder = value % alignment;
		return (remainder) ? value - remainder : value;
	}

	constexpr auto to_hex(std::byte value) noexcept
	{
		constexpr char8_t hex[] = u8"0123456789ABCDEF";
		psl::static_array<char8_t, 2> res{};
		res[0] = hex[(((int)value & 0xF0) >> 4)];
		res[1] = hex[(((int)value & 0x0F) >> 0)];
		return res;
	}

	inline auto to_hex(std::span<std::byte> value) noexcept
	{
		constexpr char8_t hex[] = u8"0123456789ABCDEF";
		psl::string res{u8"0x"};
		res.resize((value.size() * 2) + 2);
		size_t index = (value.size() * 2);
		for(auto byte : value)
		{
			res[index]	 = hex[(((int)byte & 0xF0) >> 4)];
			res[index + 1] = hex[(((int)byte & 0x0F) >> 0)];
			index -= 2;
		}
		return res;
	}

	template <typename T>
	constexpr void from_hex(psl::string_view value, T& target) noexcept(!config::exceptions)
	{
		PSL_EXCEPT_IF(sizeof(T) * 2 != value.size() - 2, "value is of incorrect size", std::runtime_error);
		PSL_EXCEPT_IF(value.substr(0, 2) != u8"0x", "value does not start with the correct prefix", std::runtime_error);
		psl::static_array<uint8_t, sizeof(T)>& result_arr = *(psl::static_array<uint8_t, sizeof(T)>*)&target;
		for(size_t n = 0; n < sizeof(T); ++n)
		{
			uint8_t& result = result_arr[sizeof(T) - (n + 1)];
			for(size_t i = 0; i < 2; ++i)
			{
				auto character = value[n * 2 + i + 2];
				int res{};
				if('0' <= character && character <= '9')
					res = character - '0';
				else if('a' <= character && character <= 'f')
					res = 10 + character - 'a';
				else
					res = 10 + character - 'A';

				result |= res << (4 * (1 - i));
			}
		}
	}

	template <typename T>
	constexpr auto from_hex(psl::string_view value)
	{
		T t{};
		from_hex(value, t);
		return t;
	}

	template <typename T>
	struct converter
	{
		constexpr auto to_string(const T& target) const noexcept { return psl::to_string<T>(target); }
	};

	template <typename T>
	struct hex_converter
	{
		constexpr auto to_string(const T& target) const noexcept
		{
			std::span<std::byte> bytes{(std::byte*)&target, sizeof(T)};
			return to_hex(bytes);
		}

		constexpr auto from_string(psl::string_view target) const noexcept(!config::exceptions)
		{
			return from_hex<T>(target);
		}
	};

	enum class sorter
	{
		hybrid,
		quick,
		insertion,
		heap,
		merge
	};

	namespace sorting
	{
		namespace details
		{
			template <typename It, class Pred>
			It partition(It first, It last, Pred&& pred) noexcept
			{
				using std::iter_swap, std::prev, std::next, std::distance;

				while(pred(*first, *next(first)))
				{
					first = next(first);

					if(next(first) == last) return last;
				}

				for(auto i = next(first); i != last; i = next(i))
				{
					if(pred(*i, *first))
					{
						iter_swap(i, first);
						first = next(first);
					}
				}
				return first;
			}
		} // namespace details

		template <typename It, class Pred>
		void quick(const It first, const It last, Pred&& pred) noexcept
		{
			using std::distance, std::next, std::prev;
			if(distance(first, last) > 1)
			{
				auto pivot = ::psl::sorting::details::partition(first, last, std::forward<Pred>(pred));
				if(pivot == last) return;
				quick(first, pivot, std::forward<Pred>(pred));
				quick(next(pivot), last, std::forward<Pred>(pred));
			}
		}

		/// \details sorting algorithm best used for data with few unique items
		template <typename It>
		void quick(const It first, const It last) noexcept
		{
			quick(first, last, std::less<typename std::iterator_traits<It>::value_type>());
		}

		template <typename It, class Pred>
		void insertion(const It first, const It last, Pred&& pred) noexcept
		{
			using std::next, std::prev, std::iter_swap;
			for(auto i = first; i != last; i = next(i))
			{
				for(auto j = i; j != first; j = prev(j))
				{
					if(pred(*j, *prev(j)))
						iter_swap(j, prev(j));
					else
						break;
				}
			}
		}

		template <typename It>
		void insertion(const It first, const It last) noexcept
		{
			insertion(first, last, std::less<typename std::iterator_traits<It>::value_type>());
		}

	} // namespace sorting

	template <typename T>
	void test()
	{
		return;
	}

	template <sorter Sorter = sorter::quick, typename It, typename Pred>
	void sort(const It first, const It last, Pred&& pred) noexcept
	{
		if constexpr(Sorter == sorter::quick)
		{
			::psl::sorting::quick(first, last, std::forward<Pred>(pred));
		}
		else if constexpr(Sorter == sorter::insertion)
		{
			::psl::sorting::insertion(first, last, std::forward<Pred>(pred));
		}
		else if constexpr(Sorter == sorter::merge)
		{
			::psl::sorting::quick(first, last, std::forward<Pred>(pred));
		}
	}

	template <sorter Sorter = sorter::quick, typename It>
	void sort(const It first, const It last) noexcept
	{
		if constexpr(Sorter == sorter::quick)
		{
			::psl::sorting::quick(first, last);
		}
		else if constexpr(Sorter == sorter::insertion)
		{
			::psl::sorting::insertion(first, last);
		}
		else if constexpr(Sorter == sorter::merge)
		{
			::psl::sorting::quick(first, last);
		}
	}
} // namespace psl