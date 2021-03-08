#pragma once
#include <psl/iterators.hpp>
#include <psl/type_concepts.hpp>
#include <psl/types.hpp>

// issue: GCC incorrectly evaluates the end ptr location in static arrays resulting in an OOB warning.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

namespace psl
{
	/**
	 * \brief Span type with optional support for stride.
	 * \details Replacement span type that supports strided element iteration.
	 * An example of where this can be useful is for example the non-immediate contiguous elements in a matrix (i.e.
	 * iterating a column in a row-major implementation).
	 *
	 * \tparam T value type that is contained
	 * \tparam Extent extent of the range, use dynamic_extent for a range that is unknown at compile time.
	 * \tparam Stride amount of elements to jump to the next element.
	 */
	template <typename T, size_t Extent = dynamic_extent, i64 Stride = 1>
	class span
	{
	  public:
		static_assert(Stride != 0, "zero sized stride is not supported");
		using element_type			 = T;
		using value_type			 = std::remove_cv_t<T>;
		using size_type				 = std::size_t;
		using difference_type		 = std::ptrdiff_t;
		using pointer				 = T*;
		using const_pointer			 = const T*;
		using reference				 = T&;
		using const_reference		 = const T&;
		using iterator				 = contiguous_range_iterator<value_type, Stride>;
		using const_iterator		 = contiguous_range_iterator<const value_type, Stride>;
		using reverse_iterator		 = contiguous_range_iterator<value_type, -Stride>;
		using const_reverse_iterator = contiguous_range_iterator<const value_type, -Stride>;

		constexpr span() noexcept = default;
		constexpr span(IsIterator auto begin) noexcept : m_Begin(&*begin){};
		constexpr span(const T* begin) noexcept : m_Begin(const_cast<T*>(begin)) {}

		template <size_t OtherExtent, i64 OtherStride>
		constexpr span(const span<T, OtherExtent, OtherStride>& other) noexcept : m_Begin(other.data())
		{}

		constexpr reference operator[](size_type index) noexcept { return m_Begin[index * Stride]; }
		constexpr const_reference operator[](size_type index) const noexcept { return m_Begin[index * Stride]; }

		constexpr iterator begin() noexcept { return {front()}; }
		constexpr const_iterator begin() const noexcept { return {front()}; }
		constexpr iterator end() noexcept { return {back()}; }
		constexpr const_iterator end() const noexcept { return {back()}; }
		constexpr const_iterator cbegin() const noexcept { return begin(); }
		constexpr const_iterator cend() const noexcept { return end(); }

		constexpr reverse_iterator rbegin() noexcept { return {back()}; }
		constexpr const_reverse_iterator rbegin() const noexcept { return {back()}; }
		constexpr reverse_iterator rend() noexcept { return {front()}; }
		constexpr const_reverse_iterator rend() const noexcept { return {front()}; }
		constexpr const_reverse_iterator crbegin() const noexcept { return begin(); }
		constexpr const_reverse_iterator crend() const noexcept { return end(); }

		constexpr pointer data() const noexcept { return m_Begin; }
		constexpr pointer front() noexcept { return m_Begin; }
		constexpr const_pointer front() const noexcept { return m_Begin; }
		constexpr pointer back() noexcept { return m_Begin + (Extent * Stride); }
		constexpr const_pointer back() const noexcept { return m_Begin + (Extent * Stride); }

		constexpr size_type size() const noexcept { return Extent; }
		constexpr size_type size_bytes() const noexcept { return Extent * sizeof(T); }
		constexpr bool empty() const noexcept { return Extent == 0; }
		static constexpr i64 stride() noexcept { return Stride; }
		static constexpr size_type abs_stride() noexcept { return (Stride > 0) ? Stride : -Stride; }

		template <size_t Count>
		constexpr span<T, Count, Stride> subspan(size_type offset) const noexcept
		{
			return span<T, Count, Stride>{&*(begin()[offset * Stride])};
		}

		template <size_t Offset, size_t Count>
		constexpr span<T, Count, Stride> subspan() const noexcept
		{
			return span<T, Count, Stride>{&*(begin()[Offset * Stride])};
		}

		constexpr span<T, Extent, -Stride> reverse() const noexcept
		{
			return span<T, Extent, -Stride>{m_Begin + ((Extent - 1) * Stride)};
		}

	  private:
		T* m_Begin{nullptr};
	};

