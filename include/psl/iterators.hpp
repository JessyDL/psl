#pragma once
#include <type_traits>
#include <psl/types.hpp>
#include <psl/type_concepts.hpp>

namespace psl
{
	inline namespace details
	{
		template <typename T, typename Y>
		concept CmpCRangeIt = std::is_same_v<typename T::value_type, typename Y::value_type>&&
			std::is_same_v<typename T::iterator_category, typename Y::iterator_category>;
	}

	/**
	 * \brief denotes an iterator that can go either direction in a contiguous range.
	 *
	 * \tparam T type of the value to iterate over
	 * \tparam Stride indices to jump
	 * \note Stride must be a non-zero value
	 */
	template <typename T, i64 Stride = 1>
	class contiguous_range_iterator
	{
		static_assert(Stride != 0, "0 stride is not valid");
		static constexpr size_t AbsStride = (Stride > 0) ? Stride : -Stride;

	  public:
		using difference_type	= std::ptrdiff_t;
		using value_type		= std::remove_cv_t<T>;
		using reference			= value_type&;
		using const_reference	= const value_type&;
		using pointer			= value_type*;
		using const_pointer		= const value_type*;
		using size_type			= size_t;
		using iterator_category = std::random_access_iterator_tag;
		using iterator_concept	= std::contiguous_iterator_tag;


		friend class contiguous_range_iterator<value_type, Stride>;
		friend class contiguous_range_iterator<const value_type, Stride>;

		constexpr contiguous_range_iterator() noexcept = default;
		constexpr contiguous_range_iterator(pointer data) noexcept : m_Data(data){};
		constexpr contiguous_range_iterator(const_pointer data) noexcept
			requires std::is_const_v<T> : m_Data(const_cast<pointer>(data)){};
		constexpr contiguous_range_iterator(const contiguous_range_iterator<value_type>& other) noexcept
			requires std::is_const_v<T> : m_Data(other.m_Data)
		{}

		constexpr ~contiguous_range_iterator()										   = default;
		constexpr contiguous_range_iterator(const contiguous_range_iterator&) noexcept = default;
		constexpr contiguous_range_iterator(contiguous_range_iterator&&) noexcept	   = default;
		constexpr contiguous_range_iterator& operator=(const contiguous_range_iterator&) noexcept = default;
		constexpr contiguous_range_iterator& operator=(contiguous_range_iterator&&) noexcept = default;

		constexpr bool operator==(const CmpCRangeIt<contiguous_range_iterator> auto& other) const noexcept
		{
			return m_Data == other.ptr();
		}
		constexpr bool operator!=(const CmpCRangeIt<contiguous_range_iterator> auto& other) const noexcept
		{
			return m_Data != other.ptr();
		}
		constexpr bool operator<(const CmpCRangeIt<contiguous_range_iterator> auto& other) const noexcept
		{
			if constexpr(Stride > 0)
				return m_Data < other.ptr();
			else
				return m_Data > other.ptr();
		}
		constexpr bool operator>(const CmpCRangeIt<contiguous_range_iterator> auto& other) const noexcept
		{
			if constexpr(Stride > 0)
				return m_Data > other.ptr();
			else
				return m_Data < other.ptr();
		}
		constexpr bool operator<=(const CmpCRangeIt<contiguous_range_iterator> auto& other) const noexcept
		{
			if constexpr(Stride > 0)
				return m_Data <= other.ptr();
			else
				return m_Data >= other.ptr();
		}
		constexpr bool operator>=(const CmpCRangeIt<contiguous_range_iterator> auto& other) const noexcept
		{
			if constexpr(Stride > 0)
				return m_Data >= other.ptr();
			else
				return m_Data <= other.ptr();
		}

		constexpr auto operator*() const noexcept -> std::conditional_t<std::is_const_v<T>, const_reference, reference>
		{
			return *m_Data;
		}

		constexpr auto operator->() const noexcept -> std::conditional_t<std::is_const_v<T>, const_pointer, pointer>
		{
			return m_Data;
		}

		auto& operator++() noexcept
		{
			m_Data += Stride;
			return *this;
		}
		auto operator++(int) const noexcept
		{
			auto copy{*this};
			return ++copy;
		}

		auto& operator--() noexcept
		{
			m_Data -= Stride;
			return *this;
		}

		auto operator--(int) const noexcept
		{
			auto copy{*this};
			return --copy;
		}

		constexpr auto& operator+=(IsIntegral auto rhs) noexcept
		{
			if constexpr(Stride > 0)
				m_Data += rhs * Stride;
			else
				m_Data -= rhs * AbsStride;

			return *this;
		}

		constexpr auto operator+(IsIntegral auto rhs) const noexcept
		{
			auto copy = *this;
			return copy += rhs;
		}


		constexpr auto& operator-=(IsIntegral auto rhs) noexcept
		{
			if constexpr(Stride > 0)
				m_Data -= rhs * Stride;
			else
				m_Data += rhs * AbsStride;
			return *this;
		}

		constexpr auto operator-(IsIntegral auto rhs) const noexcept
		{
			auto copy = *this;
			return copy -= rhs;
		}

		constexpr difference_type operator-(const contiguous_range_iterator<value_type, Stride>& other) const noexcept
		{
			return (m_Data - other.ptr()) / Stride;
		}
		constexpr difference_type
		operator-(const contiguous_range_iterator<const value_type, Stride>& other) const noexcept
		{
			return (m_Data - other.ptr()) / Stride;
		}

		constexpr auto operator[](IsIntegral auto index) const noexcept
			-> std::conditional_t<std::is_const_v<T>, const_reference, reference>
		{
			return *(m_Data + (index * Stride));
		}

		constexpr pointer ptr() noexcept requires(!std::is_const_v<T>) { return m_Data; }
		constexpr const_pointer ptr() const noexcept { return m_Data; }

		static constexpr i64 stride() noexcept { return Stride; }
		static constexpr size_t abs_stride() noexcept { return AbsStride; }

		constexpr bool is_valid_pair(const contiguous_range_iterator<value_type, Stride>& other) const noexcept
		{
			return (m_Data - other.ptr()) % Stride == 0;
		}

		constexpr bool is_valid_pair(const contiguous_range_iterator<const value_type, Stride>& other) const noexcept
		{
			return (m_Data - other.ptr()) % Stride == 0;
		}

	  private:
		pointer m_Data{nullptr};
	};
} // namespace psl