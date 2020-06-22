#pragma once
#include <tuple>

#include "psl/concepts.h"
#include "psl/details/metaprogramming.h"

namespace psl
{
	template <typename... Ts>
	class zip_iterator;

	template <typename... Ts>
	struct zip_proxy
	{
		friend class zip_iterator<Ts...>;
		template <typename... Ys, size_t... indices>
		static void swap_internal(zip_proxy<Ys...>& a, zip_proxy<Ys...>& b, std::integer_sequence<size_t, indices...>)
		{
			using std::swap;

			(swap(*std::get<indices>(a.data), *std::get<indices>(b.data)), ...);
		}

		template <size_t... indices>
		std::tuple<Ts...> as_values(std::integer_sequence<size_t, indices...>) const noexcept
		{
			return std::tuple<Ts...>{*std::get<indices>(data)...};
		}

	  public:
		constexpr zip_proxy() noexcept = default;
		constexpr ~zip_proxy()		   = default;
		constexpr zip_proxy(Ts*... data) noexcept : data(const_cast<std::remove_const_t<Ts>*>(data)...) {}
		constexpr zip_proxy(const zip_proxy& other) noexcept : data(other.data) {}
		constexpr zip_proxy(zip_proxy&& other) noexcept : data(other.data) {}

		constexpr bool operator==(const zip_proxy& other) const noexcept { return data == other.data; }
		constexpr bool operator!=(const zip_proxy& other) const noexcept { return data != other.data; }
		constexpr bool operator<(const zip_proxy& other) const noexcept { return data < other.data; }
		constexpr bool operator>(const zip_proxy& other) const noexcept { return data > other.data; }
		constexpr bool operator<=(const zip_proxy& other) const noexcept { return data <= other.data; }
		constexpr bool operator>=(const zip_proxy& other) const noexcept { return data >= other.data; }

		constexpr zip_proxy& operator=(const zip_proxy& other) noexcept requires(!std::is_const_v<Ts> && ...)
		{
			if(this != &other) data = other.data;
			return *this;
		}
		constexpr zip_proxy& operator=(zip_proxy&& other) noexcept requires(!std::is_const_v<Ts> && ...)
		{
			if(this != &other) data = other.data;
			return *this;
		}

		constexpr std::tuple<Ts...> operator*() const noexcept { return as_values(std::index_sequence_for<Ts...>{}); }

		template <size_t I>
		std::tuple_element_t<I, std::tuple<Ts...>>& get() const noexcept
		{
			return (*std::get<I>(data));
		}

		template <typename... Ys>
		constexpr friend void swap(zip_proxy<Ys...>& a, zip_proxy<Ys...>& b) requires(!std::is_const_v<Ts> && ...)
		{
			swap_internal(a, b, std::index_sequence_for<Ts...>{});
		}

	  private:
		std::tuple<std::remove_cvref_t<Ts>*...> data{};
	};

	template <size_t I, typename... Ts>
	auto& get(const zip_proxy<Ts...>& v) noexcept
	{
		return v.template get<I>();
	}
	template <size_t I, typename... Ts>
	auto& get(zip_proxy<Ts...>& v) noexcept
	{
		return v.template get<I>();
	}

	template <typename... Ts>
	struct zip_iterator
	{

	  public:
		using difference_type   = std::ptrdiff_t;
		using value_type		= zip_proxy<Ts...>;
		using reference			= value_type&;
		using const_reference   = const value_type&;
		using pointer			= value_type*;
		using const_pointer		= const value_type*;
		using size_type			= size_t;
		using iterator_category = std::random_access_iterator_tag;
		using iterator_concept  = std::contiguous_iterator_tag;

		constexpr zip_iterator() noexcept = default;
		constexpr zip_iterator(Ts*... pointers) noexcept : m_Data(pointers...) {}

		constexpr ~zip_iterator()							 = default;
		constexpr zip_iterator(const zip_iterator&) noexcept = default;
		constexpr zip_iterator(zip_iterator&&) noexcept		 = default;
		constexpr zip_iterator& operator=(const zip_iterator&) noexcept = default;
		constexpr zip_iterator& operator=(zip_iterator&&) noexcept = default;

