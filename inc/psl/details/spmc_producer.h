#pragma once
#include <algorithm>
#include <optional>
#include <atomic>
#include <memory>

#include "psl/algorithms.h"

namespace psl::spmc
{
	template <typename T>
	class consumer;

	/// \brief SPMC based on Chase-Lev deque
	///
	/// \details This class is based on Chase-Lev's deque implementation of a Single Producer Multi Consumer (SPMC).
	/// The producer should only be used on a single thread, and the 'psl::spmc::consumer' variant should be used in
	/// the consuming threads. This offers a safe API.
	/// \warning Due to the nature of this container, an item might exist up to two times for a brief moment (when the
	/// internal buffer resizes).
	/// This means that contained items need to support copy symantics.
	template <typename T>
	class producer final
	{
		friend class consumer<T>;

		/// \brief Wrapper over ring_array<T>
		///
		/// \details This wrapper class over a ring_array keeps track of its internal offset.
		/// That will be used by the producer as it schedules more and more items.
		/// The ring_array can only be resized by invoking the buffer::copy method.
		/// Internally you can keep incrementing your access indices, as long as you never exceed the range
		/// being used (range <= buffer::capacity(), where range == begin to end indices).
		/// When you exceed the capacity, you need to buffer::copy a new one.
		struct buffer
		{
		  public:
			buffer(size_t capacity) { m_Data.resize(psl::next_pow_of(2, std::max<size_t>(1u, capacity))); };
			~buffer() {}
			buffer(const buffer& other) noexcept = default;
			buffer(buffer&& other) noexcept		 = default;
			buffer& operator=(const buffer& other) noexcept = default;
			buffer& operator=(buffer&& other) noexcept = default;
			void set(int64_t index, T&& value) noexcept
			{
				m_Data[((index - static_cast<int64_t>(m_Offset)) & (m_Data.size() - 1))] = std::forward<T>(value);
			}

			auto& at(int64_t index) const noexcept
			{
				return m_Data[(index - static_cast<int64_t>(m_Offset)) & (m_Data.size() - 1)];
			}

			auto& at(int64_t index) noexcept
			{
				return m_Data[(index - static_cast<int64_t>(m_Offset)) & (m_Data.size() - 1)];
			}

			/// \brief Returns a logical continuation buffer based on this buffer
			///
			/// \details Copies the current buffer into a new one of 'at least' the given capacity. It will grow to the
			/// next logical power of 2 that can also contain the begin-end items.
			buffer* copy(size_t begin, size_t end, size_t capacity = 0)
			{
				capacity = std::max<size_t>(32, psl::next_pow_of(2, std::max(capacity, (end - begin) + 1)));

				buffer* ptr   = new buffer(capacity);
				ptr->m_Offset = begin;
				for(size_t i = begin; i != end; ++i)
				{
					ptr->set(i, T{at(i)});
				}
				return ptr;
			}

			/// \returns Max continuous range of items in the buffer.
			size_t capacity() const noexcept { return m_Data.size(); };

		  private:
			psl::array<T> m_Data{};
			size_t m_Offset{0};
		};

	  public:
		producer(int64_t capacity = 2)
		{
			capacity = psl::next_pow_of(2, std::max(capacity, (int64_t)1));
			m_Begin.store(0, std::memory_order_relaxed);
			m_End.store(0, std::memory_order_relaxed);
			auto cont = std::make_shared<buffer>(capacity);
			std::atomic_store_explicit(&m_Data, cont, std::memory_order_relaxed);
		};
		~producer()						= default;
		producer(const producer& other) = delete;
		producer(producer&& other)		= default;
		producer& operator=(const producer& other) = delete;
		producer& operator=(producer&& other) = default;

		/// \Returns a consumer that is linked to the current producer, to be used in other threads.
		::psl::spmc::consumer<T> consumer() noexcept;

		bool empty() const noexcept
		{
			auto begin = m_Begin.load(std::memory_order_relaxed);
			auto end   = m_End.load(std::memory_order_relaxed);
			return end <= begin;
		}

		/// \Returns the current count of all elements in the producer.
		size_t size() const noexcept { return static_cast<size_t>(ssize()); }

		/// \Returns the current count of all elements in the producer.
		int64_t ssize() const noexcept
		{
			auto begin = m_Begin.load(std::memory_order_relaxed);
			auto end   = m_End.load(std::memory_order_relaxed);
			return std::max<int64_t>(end - begin, 0);
		}

