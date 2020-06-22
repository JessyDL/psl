#pragma once

#include <algorithm>
#include <cstring> // memcpy
#include "psl/allocator.h"
#include "psl/exception.h"
#include "psl/literals.h"
#include "psl/concepts.h"
#include "psl/iterators.h"
#include "psl/details/sbo_storage.h"
#include "psl/enum.h"

namespace psl
{
	enum class construction_traits_t : uint8_t
	{
		// constructors are called
		construct = 1 << 0,
		// elements are initialized (no-op for trivial types, default for nontrivial's)
		default_init = construct | 1 << 1,
		// all elements are value initialized
		value_init = construct | 1 << 2,
		// destructors are called
		destruct = 1 << 3,


		// only does allocation/deallocation of memory
		minimal = 0,
		// full behaviour, as expected
		default_behaviour = value_init | destruct
	};
	namespace config
	{
		template <>
		constexpr auto enable_enum_ops<psl::construction_traits_t> = enum_ops_t::BIT | enum_ops_t::LOGICAL;
	}

	namespace tags
	{
		/**
		 * \brief Only guarantees the backing memory is present for the given element(s).
		 * \note For a comparable standard library function, think of the std::vector's reserve.
		 *
		 */
		struct alloc_only_t
		{};

		constexpr auto alloc_only = alloc_only_t{};

		/**
		 * \brief Allocates, and "acts" as if elements have been construced.
		 * \details As an example in an array, this would act like a resize, but without invoking constructors.
		 * \warning This is potentially dangerous, it is up to the user (you) to construct these objects.
		 *
		 */
		struct no_construct_t
		{};

		constexpr auto no_construct = no_construct_t{};

		/**
		 * \brief
		 *
		 */
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
		template <typename T = default_setting_t>
		using array_allocator = psl::default_allocator_t;

		template <typename Value, typename T = default_setting_t>
		constexpr size_t array_sbo_size = (sizeof(std::uintptr_t) * 8 -
										   (sizeof(size_t) * 2 + sizeof(config::array_allocator<>))) /
										  sizeof(Value);

		template <auto object_size, auto wanted_size, auto value_size>
		constexpr size_t sbo_count = (wanted_size - std::min(object_size, wanted_size)) / value_size;
	} // namespace config

