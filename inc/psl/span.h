#pragma once
#include <psl/iterators.h>
#include <psl/concepts.h>

#include <span>
/**
 * Replacement span type that supports strided element iteration.
 * An example of where this can be useful is for example the non-immediate contiguous elements in a matrix (i.e.
 * iterating a column in a row-major implementation).
 *
 * For non-strided variations it falls back to std::span
 */


namespace psl
{
	inline constexpr std::size_t dynamic_extent = std::dynamic_extent;

	inline namespace details
	{
		template <typename T, size_t Extent, size_t Stride>
		class static_strided_span
		{
		  public:
			static_assert(Stride > 0, "zero sized stride is not supported");
			using element_type	= T;
			using value_type	  = std::remove_cv_t<T>;
			using size_type		  = std::size_t;
			using difference_type = std::ptrdiff_t;
			using pointer		  = T*;
			using const_pointer   = const T*;
			using reference		  = T&;
			using const_reference = const T&;
			using iterator		  = strided_contiguous_range_iterator<T, Stride>;
			using const_iterator  = strided_contiguous_range_iterator<const T, Stride>;

			constexpr static_strided_span(T* begin) noexcept : m_Begin(begin){};

			constexpr reference operator[](size_type index) noexcept { return (begin())[index]; }
			constexpr const_reference operator[](size_type index) const noexcept { return (begin())[index]; }

			constexpr iterator begin() noexcept { return {front()}; }
			constexpr iterator begin() const noexcept { return {front()}; }
			constexpr iterator end() noexcept { return {back()}; }
			constexpr iterator end() const noexcept { return {back()}; }
			constexpr const_iterator cbegin() const noexcept { return begin(); }
			constexpr const_iterator cend() const noexcept { return end(); }

			constexpr pointer data() const noexcept { return m_Begin; }
			constexpr pointer front() noexcept { return m_Begin; }
			constexpr const_pointer front() const noexcept { return m_Begin; }
			constexpr pointer back() noexcept { return (pointer)((std::uintptr_t)m_Begin + (Extent * Stride)); }
			constexpr const_pointer back() const noexcept
			{
				return (pointer)((std::uintptr_t)m_Begin + (Extent * Stride));
			}

			constexpr size_type size() const noexcept { return Extent; }
			constexpr size_type size_bytes() const noexcept { return Extent * Stride; }
			constexpr bool empty() const noexcept { return Extent == 0; }
			constexpr size_type stride() const noexcept { return Stride; }

			template <size_t Count>
			constexpr static_strided_span<T, Count, Stride> subspan(size_type offset) const noexcept
			{
				return static_strided_span<T, Count, Stride>{&*(begin()[offset])};
			}

			template <size_t Offset, size_t Count>
			constexpr static_strided_span<T, Count, Stride> subspan() const noexcept
			{
				return static_strided_span<T, Count, Stride>{&*(begin()[Offset])};
			}

		  private:
			T* m_Begin;
		};

		template <typename T, size_t Stride>
		class dynamic_strided_span
		{
		  public:
			static_assert(Stride > 0, "zero sized stride is not supported");
			using element_type	= T;
			using value_type	  = std::remove_cv_t<T>;
			using size_type		  = std::size_t;
			using difference_type = std::ptrdiff_t;
			using pointer		  = T*;
			using const_pointer   = const T*;
			using reference		  = T&;
			using const_reference = const T&;
			using iterator		  = strided_contiguous_range_iterator<T, Stride>;
			using const_iterator  = strided_contiguous_range_iterator<const T, Stride>;

			constexpr dynamic_strided_span(T* begin, size_type count) noexcept
				: m_Begin(begin), m_End((pointer)((std::uintptr_t)begin + (count * Stride))){};

			constexpr reference operator[](size_type index) noexcept { return (begin())[index]; }
			constexpr const_reference operator[](size_type index) const noexcept { return (begin())[index]; }

			constexpr iterator begin() noexcept { return {front()}; }
			constexpr iterator begin() const noexcept { return {front()}; }
			constexpr iterator end() noexcept { return {back()}; }
			constexpr iterator end() const noexcept { return {back()}; }
			constexpr const_iterator cbegin() const noexcept { return begin(); }
			constexpr const_iterator cend() const noexcept { return end(); }

			constexpr pointer data() const noexcept { return m_Begin; }
			constexpr pointer front() noexcept { return m_Begin; }
			constexpr const_pointer front() const noexcept { return m_Begin; }
			constexpr pointer back() noexcept { return m_End; }
			constexpr const_pointer back() const noexcept { return m_End; }

			constexpr size_type size() const noexcept { return size_bytes() / Stride; }
			constexpr size_type size_bytes() const noexcept { return (std::uintptr_t)m_End - (std::uintptr_t)m_Begin; }
			constexpr bool empty() const noexcept { return m_Begin == m_End; }
			constexpr size_type stride() const noexcept { return Stride; }

		  private:
			T* m_Begin;
			T* m_End;
		};

		template <typename T>
		dynamic_strided_span(T val, size_t count)->dynamic_strided_span<T, sizeof(T)>;

		template <typename T, size_t Extent>
		class static_span
		{
		  public:
			using element_type	= T;
			using value_type	  = std::remove_cv_t<T>;
			using size_type		  = std::size_t;
			using difference_type = std::ptrdiff_t;
			using pointer		  = T*;
			using const_pointer   = const T*;
			using reference		  = T&;
			using const_reference = const T&;
			using iterator		  = contiguous_range_iterator<T>;
			using const_iterator  = contiguous_range_iterator<const T>;

			constexpr static_span(T* begin) noexcept : m_Begin(begin){};

			constexpr reference operator[](size_type index) noexcept { return (begin())[index]; }
			constexpr const_reference operator[](size_type index) const noexcept { return (begin())[index]; }

			constexpr iterator begin() noexcept { return {front()}; }
			constexpr iterator begin() const noexcept { return {front()}; }
			constexpr iterator end() noexcept { return {back()}; }
			constexpr iterator end() const noexcept { return {back()}; }
			constexpr const_iterator cbegin() const noexcept { return begin(); }
			constexpr const_iterator cend() const noexcept { return end(); }

			constexpr pointer data() const noexcept { return m_Begin; }
			constexpr pointer front() noexcept { return m_Begin; }
			constexpr const_pointer front() const noexcept { return m_Begin; }
			constexpr pointer back() noexcept { return m_Begin + Extent; }
			constexpr const_pointer back() const noexcept { return m_Begin + Extent; }

			constexpr size_type size() const noexcept { return Extent; }
			constexpr size_type size_bytes() const noexcept { return Extent * sizeof(T); }
			constexpr bool empty() const noexcept { return Extent == 0; }
			constexpr size_type stride() const noexcept { return sizeof(T); }

			template <size_t Count>
			constexpr static_span<T, Count> subspan(size_type offset) const noexcept
			{
				return static_span<T, Count>{&*(begin()[offset])};
			}

			template <size_t Offset, size_t Count>
			constexpr static_span<T, Count> subspan() const noexcept
			{
				return static_span<T, Count>{&*(begin()[Offset])};
			}

		  private:
			T* m_Begin;
		};

		template <typename T>
		using dynamic_span = std::span<T>;
	} // namespace details

	template <typename T, size_t Extent = dynamic_extent, size_t Stride = sizeof(T)>
	using span = std::conditional_t<
		Extent == dynamic_extent,
		std::conditional_t<Stride == sizeof(T), dynamic_span<T>, dynamic_strided_span<T, Stride>>,
		std::conditional_t<Stride == sizeof(T), static_span<T, Extent>, static_strided_span<T, Extent, Stride>>>;
} // namespace psl
