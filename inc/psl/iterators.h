#pragma once
#include <type_traits>
#include <cstddef>
#include <iterator>
#include "psl/concepts.h"

namespace psl
{
	template <typename T>
	class contiguous_range_iterator
	{
		constexpr static bool is_const_iterator = std::is_const_v<T>;

	  public:
		using difference_type   = std::ptrdiff_t;
		using value_type		= std::remove_cv_t<T>;
		using reference			= value_type&;
		using const_reference   = const value_type&;
		using pointer			= value_type*;
		using const_pointer		= const value_type*;
		using size_type			= size_t;
		using iterator_category = std::contiguous_iterator_tag;
		using iterator_concept  = std::contiguous_iterator_tag;

		// todo: note in current gcc10 (beta) contiguous_iterator_tag does not apply random_access_iterator_tag
		// optimizations, verify this with std::rotate

		constexpr contiguous_range_iterator() noexcept = default;
		constexpr contiguous_range_iterator(pointer data) noexcept : m_Data(data){};
		constexpr contiguous_range_iterator(
			const contiguous_range_iterator<value_type>& other) noexcept requires is_const_iterator
			: m_Data(other.m_Data)
		{}

		constexpr ~contiguous_range_iterator()										   = default;
		constexpr contiguous_range_iterator(const contiguous_range_iterator&) noexcept = default;
		constexpr contiguous_range_iterator(contiguous_range_iterator&&) noexcept	  = default;
		constexpr contiguous_range_iterator& operator=(const contiguous_range_iterator&) noexcept = default;
		constexpr contiguous_range_iterator& operator=(contiguous_range_iterator&&) noexcept = default;

		constexpr bool operator==(const contiguous_range_iterator& other) const noexcept
		{
			return m_Data == other.m_Data;
		}
		constexpr bool operator!=(const contiguous_range_iterator& other) const noexcept
		{
			return m_Data != other.m_Data;
		}
		constexpr bool operator<(const contiguous_range_iterator& other) const noexcept
		{
			return m_Data < other.m_Data;
		}
		constexpr bool operator>(const contiguous_range_iterator& other) const noexcept
		{
			return m_Data > other.m_Data;
		}
		constexpr bool operator<=(const contiguous_range_iterator& other) const noexcept
		{
			return m_Data <= other.m_Data;
		}
		constexpr bool operator>=(const contiguous_range_iterator& other) const noexcept
		{
			return m_Data >= other.m_Data;
		}


		auto operator*() const noexcept -> std::conditional_t<is_const_iterator, const_reference, reference>
		{
			return *m_Data;
		}
		auto operator-> () const noexcept -> std::conditional_t<is_const_iterator, const_pointer, pointer>
		{
			return m_Data;
		}

		auto& operator++() noexcept
		{
			++m_Data;
			return *this;
		}

		auto operator++(int) const noexcept
		{
			auto copy{*this};
			return ++copy;
		}

		auto& operator--() noexcept
		{
			--m_Data;
			return *this;
		}

		auto operator--(int) const noexcept
		{
			auto copy{*this};
			return --copy;
		}

		constexpr auto& operator+=(IsIntegral auto rhs) noexcept
		{
			m_Data += rhs;
			return *this;
		}

		constexpr auto operator+(IsIntegral auto rhs) const noexcept
		{
			auto copy = *this;
			return copy += rhs;
		}


		constexpr auto& operator-=(IsIntegral auto rhs) noexcept
		{
			m_Data -= rhs;
			return *this;
		}

		constexpr auto operator-(IsIntegral auto rhs) const noexcept
		{
			auto copy = *this;
			return copy -= rhs;
		}

		// friend constexpr auto operator+(IsIntegral auto lhs, const contiguous_range_iterator& rhs) noexcept;
		// friend constexpr auto operator-(IsIntegral auto lhs, const contiguous_range_iterator& rhs) noexcept;

		constexpr difference_type operator-(const contiguous_range_iterator<value_type>& other) const noexcept
		{
			return m_Data - other.m_Data;
		}
		constexpr difference_type operator-(const contiguous_range_iterator<const value_type>& other) const noexcept
		{
			return m_Data - other.m_Data;
		}

		auto operator[](IsIntegral auto index) const noexcept
			-> std::conditional_t<is_const_iterator, const_reference, reference>
		{
			return *(m_Data + index);
		}

	  private:
		pointer m_Data{nullptr};
	};
	static_assert(std::is_trivially_copyable_v<contiguous_range_iterator<int>>);


	template <typename T>
	class contiguous_ring_range_iterator
	{
		constexpr static bool is_const_iterator = std::is_const_v<T>;
		friend class contiguous_ring_range_iterator<std::remove_cv_t<T>>;
		friend class contiguous_ring_range_iterator<const std::remove_cv_t<T>>;

	  public:
		using difference_type   = std::ptrdiff_t;
		using value_type		= std::remove_cv_t<T>;
		using reference			= value_type&;
		using const_reference   = const value_type&;
		using pointer			= value_type*;
		using const_pointer		= const value_type*;
		using size_type			= size_t;
		using iterator_category = std::contiguous_iterator_tag;
		using iterator_concept  = std::contiguous_iterator_tag;

		constexpr contiguous_ring_range_iterator() noexcept = default;
		constexpr contiguous_ring_range_iterator(pointer data, pointer begin, pointer end, size_t index) noexcept
			: m_Data(data), m_Begin(begin), m_End(end), m_Index(index)
		{}
		constexpr contiguous_ring_range_iterator(
			const contiguous_ring_range_iterator<value_type>& other) noexcept requires is_const_iterator
			: m_Data(other.m_Data),
			  m_Begin(other.m_Begin),
			  m_End(other.m_End),
			  m_Index(other.m_Index)
		{}

