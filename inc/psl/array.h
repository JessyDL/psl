#pragma once

#include <algorithm>
#include "psl/allocator.h"
#include "psl/exception.h"
#include "psl/literals.h"
#include "psl/concepts.h"
#include "psl/iterators.h"

namespace psl
{
	namespace tags
	{
		struct alloc_only_t
		{};

		constexpr auto alloc_only = alloc_only_t{};

		struct no_construct_t
		{};

		constexpr auto no_construct = no_construct_t{};

		struct no_destroy_t
		{};

		constexpr auto no_destroy = no_destroy_t{};

		struct no_value_init_t
		{};

		constexpr auto no_value_init = no_value_init_t{};

		struct no_deinit_t
		{};

		constexpr auto no_deinit = no_deinit_t{};
	} // namespace tags

	namespace config
	{
		template <typename T>
		using array_default_allocator = psl::default_allocator_t;
	}

	template <typename T, typename Allocator = config::array_default_allocator<config::default_setting_t>>
	requires psl::traits::IsPhysicallyAllocated<Allocator> class array
	{
		static constexpr bool is_trivially_destructible = std::is_trivially_destructible_v<std::remove_cv_t<T>>;
		static constexpr bool is_nothrow_destructible   = std::is_nothrow_destructible_v<std::remove_cv_t<T>>;

	  public:
		using value_type	 = std::remove_cv_t<T>;
		using reference		 = value_type&;
		using creference	 = const value_type&;
		using pointer		 = value_type*;
		using cpointer		 = const value_type*;
		using size_type		 = size_t;
		using iterator		 = psl::contiguous_range_iterator<value_type>;
		using const_iterator = psl::contiguous_range_iterator<const value_type>;

		using allocator_type = Allocator;

		constexpr array() noexcept = default;
		constexpr array(allocator_type& allocator) noexcept : m_Allocator(&allocator){};
		~array() noexcept(is_nothrow_destructible)
		{
			using namespace psl::literals;
			if constexpr(!is_trivially_destructible)
			{
				for(auto ptr = m_Begin; ptr != m_End; ++ptr)
				{
					ptr->~value_type();
				}
			}
			if(m_Allocator) m_Allocator->deallocate(m_Begin);
		}

		constexpr array(const array& other) : m_Allocator(other.m_Allocator)
		{
			reserve(other.capacity());
			for(const auto& element : other) emplace_back(element);
		}

		constexpr array(array&& other) noexcept
			: m_Begin(other.m_Begin), m_End(other.m_End), m_Capacity(other.m_Capacity), m_Allocator(other.m_Allocator)
		{
			other.m_Begin	= nullptr;
			other.m_End		 = nullptr;
			other.m_Capacity = nullptr;
		}
		constexpr array& operator=(const array& other)
		{
			if(this == &other) return *this;

			if constexpr(!is_trivially_destructible)
			{
				for(auto ptr = m_Begin; ptr != m_End; ++ptr)
				{
					ptr->~value_type();
				}
			}

			m_Allocator->deallocate(m_Begin);

			m_Allocator = other.m_Allocator;
			m_Begin		= nullptr;
			m_End		= nullptr;
			m_Capacity  = nullptr;

			reserve(other.capacity());
			for(const auto& element : other) emplace_back(element);

			return *this;
		}
		constexpr array& operator=(array&& other) noexcept(is_nothrow_destructible)
		{
			if(this == &other) return *this;

			if constexpr(!is_trivially_destructible)
			{
				for(auto ptr = m_Begin; ptr != m_End; ++ptr)
				{
					ptr->~value_type();
				}
			}
			m_Allocator->deallocate(m_Begin);

			m_Begin		= other.m_Begin;
			m_End		= other.m_End;
			m_Capacity  = other.m_Capacity;
			m_Allocator = other.m_Allocator;

			other.m_Begin	= nullptr;
			other.m_End		 = nullptr;
			other.m_Capacity = nullptr;

			return *this;
		}

		constexpr reference operator[](size_type index) noexcept(!psl::config::exceptions)
		{
			PSL_EXCEPT_IF(index >= size(), "tried to access array with out of range index", std::out_of_range);
			return *(m_Begin + index);
		}

		constexpr creference operator[](size_type index) const noexcept(!psl::config::exceptions)
		{
			PSL_EXCEPT_IF(index >= size(), "tried to access array with out of range index", std::out_of_range);
			return *(m_Begin + index);
		}

		constexpr reference at(size_type index) noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(index >= size(), "tried to access array with out of range index", std::out_of_range);
			return *(m_Begin + index);
		}