		/// \brief Resizes the internal buffer to the given size
		///
		/// \details Tries to resize to the given size, it will automatically align itself to the next power of 2 if
		/// the value isn't a power of 2 already. The minimum size will be 'at least' equal to, or bigger than, the
		/// current size (not capacity) of the internal buffer.
		void resize(size_t size)
		{
			auto cont = m_Data.load(std::memory_order_relaxed);
			size	  = psl::next_pow_of(2, size);
			if(size == (int64_t)cont->capacity()) return;

			auto begin   = m_Begin.load(std::memory_order_relaxed);
			auto end	 = m_End.load(std::memory_order_relaxed);
			auto newCont = std::shared_ptr<buffer>{cont->copy(begin, end)};
			std::atomic_store_explicit(&m_Data, newCont, std::memory_order_relaxed);
		}

		/// \brief Push the given element onto the end of the deque.
		///
		/// \details Will push the given element onto the deque if enough space is present.
		/// If not enough space is present it will construct a new internal buffer that contains at least enough space.
		/// If a previous (unused) buffer is present it will now clean that buffer up.
		/// \warning Callable only on the owning thread, do not call from multiple threads.
		void push(T&& value)
		{
			int64_t end   = m_End.load(std::memory_order_relaxed);
			int64_t begin = m_Begin.load(std::memory_order_acquire);
			auto cont	 = std::atomic_load_explicit(&m_Data, std::memory_order_relaxed);

			if(static_cast<int64_t>(cont->capacity()) < (end - begin) + 1)
			{
				cont = std::shared_ptr<buffer>{cont->copy(begin, end)};
				std::atomic_store_explicit(&m_Data, cont, std::memory_order_relaxed);
			}

			cont->set(end, std::forward<T>(value));
			std::atomic_thread_fence(std::memory_order_release);
			m_End.store(end + 1, std::memory_order_relaxed);
		}

		/// \brief Pops an element (if any are left) off the deque from the end.
		///
		/// \details Tries to pop an element off the end of the deque.
		/// \warning Only callable from the owning thread, otherwise the results will be undefined.
		std::optional<T> pop() noexcept
		{
			int64_t end = m_End.load(std::memory_order_relaxed) - 1;
			auto cont   = std::atomic_load_explicit(&m_Data, std::memory_order_relaxed);
			m_End.store(end, std::memory_order_relaxed);
			std::atomic_thread_fence(std::memory_order_seq_cst);
			int64_t begin = m_Begin.load(std::memory_order_relaxed);

			std::optional<T> res{std::nullopt};

			if(begin <= end)
			{
				res = cont->at(end);
				if(begin == end)
				{
					if(!m_Begin.compare_exchange_strong(begin, begin + 1, std::memory_order_seq_cst,
														std::memory_order_relaxed))
					{
						res = std::nullopt;
					}
					m_End.store(end + 1, std::memory_order_relaxed);
				}
			}
			else
			{
				m_End.store(end + 1, std::memory_order_relaxed);
			}
			return res;
		}

		void clear() noexcept
		{
			auto end = m_End.load(std::memory_order_relaxed);
			m_Begin.store(end, std::memory_order_seq_cst);
		}

	  private:
		/// \brief Pops an element (if any are left) off the deque from the front.
		///
		/// \details To be used by consumer threads, this gives a thread safe way of stealing items from the deque.
		/// It can be called by any thread, but is only exposed to the consumer class.
		std::optional<T> steal() noexcept
		{
			int64_t begin = m_Begin.load(std::memory_order_acquire);
			std::atomic_thread_fence(std::memory_order_seq_cst);
			int64_t end = m_End.load(std::memory_order_acquire);

			std::optional<T> res{std::nullopt};

			if(begin < end)
			{
				auto data = std::atomic_load_explicit(&m_Data, std::memory_order_consume);
				res		  = data->at(begin);

				if(!m_Begin.compare_exchange_strong(begin, begin + 1, std::memory_order_seq_cst,
													std::memory_order_relaxed))
				{
					return std::nullopt;
				}
			}

			return res;
		}

		std::atomic_int64_t m_Begin;
		std::atomic_int64_t m_End;
		std::shared_ptr<buffer> m_Data;
		buffer* m_Last{nullptr};
	};

	template <typename T>
	::psl::spmc::consumer<T> producer<T>::consumer() noexcept
	{
		return ::psl::spmc::consumer<T>(*this);
	}
} // namespace psl::spmc