		constexpr ~contiguous_ring_range_iterator()												 = default;
		constexpr contiguous_ring_range_iterator(const contiguous_ring_range_iterator&) noexcept = default;
		constexpr contiguous_ring_range_iterator(contiguous_ring_range_iterator&&) noexcept		 = default;
		constexpr contiguous_ring_range_iterator& operator=(const contiguous_ring_range_iterator&) noexcept = default;
		constexpr contiguous_ring_range_iterator& operator=(contiguous_ring_range_iterator&&) noexcept = default;

		constexpr bool operator==(const contiguous_ring_range_iterator& other) const noexcept
		{
			return m_Index == other.m_Index;
		}
		constexpr bool operator!=(const contiguous_ring_range_iterator& other) const noexcept
		{
			return m_Index != other.m_Index;
		}
		constexpr bool operator<(const contiguous_ring_range_iterator& other) const noexcept
		{
			return m_Index < other.m_Index;
		}
		constexpr bool operator>(const contiguous_ring_range_iterator& other) const noexcept
		{
			return m_Index > other.m_Index;
		}
		constexpr bool operator<=(const contiguous_ring_range_iterator& other) const noexcept
		{
			return m_Index <= other.m_Index;
		}
		constexpr bool operator>=(const contiguous_ring_range_iterator& other) const noexcept
		{
			return m_Index >= other.m_Index;
		}


		auto operator*() const noexcept -> std::conditional_t<is_const_iterator, const_reference, reference>
		{
			return *m_Data;
		}
		auto operator-> () const noexcept -> std::conditional_t<is_const_iterator, const_pointer, pointer>
		{
			return m_Data;
		}

		auto& operator++() noexcept
		{
			++m_Data;
			++m_Index;
			if(m_Data == m_End) m_Data = m_Begin;
			return *this;
		}

		auto operator++(int) const noexcept
		{
			auto copy{*this};
			return ++copy;
		}

		auto& operator--() noexcept
		{
			--m_Data;
			--m_Index;
			if(m_Data < m_Begin) m_Data = m_End - 1;
			return *this;
		}

		auto operator--(int) const noexcept
		{
			auto copy{*this};
			return --copy;
		}

		constexpr auto& operator+=(IsIntegral auto rhs) noexcept
		{
			m_Data += rhs;
			m_Index += rhs;
			if(m_Data >= m_End) m_Data = m_Begin + (m_Data - m_End);
			return *this;
		}

		constexpr auto operator+(IsIntegral auto rhs) const noexcept
		{
			auto copy = *this;
			return copy += rhs;
		}


		constexpr auto& operator-=(IsIntegral auto rhs) noexcept
		{
			m_Data -= rhs;
			m_Index -= rhs;
			if(m_Data < m_Begin) m_Data = m_End - (m_Begin - m_Data);
			return *this;
		}

		constexpr auto operator-(IsIntegral auto rhs) const noexcept
		{
			auto copy = *this;
			return copy -= rhs;
		}

		// friend constexpr auto operator+(IsIntegral auto lhs, const contiguous_ring_range_iterator& rhs) noexcept;
		// friend constexpr auto operator-(IsIntegral auto lhs, const contiguous_ring_range_iterator& rhs) noexcept;

		constexpr difference_type operator-(const contiguous_ring_range_iterator<value_type>& other) const noexcept
		{
			return static_cast<difference_type>(m_Index) - static_cast<difference_type>(other.m_Index);
		}
		constexpr difference_type operator-(const contiguous_ring_range_iterator<const value_type>& other) const
			noexcept
		{
			return static_cast<difference_type>(m_Index) - static_cast<difference_type>(other.m_Index);
		}

		auto operator[](IsIntegral auto index) const noexcept
			-> std::conditional_t<is_const_iterator, const_reference, reference>
		{
			return *(m_Data + index);
		}

	  private:
		pointer m_Data{nullptr};
		pointer m_Begin{nullptr};
		pointer m_End{nullptr};
		size_t m_Index{0};
	};
} // namespace psl

template <typename T>
constexpr auto operator+(psl::IsIntegral auto lhs, const psl::contiguous_range_iterator<T>& rhs) noexcept
{
	auto copy = rhs;
	return copy += lhs;
}
template <typename T>
constexpr auto operator-(psl::IsIntegral auto lhs, const psl::contiguous_range_iterator<T>& rhs) noexcept
{
	auto copy = rhs;
	return copy -= lhs;
}

template <typename T>
constexpr auto operator+(psl::IsIntegral auto lhs, const psl::contiguous_ring_range_iterator<T>& rhs) noexcept
{
	auto copy = rhs;
	return copy += lhs;
}
template <typename T>
constexpr auto operator-(psl::IsIntegral auto lhs, const psl::contiguous_ring_range_iterator<T>& rhs) noexcept
{
	auto copy = rhs;
	return copy -= lhs;
}

namespace std
{
	template <typename T>
	struct iterator_traits<psl::contiguous_range_iterator<T>>
	{
		typedef typename psl::contiguous_range_iterator<T>::value_type value_type;
		typedef typename psl::contiguous_range_iterator<T>::difference_type difference_type;
		typedef typename psl::contiguous_range_iterator<T>::iterator_category iterator_category;
		typedef typename psl::contiguous_range_iterator<T>::iterator_concept iterator_concept;
		typedef typename psl::contiguous_range_iterator<T>::pointer pointer;
		typedef typename psl::contiguous_range_iterator<T>::reference reference;
	};
} // namespace std