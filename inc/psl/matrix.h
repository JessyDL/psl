#pragma once
#include <array>
#include <psl/span.h>

#define ACCESSOR(name, index0, index1)                                                                                 \
	constexpr reference name##index0##index1() noexcept requires(Rows > index0 && Columns > index1)                    \
	{                                                                                                                  \
		return m_Data[index0 * Columns + index1];                                                                      \
	}                                                                                                                  \
	constexpr const_reference name##index0##index1() const noexcept requires(Rows > index0 && Columns > index1)        \
	{                                                                                                                  \
		return m_Data[index0 * Columns + index1];                                                                      \
	}

namespace psl
{
	namespace config
	{
		template <typename T>
		static constexpr bool matrix_row_major = true;
	}
	template <typename T, size_t Rows, size_t Columns,
			  bool RowMajor = config::matrix_row_major<config::default_setting_t>>
	class tmat
	{
	  public:
		using value_type	  = T;
		using reference		  = T&;
		using const_reference = const T&;

		template <typename... Ys>
		constexpr tmat(Ys&&... values) noexcept : m_Data({std::forward<Ys>(values)...})
		{}
		ACCESSOR(a, 0, 0)
		ACCESSOR(a, 0, 1)
		ACCESSOR(a, 0, 2)
		ACCESSOR(a, 0, 3)
		ACCESSOR(a, 1, 0)
		ACCESSOR(a, 1, 1)
		ACCESSOR(a, 1, 2)
		ACCESSOR(a, 1, 3)
		ACCESSOR(a, 2, 0)
		ACCESSOR(a, 2, 1)
		ACCESSOR(a, 2, 2)
		ACCESSOR(a, 2, 3)
		ACCESSOR(a, 3, 0)
		ACCESSOR(a, 3, 1)
		ACCESSOR(a, 3, 2)
		ACCESSOR(a, 3, 3)

		template <size_t I>
		constexpr span<value_type, Columns> row() noexcept requires(I < Rows)
		{
			return span<value_type, Columns>{m_Data.data() + Columns * I};
		}
		constexpr span<value_type, Columns> row(size_t index) noexcept
		{
			return span<value_type, Columns>{m_Data.data() + Columns * index};
		}
		constexpr size_t rows() const noexcept { return Rows; }

		template <size_t I>
		constexpr span<value_type, Rows, Columns * sizeof(value_type)> column() noexcept requires(I < Columns)
		{
			return span<value_type, Rows, Columns * sizeof(value_type)>{m_Data.data() + I};
		}
		constexpr span<value_type, Rows, Columns * sizeof(value_type)> column(size_t index) noexcept
		{
			return span<value_type, Rows, Columns * sizeof(value_type)>{m_Data.data() + index};
		}
		constexpr size_t columns() const noexcept { return Columns; }

	  private:
		std::array<T, Rows * Columns> m_Data;
	};
} // namespace psl

#undef ACCESSOR