#pragma once
#include "psl/allocator.h"
#include "psl/exception.h"
#include "psl/literals.h"

namespace psl
{
	// template <typename T>
	// using array = std::vector<T>;

	template <typename T>
	class array
	{
	  public:
		class iterator
		{
		  public:
			constexpr iterator() noexcept = default;
			constexpr iterator(T* data) noexcept : m_Data(data) {}
			constexpr iterator(const iterator& other) noexcept = default;
			constexpr iterator(iterator&& other) noexcept	  = default;
			constexpr iterator& operator=(const iterator& other) noexcept = default;
			constexpr iterator& operator=(iterator&& other) noexcept = default;

			constexpr bool operator==(const iterator& other) noexcept { return m_Data == other.m_Data; }
			constexpr bool operator!=(const iterator& other) noexcept { return m_Data != other.m_Data; }

			constexpr iterator& operator++(int) const noexcept
			{
				auto copy = *this;
				return ++copy;
			}
			constexpr iterator& operator++() noexcept
			{
				++m_Data;
				return *this;
			}

			constexpr iterator& operator--(int) const noexcept
			{
				auto copy = *this;
				return --copy;
			}
			constexpr iterator& operator--() noexcept
			{
				--m_Data;
				return *this;
			}

			operator T&() noexcept { return *m_Data; }

			operator const T&() const noexcept { return *m_Data; }

			T* operator->() const noexcept { return m_Data; }

			T& operator*() noexcept { return *m_Data; }
			const T& operator*() const noexcept { return *m_Data; }

		  private:
			T* m_Data{nullptr};
		};
		constexpr array() noexcept = default;
		constexpr array(psl::memory::allocator& allocator) noexcept : m_Allocator(&allocator){};
		~array()
		{
			using namespace psl::literals;
			if constexpr(!std::is_trivially_destructible_v<T>)
			{
				for(auto i = 0_sz, size = size(); i < size; ++i)
				{
					delete m_Begin[i];
				}
			}
			m_Allocator->deallocate(m_Begin);
		}

		T& operator[](size_t index) noexcept(!psl::config::exceptions)
		{
			PSL_EXCEPT_IF(index >= size(), "tried to access array with out of range index", std::out_of_range);
			return *(m_Begin + index);
		}

		const T& operator[](size_t index) const noexcept(!psl::config::exceptions)
		{
			PSL_EXCEPT_IF(index >= size(), "tried to access array with out of range index", std::out_of_range);
			return *(m_Begin + index);
		}

		template <typename... Args>
		auto& emplace_back(Args&&... args)
		{
			grow(size() + 1);
			new(m_End) T(std::forward<Args>(args)...);
			++m_End;
			return *(m_End - 1);
		}

		inline size_t size() const noexcept { return m_End - m_Begin; }
		inline size_t capacity() const noexcept { return m_Capacity - m_Begin; }

		auto begin() const noexcept { return iterator{m_Begin}; }
		auto end() const noexcept { return iterator{m_End}; }

	  private:
		void grow(size_t needed_size)
		{
			if(needed_size <= capacity()) return;

			auto res = m_Allocator->allocate_n<T>(needed_size * 2);

			PSL_EXCEPT_IF(!res, "could not allocate anymore", std::runtime_error);

			if(res)
			{
				auto oldsize = size();
				m_Begin		 = res.data;
				m_End		 = m_Begin + oldsize;
				m_Capacity   = res.data + needed_size * 2;
			}
		}
		T* m_Begin{nullptr};
		T* m_End{nullptr};
		T* m_Capacity{nullptr};
		psl::memory::allocator* m_Allocator{&psl::memory::default_allocator};
	};
} // namespace psl