		constexpr bool operator==(const zip_iterator& other) const noexcept { return m_Data == other.m_Data; }
		constexpr bool operator!=(const zip_iterator& other) const noexcept { return m_Data != other.m_Data; }
		constexpr bool operator<(const zip_iterator& other) const noexcept { return m_Data < other.m_Data; }
		constexpr bool operator>(const zip_iterator& other) const noexcept { return m_Data > other.m_Data; }
		constexpr bool operator<=(const zip_iterator& other) const noexcept { return m_Data <= other.m_Data; }
		constexpr bool operator>=(const zip_iterator& other) const noexcept { return m_Data >= other.m_Data; }


		const_reference operator*() const noexcept { return m_Data; }
		const_pointer operator->() const noexcept { return m_Data; }
		reference operator*() noexcept { return m_Data; }
		pointer operator->() noexcept { return m_Data; }

		auto& operator++() noexcept
		{
			std::apply([](auto&&... data) { (void(++data), ...); }, m_Data.data);
			return *this;
		}

		auto operator++(int) const noexcept
		{
			auto copy{*this};
			return ++copy;
		}

		auto& operator--() noexcept
		{
			std::apply([](auto&&... data) { ((--data), ...); }, m_Data.data);
			return *this;
		}

		auto operator--(int) const noexcept
		{
			auto copy{*this};
			return --copy;
		}

		constexpr auto& operator+=(IsIntegral auto rhs) noexcept
		{
			std::apply([&rhs](auto&&... data) { (void(data += rhs), ...); }, m_Data.data);
			return *this;
		}

		constexpr auto operator+(IsIntegral auto rhs) const noexcept
		{
			auto copy = *this;
			return copy += rhs;
		}

		constexpr auto& operator-=(IsIntegral auto rhs) noexcept
		{
			std::apply([&rhs](auto&&... data) { (void(data -= rhs), ...); }, m_Data.data);
			return *this;
		}

		constexpr auto operator-(IsIntegral auto rhs) const noexcept
		{
			auto copy = *this;
			return copy -= rhs;
		}

		constexpr difference_type operator-(const zip_iterator& other) const noexcept
		{
			return std::get<0>(m_Data.data) - std::get<0>(other.m_Data.data);
		}
		reference operator[](IsIntegral auto index) noexcept { return *(m_Data + index); }
		const_reference operator[](IsIntegral auto index) const noexcept { return *(m_Data + index); }

		// template <size_t I>
		// decltype(auto) get() const noexcept
		// {
		// 	return m_Data.template get<I>();
		// }


	  private:
		value_type m_Data{};
	};

	template <typename... Ts>
	class zip_range
	{
	  public:
		template <IsRange... Ranges>
		zip_range(Ranges&&... ranges) : m_Begin(&*(ranges.begin())...), m_End(&*(ranges.end())...)
		{}
		auto begin() noexcept { return m_Begin; }
		auto begin() const noexcept { return m_Begin; }
		auto cbegin() const noexcept { return m_Begin; }
		auto end() noexcept { return m_End; }
		auto end() const noexcept { return m_End; }
		auto cend() const noexcept { return m_End; }

		auto size() const noexcept { return m_End - m_Begin; }

	  private:
		zip_iterator<Ts...> m_Begin{};
		zip_iterator<Ts...> m_End{};
	};
} // namespace psl

namespace std
{
	template <typename... Ts>
	struct tuple_size<psl::zip_proxy<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)>
	{};

	template <typename... Ts>
	struct tuple_size<psl::zip_iterator<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)>
	{};

	template <std::size_t N, typename... Ts>
	struct tuple_element<N, psl::zip_proxy<Ts...>>
	{
		using type = decltype(std::declval<psl::zip_proxy<Ts...>>().template get<N>());
	};

	template <std::size_t N, typename... Ts>
	struct tuple_element<N, psl::zip_iterator<Ts...>>
	{
		using type = decltype(std::declval<psl::zip_iterator<Ts...>>().template get<N>());
	};
} // namespace std

namespace psl
{
	template <IsRange T, IsRange... Ts>
	auto zip(T& range, Ts&... ranges)
	{
		PSL_EXCEPT_IF(any_of([&]() { return range.size() != ranges.size(); }...),
					  "incorrect elements in range, they should all be equal", std::range_error);
		return zip_range<
			std::conditional_t<std::is_const_v<T>, const typename T::value_type, typename T::value_type>,
			std::conditional_t<std::is_const_v<Ts>, const typename Ts::value_type, typename Ts::value_type>...>{
			range, ranges...};
	}
} // namespace psl