#pragma once

#include <algorithm>
#include "psl/allocator.h"
#include "psl/exception.h"
#include "psl/literals.h"
#include "psl/concepts.h"
#include "psl/iterators.h"

namespace psl
{
	// template <typename T>
	// using array = std::vector<T>;

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

	template <typename T>
	class array
	{
	  public:
		using value_type	 = std::remove_cv_t<T>;
		using reference		 = value_type&;
		using creference	 = const value_type&;
		using pointer		 = value_type*;
		using cpointer		 = const value_type*;
		using size_type		 = size_t;
		using iterator		 = psl::contiguous_range_iterator<value_type>;
		using const_iterator = psl::contiguous_range_iterator<const value_type>;

		constexpr array() noexcept = default;
		constexpr array(psl::allocator& allocator) noexcept : m_Allocator(&allocator){};
		~array()
		{
			using namespace psl::literals;
			if constexpr(!std::is_trivially_destructible_v<T>)
			{
				for(auto i = 0_sz, size = this->size(); i < size; ++i)
				{
					(*(m_Begin + i)).~T();
				}
			}
			m_Allocator->deallocate(m_Begin);
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
			new(m_End) T(std::forward<Args>(args)...);
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
			for(auto i = newSize; i < oldSize; ++i) (m_Begin + i)->~T();

			apply_size_change(m_Allocator->allocate_n<T>(newSize));
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

			apply_size_change(m_Allocator->allocate_n<T>(newSize));
			m_End = m_Begin + newSize;
		}

		constexpr void resize(size_type newSize)
		{
			if(newSize == size()) return;

			const auto oldSize = size();
			// for shrinking
			for(auto i = newSize; i < oldSize; ++i) (m_Begin + i)->~T();

			apply_size_change(m_Allocator->allocate_n<T>(newSize));
			auto old_end = m_Begin + std::min(oldSize, newSize);
			m_End		 = m_Begin + newSize;
			for(auto ptr = old_end; ptr != m_End; ++ptr)
			{
				new(ptr) T{};
			}
		}

		constexpr void reserve(size_type newCapacity)
		{
			if(newCapacity <= capacity()) return;

			apply_size_change(m_Allocator->allocate_n<T>(newCapacity));
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

		constexpr void erase(const size_type first, const size_type count = 1)
		{
			auto it = begin() + first;
			std::rotate(it, it + count, end());
			m_End -= count;
			if constexpr(!std::is_trivially_destructible_v<T>)
			{
				for(auto ptr = m_End; ptr != m_End + count; ++ptr) (*ptr).~T();
			}
		}

		constexpr void clear() noexcept
		{
			if constexpr(!std::is_trivially_destructible_v<T>)
			{
				for(auto ptr = m_Begin; ptr != m_End; ++ptr) (*ptr).~T();
			}
			m_End = m_Begin;
		}
		constexpr void clear(tags::no_destroy_t) noexcept requires std::is_trivially_destructible_v<T>
		{
			m_End = m_Begin;
		}

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
			apply_size_change(m_Allocator->allocate_n<T>(needed_size * 2));
		}

		constexpr void apply_size_change(auto res)
		{
			PSL_EXCEPT_IF(!res, "could not allocate anymore", std::runtime_error);
			if(res)
			{
				move_elements(res.data);
				m_End   = res.data + size();
				m_Begin = res.data;

				m_Capacity = (T*)ralign_to((size_type)res.tail, sizeof(T));
			}
		}

		constexpr void move_elements(T* newLocation)
		{
			if(m_Begin == newLocation) return;

			for(auto ptr = m_Begin; ptr != m_End; ++ptr)
			{
				new(newLocation++) T(std::move(*ptr));
			}
		}

		T* m_Begin{nullptr};
		T* m_End{nullptr};
		T* m_Capacity{nullptr};
		psl::allocator* m_Allocator{&psl::default_allocator};
	};
} // namespace psl