	inline namespace details
	{
		constexpr auto array_growth_for(auto capacity) noexcept
		{
			auto newCapacity = std::max(capacity, decltype(capacity){2});
			return newCapacity * newCapacity;
		}

		template <typename T, size_t SBO = psl::config::array_sbo_size<T>,
				  typename Allocator = config::array_allocator<>, bool AliasedSBO = false>
		requires psl::traits::IsPhysicallyAllocated<Allocator> class array_base
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

			using allocator_type = Allocator;

			constexpr bool is_stored_inlined() const noexcept
			{
				if constexpr(SBO == 0)
					return false;
				else
					return SBO == m_Capacity;
			}

			constexpr size_type size() const noexcept { return m_Size; }
			constexpr size_type capacity() const noexcept { return m_Capacity; }

			constexpr pointer data() noexcept
			{
				if constexpr(SBO == 0 || !AliasedSBO)
					return m_Storage.ext;
				else
				{
					if(is_stored_inlined())
						[[unlikely]] return m_Storage.local.data();
					else
						[[likely]] return m_Storage.ext;
				}
			}
			constexpr cpointer data() const noexcept
			{
				if constexpr(SBO == 0 || !AliasedSBO)
					return m_Storage.ext;
				else
				{
					if(is_stored_inlined())
						[[unlikely]] return m_Storage.local.data();
					else
						[[likely]] return m_Storage.ext;
				}
			}
			constexpr cpointer cdata() const noexcept
			{
				if constexpr(SBO == 0 || !AliasedSBO)
					return m_Storage.ext;
				else
				{
					if(is_stored_inlined())
						[[unlikely]] return m_Storage.local.data();
					else
						[[likely]] return m_Storage.ext;
				}
			}


		  protected:
			constexpr array_base() noexcept(!config::exceptions) : array_base(size_type{0}, psl::default_allocator){};
			constexpr array_base(size_type size) noexcept(!config::exceptions)
				: array_base(size, psl::default_allocator)
			{}
			constexpr array_base(const allocator_type& allocator) noexcept(!config::exceptions)
				: array_base(size_type{0}, allocator)
			{}

			constexpr array_base(size_type size, const allocator_type& allocator) noexcept(!config::exceptions)
				: m_Size(size), m_Allocator(allocator)
			{
				if constexpr(SBO == 0) m_Storage.ext = nullptr;

				if constexpr(!AliasedSBO) m_Storage.ext = m_Storage.local.data();

				if(size <= SBO)
				{
					m_Capacity = SBO;
				}
				else
				{
					auto res = m_Allocator.template allocate_n<value_type>(size);
					PSL_EXCEPT_IF(!res, "could not allocate", std::runtime_error);

					m_Storage.ext = res.data;
					m_Capacity	= (pointer)ralign_to((size_type)res.tail, sizeof(value_type)) - m_Storage.ext;
				}
			}

			constexpr ~array_base()
			{
				if(!is_stored_inlined())
				{
					m_Allocator.deallocate(m_Storage.ext);
				}
			}

			void reset_to_capacity() noexcept
			{
				if(!is_stored_inlined())
				{
					m_Storage.ext = nullptr;
					if constexpr(!AliasedSBO) m_Storage.ext = m_Storage.local.data();
				}
				m_Capacity = SBO;
				m_Size	 = 0;
			}

			void deallocate()
			{
				if(!is_stored_inlined())
				{
					m_Allocator.deallocate(m_Storage.ext);
				}
				m_Capacity = SBO;
				m_Size	 = 0;
			}


			template <typename Fn>
			void reallocate(size_type size, Fn&& move_fn)
			{
				if constexpr(SBO == 0)
				{
					if(size == 0)
					{
						move_fn(m_Storage.ext, nullptr, m_Size);
						m_Allocator.deallocate(m_Storage.ext);

						m_Storage.ext = nullptr;
						m_Capacity	= 0;
						return;
					}
					auto res = m_Allocator.template allocate_n<value_type>(size);
					PSL_EXCEPT_IF(!res, "could not allocate", std::runtime_error);

					move_fn(m_Storage.ext, res.data, m_Size);
					m_Allocator.deallocate(m_Storage.ext);

					m_Storage.ext = res.data;
					m_Capacity	= (pointer)ralign_to((size_type)res.tail, sizeof(value_type)) - m_Storage.ext;
				}
				else
				{
					if(size <= SBO)
					{
						// no change, the minimum capacity is always the SBO size
						if(is_stored_inlined())
						{
							return;
						}
						// transition from ext storage to sbo storage
						else
						{
							auto storage = m_Storage.ext;
							move_fn(storage, m_Storage.local.data(), m_Size);
							m_Allocator.deallocate(storage);
							m_Capacity = SBO;
							if constexpr(!AliasedSBO) m_Storage.ext = m_Storage.local.data();
						}
					}
					else
					{
						auto res = m_Allocator.template allocate_n<value_type>(size);
						PSL_EXCEPT_IF(!res, "could not allocate", std::runtime_error);

						// transition from sbo storage to ext storage
						if(is_stored_inlined())
						{
							move_fn(m_Storage.local.data(), res.data, m_Size);
						}
						// no change, still in ext storage
						else
						{
							move_fn(m_Storage.ext, res.data, m_Size);
							m_Allocator.deallocate(m_Storage.ext);
						}

						m_Storage.ext = res.data;
						m_Capacity	= (pointer)ralign_to((size_type)res.tail, sizeof(value_type)) - m_Storage.ext;
					}
				}
			}

			sbo_storage<T, SBO, AliasedSBO> m_Storage;
			size_type m_Size;
			size_type m_Capacity;
			allocator_type m_Allocator;
		};
	} // namespace details

	template <typename T, size_t SBO = psl::config::array_sbo_size<T>, typename Allocator = config::array_allocator<>>
	requires psl::traits::IsPhysicallyAllocated<Allocator> class array : public array_base<T, SBO, Allocator>
	{
		using is_trivially_destructible = std::is_trivially_destructible<std::remove_cv_t<T>>;
		using is_nothrow_destructible   = std::is_nothrow_destructible<std::remove_cv_t<T>>;

		using base_type = array_base<T, SBO, Allocator>;

	  public:
		using value_type	 = base_type::value_type;
		using reference		 = base_type::reference;
		using creference	 = base_type::creference;
		using pointer		 = base_type::pointer;
		using cpointer		 = base_type::cpointer;
		using size_type		 = base_type::size_type;
		using iterator		 = base_type::iterator;
		using const_iterator = base_type::const_iterator;

		using allocator_type = base_type::allocator_type;

	  private:
		static void copy_elements(cpointer source, pointer destination, size_type size)
		{
			if(source == destination) return;
			if constexpr(!std::is_trivial_v<value_type>)
			{

				for(auto src = source, end = src + size, dst = destination; src != end; ++src, ++dst)
				{
					new(dst) value_type(*src);
				}
			}
			else
			{
				std::memcpy(destination, source, size * sizeof(value_type));
			}
		}

		static void move_elements(pointer source, pointer destination, size_type size)
		{
			if(source == destination) return;
			if constexpr(!std::is_trivial_v<value_type>)
			{

				for(auto src = source, end = src + size, dst = destination; src != end; ++src, ++dst)
				{
					new(dst) value_type(std::move(*src));
					src->~value_type();
				}
			}
			else
			{
				std::memcpy(destination, source, size * sizeof(value_type));
			}
		}


	  public:
		constexpr array() noexcept = default;
		constexpr array(IsIntegral auto size) noexcept requires std::is_constructible_v<value_type> : base_type(size)
		{
			for(auto ptr = base_type::data(), end = ptr + base_type::m_Size; ptr != end; ++ptr) new(ptr) value_type();
		}
		constexpr array(IsIntegral auto size,
						const value_type& prototype) noexcept requires std::is_copy_constructible_v<value_type>
			: base_type(size)
		{
			for(auto ptr = base_type::data(), end = ptr + base_type::m_Size; ptr != end; ++ptr)
				new(ptr) value_type(prototype);
		}

		constexpr array(allocator_type& allocator) noexcept : base_type(allocator){};
		~array() noexcept(is_nothrow_destructible::value) { clear(); }

		template <typename It>
		constexpr array(It&& begin, It&& end) noexcept : base_type(std::distance(begin, end))
		{
			for(auto it = begin; it != end; ++it)
			{
				emplace_back(*it);
			}
		}

		template <typename... Ts>
		constexpr array(Ts&&... values) noexcept : base_type(sizeof...(Ts))
		{
			(emplace_back(std::forward<Ts>(values)), ...);
		}

		template <typename Y>
		requires requires(Y y)
		{
			y.begin();
			y.end();
		}
		constexpr array(Y&& values) noexcept : array(values.begin(), values.end()) {}

		constexpr array(const array& other) : base_type(other.m_Allocator)
		{
			reserve(other.capacity());
			for(const auto& element : other) emplace_back(element);
		}

		constexpr array(array&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>)
			: base_type(other.m_Allocator)
		{
			if(other.is_stored_inlined())
			{
				move_elements(other.data(), base_type::data(), other.size());
			}
			else
			{
				base_type::m_Storage.ext = other.m_Storage.ext;
				base_type::m_Capacity	= other.m_Capacity;
			}

			base_type::m_Size = other.m_Size;
			other.reset_to_capacity();
		}
		void clear() noexcept(is_nothrow_destructible::value)
		{
			if constexpr(!is_trivially_destructible::value)
			{
				for(auto ptr = base_type::data(), end = ptr + base_type::m_Size; ptr != end; ++ptr)
				{
					ptr->~value_type();
				}
			}
			base_type::m_Size = 0;
		}

		void shrink_to_fit() noexcept
		{
			if(base_type::m_Size == 0 && base_type::m_Capacity != SBO)
			{
				base_type::deallocate();
			}
			else
			{
				resize(base_type::m_Size);
			}
		}

		constexpr array& operator=(const array& other)
		{
			if(this == &other) return *this;
			clear();
			reserve(other.size());

			copy_elements(other.data(), base_type::data(), other.size());
			base_type::m_Size = other.size();
			return *this;
		}

		constexpr array& operator=(array&& other)
		{
			if(this == &other) return *this;
			clear();
			base_type::deallocate();

			if(other.is_stored_inlined())
			{
				move_elements(other.data(), base_type::data(), other.size());
			}
			else
			{
				base_type::m_Storage.ext = other.m_Storage.ext;
				base_type::m_Capacity	= other.m_Capacity;
			}

			base_type::m_Size = other.m_Size;
			other.reset_to_capacity();

			return *this;
		}

		constexpr reference operator[](size_type index) noexcept(!psl::config::exceptions)
		{
			PSL_EXCEPT_IF(index >= base_type::size(), "tried to access array with out of range index",
						  std::out_of_range);
			return *(base_type::data() + index);
		}

		constexpr creference operator[](size_type index) const noexcept(!psl::config::exceptions)
		{
			PSL_EXCEPT_IF(index >= base_type::size(), "tried to access array with out of range index",
						  std::out_of_range);
			return *(base_type::data() + index);
		}

		constexpr reference at(size_type index) noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(index >= base_type::size(), "tried to access array with out of range index",
						  std::out_of_range);
			return *(base_type::data() + index);
		}

		constexpr creference at(size_type index) const noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(index >= base_type::size(), "tried to access array with out of range index",
						  std::out_of_range);
			return *(base_type::data() + index);
		}

		constexpr reference front() noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(0 >= base_type::size(), "tried to access array with out of range index", std::out_of_range);
			return *base_type::data();
		}

		constexpr creference front() const noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(0 >= base_type::size(), "tried to access array with out of range index", std::out_of_range);
			return *base_type::data();
		}

		constexpr reference back() noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(0 >= base_type::size(), "tried to access array with out of range index", std::out_of_range);
			return *(base_type::data() + base_type::size() - 1);
		}

		constexpr creference back() const noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(0 >= base_type::size(), "tried to access array with out of range index", std::out_of_range);
			return *(base_type::data() + base_type::size() - 1);
		}

		constexpr auto begin() noexcept { return iterator{base_type::data()}; }
		constexpr auto end() noexcept { return iterator{base_type::data() + base_type::size()}; }
		constexpr auto begin() const noexcept { return const_iterator{(pointer)base_type::data()}; }
		constexpr auto end() const noexcept { return const_iterator{(pointer)base_type::data() + base_type::size()}; }
		constexpr auto cbegin() const noexcept { return const_iterator{(pointer)base_type::data()}; }
		constexpr auto cend() const noexcept { return const_iterator{(pointer)base_type::data() + base_type::size()}; }

		constexpr bool full() const noexcept { return base_type::size() == base_type::capacity(); }
		constexpr bool empty() const noexcept { return base_type::size() == 0; }

		void resize(size_type size) requires std::is_constructible_v<value_type>
		{
			base_type::reallocate(size, [newSize = size](pointer oldData, pointer newData, size_type oldSize) {
				auto size = std::min(oldSize, newSize);
				if(oldData != newData)
				{
					// copy over all items that are migrating to the new memory region
					for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst)
					{
						new(dst) value_type(std::move(*src));
						src->~value_type();
					}
				}

				// run the remaining destructors in the old memory region
				for(auto i = size; i < oldSize; ++i) (oldData + i)->~value_type();
			});


			// run the remaining constructors in the new memory region
			pointer ptr = base_type::data();
			for(auto i = base_type::m_Size; i < size; ++i) new(ptr + i) value_type();

			base_type::m_Size = size;
		}

		void resize(tags::alloc_only_t, size_type size) { resize(construction_traits_t::minimal, size); }

		void resize(construction_traits_t traits, size_type size)
		{
			if(size == base_type::m_Size) return;

			if(!(traits && construction_traits_t::destruct)) base_type::m_Size = std::min(size, base_type::m_Size);

			base_type::reallocate(size, [newSize = size](pointer oldData, pointer newData, size_type oldSize) {
				auto size = std::min(oldSize, newSize);
				if(oldData != newData)
				{
					// copy over all items that are migrating to the new memory region
					for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst)
					{
						new(dst) value_type(std::move(*src));
						src->~value_type();
					}
				}

				// run the remaining destructors in the old memory region
				for(auto i = size; i < oldSize; ++i) (oldData + i)->~value_type();
			});

			// run the remaining constructors in the new memory region
			pointer ptr = base_type::data();
			for(auto i = base_type::m_Size; i < size; ++i) new(ptr + i) value_type();

			base_type::m_Size = size;
		}

		void resize(size_type size, const value_type& prototype) requires std::is_copy_constructible_v<value_type>
		{
			base_type::reallocate(
				size, [newSize = size, &prototype](pointer oldData, pointer newData, size_type oldSize) {
					auto size = std::min(oldSize, newSize);
					if(oldData != newData)
					{
						// copy over all items that are migrating to the new memory region
						for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst)
						{
							new(dst) value_type(std::move(*src));
							src->~value_type();
						}
					}

					// run the remaining destructors in the old memory region
					for(auto i = size; i < oldSize; ++i) (oldData + i)->~value_type();
				});

			// run the remaining constructors in the new memory region
			pointer ptr = base_type::data();
			for(auto i = base_type::m_Size; i < size; ++i) new(ptr + i) value_type(prototype);

			base_type::m_Size = size;
		}

		size_type reserve(size_type size)
		{
			if(size <= base_type::capacity()) return size;

			base_type::reallocate(size, [](pointer oldData, pointer newData, size_type size) {
				if(oldData == newData) return;
				if constexpr(!std::is_trivial_v<value_type>)
				{

					for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst)
					{
						new(dst) value_type(std::move(*src));
						src->~value_type();
					}
				}
				else
				{
					std::memcpy(newData, oldData, size * sizeof(value_type));
				}
			});
			return base_type::capacity();
		}
		template <typename... Args>
		reference emplace(iterator it, Args&&... args) noexcept(
			std::is_nothrow_constructible_v<value_type, Args...>) requires std::is_constructible_v<value_type, Args...>
		{
			pointer it_loc  = &*it;
			auto index		= std::distance(this->begin(), it);
			const auto size = base_type::size();
			if(size == base_type::capacity())
			{
				base_type::reallocate(array_growth_for(size), [it_loc](pointer oldData, pointer newData,
																	   size_type size) {
					if(oldData == newData)
					{
						new(oldData + size) value_type();
						size += 1;
						std::rotate(it_loc, it_loc - 1, oldData + size);
						return;
					}
					auto first_size  = it_loc - oldData;
					auto second_size = size - first_size;

					if constexpr(std::is_trivially_copyable_v<value_type>)
					{
						std::memcpy(newData, oldData, first_size * sizeof(value_type));
						std::memcpy(newData + first_size + 1, oldData + first_size, second_size * sizeof(value_type));
					}
					else
					{
						for(auto src = oldData, end = src + first_size, dst = newData; src != end; ++src, ++dst)
						{
							if constexpr(std::is_move_constructible_v<value_type>)
								new(dst) value_type(std::move(*src));
							else
								new(dst) value_type(*src);

							if constexpr(!std::is_trivially_destructible_v<value_type>) src->~value_type();
						}

						for(auto src = oldData + first_size + 1, end = src + second_size, dst = newData + first_size;
							src != end; ++src, ++dst)
						{
							if constexpr(std::is_move_constructible_v<value_type>)
								new(dst) value_type(std::move(*src));
							else
								new(dst) value_type(*src);

							if constexpr(!std::is_trivially_destructible_v<value_type>) src->~value_type();
						}
					}
				});
			}
			auto ptr = base_type::data() + index;
			new(ptr) value_type(std::forward<Args>(args)...);
			base_type::m_Size += 1;
			return *ptr;
		}

		template <typename... Args>
		reference emplace(size_type index, Args&&... args) noexcept(
			std::is_nothrow_constructible_v<value_type, Args...>) requires std::is_constructible_v<value_type, Args...>
		{
			return emplace(std::next(this->begin(), index), std::forward<Args>(args)...);
		}

		template <typename... Args>
		reference emplace_back(Args&&... args) noexcept(
			std::is_nothrow_constructible_v<value_type, Args...>) requires std::is_constructible_v<value_type, Args...>
		{
			const auto size = base_type::size();
			if(size == base_type::capacity())
			{
				base_type::reallocate(array_growth_for(size), [](pointer oldData, pointer newData, size_type size) {
					if(oldData == newData) return;
					if constexpr(std::is_trivially_copyable_v<value_type>)
					{
						std::memcpy(newData, oldData, size * sizeof(value_type));
					}
					else
					{
						for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst)
						{
							if constexpr(std::is_move_constructible_v<value_type>)
								new(dst) value_type(std::move(*src));
							else
								new(dst) value_type(*src);

							if constexpr(!std::is_trivially_destructible_v<value_type>) src->~value_type();
						}
					}
				});
			}
			auto ptr = base_type::data() + size;
			new(ptr) value_type(std::forward<Args>(args)...);
			base_type::m_Size += 1;
			return *ptr;
		}

		constexpr void swap(array& other) noexcept
		{
			std::swap(base_type::m_Storage, other.m_Storage);
			std::swap(base_type::m_Size, other.m_Size);
			std::swap(base_type::m_Capacity, other.m_Capacity);
			std::swap(base_type::m_Allocator, other.m_Allocator);
		}


		constexpr void erase(const size_type first,
							 const size_type count = 1) noexcept(!config::exceptions && is_nothrow_destructible::value)
		{
			PSL_EXCEPT_IF(first > base_type::size(), "the first element was beyond the end", std::range_error);
			PSL_EXCEPT_IF(first + count > base_type::size(), "the last element was beyond the end", std::range_error);

			if(count == 0) return;
			auto it = begin() + first;
			std::rotate(it, it + count, end());
			base_type::m_Size -= count;
			if constexpr(!is_trivially_destructible::value)
			{
				for(auto ptr = base_type::data() + base_type::m_Size, end = ptr + count; ptr != end; ++ptr)
					ptr->~value_type();
			}
		}
		constexpr void erase(iterator first) noexcept(!config::exceptions && is_nothrow_destructible::value)
		{
			return erase(first, first + 1);
		}
		constexpr void erase(iterator first,
							 iterator last) noexcept(!config::exceptions && is_nothrow_destructible::value)
		{
			PSL_EXCEPT_IF(last < first, "the given range is negative (last is earlier than first)", std::range_error);
			PSL_EXCEPT_IF(first > end(), "the first element was beyond the end", std::range_error);
			PSL_EXCEPT_IF(last > end(), "the last element was beyond the end", std::range_error);

			auto distance = std::distance(first, last);
			if(distance == 0) return;

			std::rotate(first, last, end());
			base_type::m_Size -= distance;
			if constexpr(!is_trivially_destructible::value)
			{
				for(auto it = first; it != last; ++it) it->~value_type();
			}
		}
	};
} // namespace psl