	template <typename T, i64 Stride>
	class span<T, dynamic_extent, Stride>
	{
	  public:
		static_assert(Stride != 0, "zero sized stride is not supported");
		using element_type			 = T;
		using value_type			 = std::remove_cv_t<T>;
		using size_type				 = std::size_t;
		using difference_type		 = std::ptrdiff_t;
		using pointer				 = T*;
		using const_pointer			 = const T*;
		using reference				 = T&;
		using const_reference		 = const T&;
		using iterator				 = contiguous_range_iterator<value_type, Stride>;
		using const_iterator		 = contiguous_range_iterator<const value_type, Stride>;
		using reverse_iterator		 = contiguous_range_iterator<value_type, -Stride>;
		using const_reverse_iterator = contiguous_range_iterator<const value_type, -Stride>;
		constexpr span() noexcept	= default;
		constexpr span(IsIterator auto begin, size_type count) noexcept
			: m_Begin(&*begin), m_End(m_Begin + (count * Stride)){};
		constexpr span(const T* begin, size_type count) noexcept
			: m_Begin(const_cast<T*>(begin)), m_End(m_Begin + (count * Stride)){};

		template <size_t OtherExtent, i64 OtherStride>
		constexpr span(const span<T, OtherExtent, OtherStride>& other) noexcept
			: m_Begin(other.data()), m_End(other.data() + other.size())
		{}

		constexpr reference operator[](size_type index) noexcept { return m_Begin[index * Stride]; }
		constexpr const_reference operator[](size_type index) const noexcept { return m_Begin[index * Stride]; }

		constexpr iterator begin() noexcept { return {front()}; }
		constexpr const_iterator begin() const noexcept { return {front()}; }
		constexpr iterator end() noexcept { return {back()}; }
		constexpr const_iterator end() const noexcept { return {back()}; }
		constexpr const_iterator cbegin() const noexcept { return begin(); }
		constexpr const_iterator cend() const noexcept { return end(); }

		constexpr reverse_iterator rbegin() noexcept { return {back()}; }
		constexpr const_reverse_iterator rbegin() const noexcept { return {back()}; }
		constexpr reverse_iterator rend() noexcept { return {front()}; }
		constexpr const_reverse_iterator rend() const noexcept { return {front()}; }
		constexpr const_reverse_iterator crbegin() const noexcept { return begin(); }
		constexpr const_reverse_iterator crend() const noexcept { return end(); }

		constexpr pointer data() const noexcept { return m_Begin; }
		constexpr pointer front() noexcept { return m_Begin; }
		constexpr const_pointer front() const noexcept { return m_Begin; }
		constexpr pointer back() noexcept { return m_End; }
		constexpr const_pointer back() const noexcept { return m_End; }

		constexpr size_type size() const noexcept { return (m_End - m_Begin) / Stride; }
		constexpr size_type size_bytes() const noexcept { return size() * sizeof(T); }
		constexpr bool empty() const noexcept { return m_Begin == m_End; }
		static constexpr i64 stride() noexcept { return Stride; }
		static constexpr size_type abs_stride() noexcept { return (Stride > 0) ? Stride : -Stride; }

		constexpr span<T, dynamic_extent, -Stride> reverse() const noexcept
		{
			return span<T, dynamic_extent, -Stride>{m_End - Stride, size()};
		}

	  private:
		T* m_Begin{nullptr};
		T* m_End{nullptr};
	};

	namespace _priv
	{
		template <typename T>
		struct is_span : std::false_type
		{};
		template <typename T, size_t Extent, i64 Stride>
		struct is_span<span<T, Extent, Stride>> : std::true_type
		{
			static inline constexpr bool is_static  = Extent != dynamic_extent;
			static inline constexpr bool is_forward = Stride > 0;
			static inline constexpr bool is_reverse = Stride < 0;
		};
	} // namespace _priv

	template <typename T>
	struct is_span : _priv::is_span<std::remove_cvref_t<T>>
	{};

	template <typename T>
	inline constexpr auto is_span_v = is_span<T>::value;

	template <typename T>
	concept IsSpan = is_span_v<T>;

	template <IsSpan T>
	struct is_static_span
		: std::conditional_t<_priv::is_span<std::remove_cvref_t<T>>::is_static, std::true_type, std::false_type>
	{};

	template <typename T>
	inline constexpr auto is_static_span_v = is_static_span<T>::value;

	template <IsSpan T>
	struct is_dynamic_span
		: std::conditional_t<_priv::is_span<std::remove_cvref_t<T>>::is_static, std::false_type, std::true_type>
	{};

	template <typename T>
	inline constexpr auto is_dynamic_span_v = is_dynamic_span<T>::value;
} // namespace psl
#pragma GCC diagnostic pop
