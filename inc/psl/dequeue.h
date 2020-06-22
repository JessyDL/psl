#pragma once

#include <algorithm>

#include "psl/allocator.h"
#include "psl/array.h"
#include "psl/literals.h"
#include "psl/config.h"
#include "psl/exception.h"
#include "psl/iterators.h"

#include <vector>
namespace psl
{
	namespace config
	{
		template <typename T = default_setting_t>
		using dequeue_allocator = array_allocator<T>;

		template <typename Value, typename T = default_setting_t>
		constexpr size_t dequeue_sbo = (64 - (array_sbo_size<Value, T> * sizeof(Value) + sizeof(std::uintptr_t) * 2 +
											  sizeof(bool))) /
									   sizeof(Value);
	} // namespace config

	/**
	 * \brief Double ended queue.
	 * \details A collection type that has the ability to add/remove elements from the front and back. Internally uses
	 * psl::array<T>, and so has the same constraints, performance characteristics, and iterator stability.
	 *
	 * \tparam T
	 * \tparam config::dequeue_allocator<config::default_setting_t>
	 */
	template <typename T, size_t SBO = config::dequeue_sbo<T>, typename Allocator = config::dequeue_allocator<>>
	class dequeue
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

		using array_type	 = psl::array<T, SBO, Allocator>;
		using allocator_type = Allocator;

		dequeue() noexcept(std::is_nothrow_constructible_v<array_type>)
		{
			m_Head = m_Data.data();
			m_Tail = m_Data.data();
		}


		dequeue(IsIntegral auto size) noexcept(std::is_nothrow_constructible_v<array_type, decltype(size)>)
			: m_Data(size)
		{}
		dequeue(IsIntegral auto size,
				const T& value) noexcept(std::is_nothrow_constructible_v<array_type, decltype(size), const T&>)
			: m_Data(size, value)
		{}

		dequeue(allocator_type& allocator) noexcept(std::is_nothrow_constructible_v<array_type, allocator_type&>)
			: m_Data(allocator)
		{
			m_Head = m_Data.data();
			m_Tail = m_Data.data();
		}

		~dequeue() { clear(); }

		dequeue(const dequeue& other)
			: m_Data(other.m_Data), m_Head(m_Data.data()), m_Tail(m_Head + other.size()), m_Empty(other.m_Empty)
		{
			if(m_Tail >= m_Data.data() + m_Data.size())
				m_Tail = (m_Tail - (m_Data.data() + m_Data.size())) + m_Data.data();
		}
		dequeue(dequeue&& other)
			: m_Data(std::move(other.m_Data)), m_Head(m_Data.data()), m_Tail(m_Head + other.size()),
			  m_Empty(other.m_Empty)
		{
			if(m_Tail >= m_Data.data() + m_Data.size())
				m_Tail = (m_Tail - (m_Data.data() + m_Data.size())) + m_Data.data();
		}
		dequeue& operator=(const dequeue& other)
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
		dequeue& operator=(dequeue&& other)
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

