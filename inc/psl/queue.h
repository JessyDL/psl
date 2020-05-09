#pragma once
#include "psl/array.h"
#include "psl/exception.h"

namespace psl
{
	template <typename T>
	class queue
	{
	  public:
		using value_type	 = std::remove_cv_t<T>;
		using reference		 = value_type&;
		using creference	 = const value_type&;
		using pointer		 = value_type*;
		using cpointer		 = const value_type*;
		using size_type		 = size_t;
		using iterator		 = contiguous_ring_range_iterator<value_type>;
		using const_iterator = contiguous_ring_range_iterator<const value_type>;

		constexpr queue() noexcept(std::is_nothrow_constructible_v<psl::array<T>>) = default;
		constexpr queue(psl::default_allocator_t& allocator) noexcept(
			std::is_nothrow_constructible_v<psl::array<T>, psl::default_allocator_t&>)
			: m_Data(allocator){};

		~queue() { clear(); }
		queue(const queue& other)
			: m_Data(other.m_Data), m_Head(m_Data.data()), m_Tail(m_Head + other.size()), m_Empty(other.m_Empty)
		{
			if(m_Tail >= m_Data.data() + m_Data.size())
				m_Tail = (m_Tail - (m_Data.data() + m_Data.size())) + m_Data.data();
		}
		queue(queue&& other)
			: m_Data(std::move(other.m_Data)), m_Head(m_Data.data()), m_Tail(m_Head + other.size()),
			  m_Empty(other.m_Empty)
		{
			if(m_Tail >= m_Data.data() + m_Data.size())
				m_Tail = (m_Tail - (m_Data.data() + m_Data.size())) + m_Data.data();
		}
		queue& operator=(const queue& other)
		{
			if(this == &other) return *this;

			clear();

			m_Data  = other.m_Data;
			m_Head  = m_Data.data();
			m_Tail  = m_Head + other.size();
			m_Empty = other.m_Empty;

			if(m_Tail >= m_Data.data() + m_Data.size())
				m_Tail = (m_Tail - (m_Data.data() + m_Data.size())) + m_Data.data();
			return *this;
		}
		queue& operator=(queue&& other)
		{
			if(this == &other) return *this;

			clear();

			m_Data  = std::move(other.m_Data);
			m_Head  = m_Data.data();
			m_Tail  = m_Head + other.size();
			m_Empty = other.m_Empty;

			if(m_Tail >= m_Data.data() + m_Data.size())
				m_Tail = (m_Tail - (m_Data.data() + m_Data.size())) + m_Data.data();
			return *this;
		}

		creference front() const noexcept(!config::exceptions) { return *m_Head; }
		reference front() noexcept(!config::exceptions) { return *m_Head; }
		creference back() const noexcept(!config::exceptions) { return *m_Tail; }
		reference back() noexcept(!config::exceptions) { return *m_Tail; }

		template <typename... Args>
		reference emplace()
		{
			PSL_NOT_IMPLEMENTED(0);
		}

		void erase(const_iterator first, const_iterator last) { PSL_NOT_IMPLEMENTED(0); }

		void clear() { PSL_NOT_IMPLEMENTED(0); }

	  private:
		psl::array<value_type> m_Data{};
		pointer m_Head{m_Data.data()};
		pointer m_Tail{m_Head};
		bool m_Empty{true};
	};
} // namespace psl