		constexpr creference at(size_type index) const noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(index >= size(), "tried to access array with out of range index", std::out_of_range);
			return *(m_Begin + index);
		}

		constexpr reference front() noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(0 >= size(), "tried to access array with out of range index", std::out_of_range);
			return *m_Begin;
		}

		constexpr creference front() const noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(0 >= size(), "tried to access array with out of range index", std::out_of_range);
			return *m_Begin;
		}

		constexpr reference back() noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(0 >= size(), "tried to access array with out of range index", std::out_of_range);
			return *(m_End - 1);
		}

		constexpr creference back() const noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(0 >= size(), "tried to access array with out of range index", std::out_of_range);
			return *(m_End - 1);
		}

		constexpr pointer data() noexcept { return m_Begin; }
		constexpr cpointer data() const noexcept { return m_Begin; }


		template <typename... Args>
		auto& emplace_back(Args&&... args)
		{
			grow(size() + 1);
			new(m_End) value_type(std::forward<Args>(args)...);
			++m_End;
			return *(m_End - 1);
		}

		constexpr bool full() const noexcept { return m_End == m_Capacity; }
		constexpr bool empty() const noexcept { return m_End == m_Begin; }
		constexpr size_type size() const noexcept { return m_End - m_Begin; }
		constexpr size_type capacity() const noexcept { return m_Capacity - m_Begin; }

		/**
		 * \brief Will resize the container to the given size, but in the case of growing, will not
		 * instantiate objects in the new memory, this is up to the caller to handle.
		 *
		 * \param[in] newSize
		 */
		constexpr void resize(tags::no_construct_t, size_type newSize)
		{
			if(newSize == size()) return;

			const auto oldSize = size();
			// for shrinking
			for(auto i = newSize; i < oldSize; ++i) (m_Begin + i)->~value_type();

			m_End = m_Begin + std::min(newSize, oldSize);

			apply_size_change(m_Allocator->template allocate_n<value_type>(newSize));
			m_End = m_Begin + newSize;
		}

		/**
		 * \brief Will resize the container to the given size, but in the case of growing, will not
		 * instantiate objects in the new memory, this is up to the caller to handle. In the case of shrinking, will not
		 * de-init the items either, this is also up to the caller.
		 *
		 * \param[in] newSize
		 */
		constexpr void resize(tags::alloc_only_t, size_type newSize)
		{
			if(newSize == size()) return;
			m_End = m_Begin + std::min(newSize, size());
			apply_size_change(m_Allocator->template allocate_n<value_type>(newSize));
			m_End = m_Begin + newSize;
		}

		constexpr void resize(size_type newSize)
		{
			if(newSize == size()) return;

			const auto oldSize = size();
			// for shrinking
			if constexpr(!is_trivially_destructible)
			{
				for(auto i = newSize; i < oldSize; ++i) (m_Begin + i)->~value_type();
			}

			m_End = m_Begin + std::min(newSize, oldSize);

			apply_size_change(m_Allocator->template allocate_n<value_type>(newSize));
			auto old_end = m_Begin + std::min(oldSize, newSize);
			m_End		 = m_Begin + newSize;
			for(auto ptr = old_end; ptr != m_End; ++ptr)
			{
				new(ptr) value_type{};
			}
		}

		constexpr void reserve(size_type newCapacity)
		{
			if(newCapacity <= capacity()) return;

			apply_size_change(m_Allocator->template allocate_n<value_type>(newCapacity));
		}

		constexpr size_type next_growth() const noexcept
		{
			using namespace psl::literals;
			auto newCapacity = std::max(capacity(), 2_sz);
			return newCapacity * newCapacity;
		}

		constexpr auto begin() noexcept { return iterator{m_Begin}; }
		constexpr auto end() noexcept { return iterator{m_End}; }
		constexpr auto begin() const noexcept { return const_iterator{m_Begin}; }
		constexpr auto end() const noexcept { return const_iterator{m_End}; }

		constexpr void erase(const size_type first,
							 const size_type count = 1) noexcept(!config::exceptions && is_nothrow_destructible)
		{
			PSL_EXCEPT_IF(first > size(), "the first element was beyond the end", std::range_error);
			PSL_EXCEPT_IF(first + count > size(), "the last element was beyond the end", std::range_error);

			if(count == 0) return;
			auto it = begin() + first;
			std::rotate(it, it + count, end());
			m_End -= count;
			if constexpr(!is_trivially_destructible)
			{
				for(auto ptr = m_End; ptr != m_End + count; ++ptr) ptr->~value_type();
			}
		}
		constexpr void erase(const_iterator first) noexcept(!config::exceptions && is_nothrow_destructible)
		{
			return erase(first, first + 1);
		}
		constexpr void erase(const_iterator first,
							 const_iterator last) noexcept(!config::exceptions && is_nothrow_destructible)
		{
			PSL_EXCEPT_IF(last < first, "the given range is negative (last is earlier than first)", std::range_error);
			PSL_EXCEPT_IF(first > end, "the first element was beyond the end", std::range_error);
			PSL_EXCEPT_IF(last > end, "the last element was beyond the end", std::range_error);

			auto distance = std::distance(first, last);
			if(distance == 0) return;

			std::rotate(first, last, end());
			m_End -= distance;
			if constexpr(!is_trivially_destructible)
			{
				for(auto it = first; it != last; ++it) it->~value_type();
			}
		}

		constexpr void clear() noexcept(!config::exceptions && is_nothrow_destructible)
		{
			if constexpr(!is_trivially_destructible)
			{
				for(auto ptr = m_Begin; ptr != m_End; ++ptr) (*ptr).~value_type();
			}
			m_End = m_Begin;
		}
		constexpr void clear(tags::no_destroy_t) noexcept { m_End = m_Begin; }

		constexpr void swap(array& other) noexcept
		{
			std::swap(m_Begin, other.m_Begin);
			std::swap(m_End, other.m_End);
			std::swap(m_Capacity, other.m_Capacity);
			std::swap(m_Allocator, other.m_Allocator);
		}

	  private:
		constexpr void grow(size_type needed_size)
		{
			if(needed_size <= capacity()) return;
			apply_size_change(m_Allocator->template allocate_n<value_type>(needed_size * 2));
		}

		constexpr void apply_size_change(auto res)
		{
			PSL_EXCEPT_IF(!res, "could not allocate anymore", std::runtime_error);
			if(res)
			{
				if(m_Begin != res.data)
				{
					move_elements(res.data);
					m_End = res.data + size();
					m_Allocator->deallocate(m_Begin);
					m_Begin = res.data;
				}

				m_Capacity = (pointer)ralign_to((size_type)res.tail, sizeof(value_type));
			}
		}

		constexpr void move_elements(pointer newLocation)
		{
			if(m_Begin == newLocation) return;

			for(auto ptr = m_Begin; ptr != m_End; ++ptr)
			{
				new(newLocation++) value_type(std::move(*ptr));
			}
		}

		pointer m_Begin{nullptr};
		pointer m_End{nullptr};
		pointer m_Capacity{nullptr};
		allocator_type* m_Allocator{&psl::default_allocator};
	};
} // namespace psl