		auto& operator[](size_type index) noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(index >= size(), "index exceeded size", std::range_error);
			auto offset = m_Head - m_Data.data();
			index		= (index + offset) % m_Data.size();
			return m_Data[index];
		}

		const auto& operator[](size_type index) const noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(index >= size(), "index exceeded size", std::range_error);
			auto offset = m_Head - m_Data.data();
			index		= (index + offset) % m_Data.size();
			return m_Data[index];
		}

		creference front() const noexcept(!config::exceptions) { return *m_Head; }
		reference front() noexcept(!config::exceptions) { return *m_Head; }
		creference back() const noexcept(!config::exceptions) { return *m_Tail; }
		reference back() noexcept(!config::exceptions) { return *m_Tail; }

		bool full() const noexcept { return m_Head == m_Tail && !m_Empty; }
		bool empty() const noexcept { return m_Empty; }
		size_type size() const noexcept
		{
			if(m_Tail > m_Head)
			{
				return m_Tail - m_Head;
			}
			else if(m_Tail < m_Head)
			{
				cpointer begin_ptr = m_Data.data();
				auto offset		   = m_Head - begin_ptr;
				return (m_Data.size() - offset) + (m_Tail - begin_ptr);
			}
			else if(m_Tail == m_Head && !m_Empty)
			{
				return m_Data.size();
			}
			return 0;
		}

		size_type capacity() const noexcept { return m_Data.size(); }

		auto begin() noexcept { return iterator{m_Head, m_Data.data(), m_Data.data() + m_Data.size(), 0}; };
		auto end() noexcept { return iterator{m_Tail, m_Data.data(), m_Data.data() + m_Data.size(), size()}; };
		auto begin() const noexcept { return const_iterator{m_Head, m_Data.data(), m_Data.data() + m_Data.size(), 0}; };
		auto end() const noexcept
		{
			return const_iterator{m_Tail, m_Data.data(), m_Data.data() + m_Data.size(), size()};
		};

		void resize(size_type newSize)
		{
			if(newSize == size()) return;

			const auto oldSize = size();
			if(newSize < oldSize) erase(begin() + newSize, end());

			reorient_data();

			m_Data.resize(tags::alloc_only, newSize);
			m_Head = m_Data.data();
			m_Tail = m_Head + newSize;

			if(oldSize < newSize)
			{
				for(auto ptr = m_Head + oldSize; ptr != m_Head + newSize; ++ptr)
				{
					new(ptr) T{};
				}
			}
		}

		void reserve(size_type newCapacity)
		{
			if(newCapacity <= capacity()) return;

			const auto current_size = size();

			reorient_data();

			m_Data.resize(tags::alloc_only, newCapacity);

			m_Head = m_Data.data();
			m_Tail = m_Head + current_size;
		}

		void erase(const_iterator first, const_iterator last)
		{
			for(auto it = first; it != last; ++it)
			{
				it->~T();
			}

			if(&*first == m_Head && &*last == m_Tail) m_Empty = true;

			m_Tail -= last - first;
			if(m_Tail < m_Data.data()) m_Tail = (m_Data.data() + m_Data.size()) - (m_Data.data() - m_Tail);
		}

	  private:
		/**
		 * \brief Reorients the data so that m_Head is the first element in the range.
		 *
		 */
		void reorient_data()
		{
			if(m_Data.data() == m_Head) return;

			auto data_begin = m_Data.begin();
			auto count		= m_Head - m_Data.data();
			std::rotate(data_begin, std::next(data_begin, count), m_Data.end());
			m_Head = m_Data.data();
			m_Tail = m_Head + size();
		}
		void grow_if_necesary()
		{
			if(full() || m_Data.size() == 0)
			{
				auto esize = size();
				reorient_data();

				m_Data.resize(tags::alloc_only, details::array_growth_for(m_Data.capacity()));
				m_Head = m_Data.data();
				m_Tail = m_Head + esize;
			}
		}

	  public:
		template <typename... Args>
		auto& emplace_front(Args&&... args)
		{
			grow_if_necesary();
			auto* data_ptr = m_Data.data();
			m_Head		   = (data_ptr == m_Head) ? m_Data.data() + m_Data.size() - 1 : m_Head - 1;

			size_t index = m_Head - data_ptr;
			m_Empty		 = false;
			return *new(&m_Data[index]) T(std::forward<Args...>(args)...);
		}

		template <typename... Args>
		auto& emplace_back(Args&&... args)
		{
			grow_if_necesary();
			auto* data_ptr = m_Data.data();

			using namespace psl::literals;
			size_t index = m_Tail - data_ptr;
			m_Tail		 = (index == m_Data.capacity() - 1_sz) ? data_ptr : m_Tail + 1;
			m_Empty		 = false;
			return *new(&m_Data[index]) T(std::forward<Args...>(args)...);
		}

		[[nodiscard]] value_type&& pop_back() noexcept
		{
			m_Empty	= size() == 1;
			auto index = m_Tail - &*m_Data.begin();
			if(index == 0)
			{
				m_Tail = &*m_Data.end();
			}

			m_Tail -= 1;

			return std::move(*(m_Tail));
		}

		[[nodiscard]] value_type&& pop_front() noexcept
		{
			m_Empty = size() == 1;
			using namespace psl::literals;
			size_t index = m_Head - &*m_Data.begin();

			if(size() - 1 == 0)
			{
				m_Head = &*m_Data.begin();
				m_Tail = m_Head;
			}
			else
			{
				++m_Head;
				if(index == m_Data.size() - 1_sz) m_Head = &*m_Data.begin();
			}
			return std::move(m_Data[index]);
		}

		void clear() noexcept(std::is_nothrow_destructible_v<T>) { erase(begin(), end()); }

	  private:
		array_type m_Data{};
		pointer m_Head{m_Data.data()};
		pointer m_Tail{m_Head};
		bool m_Empty{true};
	};
} // namespace psl