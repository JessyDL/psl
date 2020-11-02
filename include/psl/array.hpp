#pragma once

#include <type_traits>
#include <psl/exceptions.hpp>
#include <psl/types.hpp>
#include <psl/allocator_traits.hpp>
#include <psl/config.hpp>
#include <psl/allocator.hpp>
#include <psl/details/sbo_storage.hpp>

#pragma region definition

namespace psl
{
	/**
	 * \brief Keeps relative ordering elements stable during the operation.
	 * \note For example when used in `array.erase()` it will keep the relative order of elements.
	 *
	 */
	struct keep_stability_t : _priv::id_token<keep_stability_t>
	{
		explicit constexpr keep_stability_t(identifier) noexcept {}
	};
	/**  \copydoc keep_stability_t */
	inline constexpr keep_stability_t keep_stability{keep_stability_t::identifier::token};

	/**
	 * \brief Allows elements to be re-ordered to minimize operations.
	 * \note For example when used in `array.erase()` it will move the last elements to fill the created hole, rather
	 * than move the entire sequence from the erasion.
	 *
	 */
	struct allow_instability_t : _priv::id_token<allow_instability_t>
	{
		explicit constexpr allow_instability_t(identifier) noexcept {}
	};
	/**  \copydoc allow_instability_t */
	inline constexpr allow_instability_t allow_instability{allow_instability_t::identifier::token};

	inline constexpr size_t default_sbo		 = -1;
	inline constexpr size_t default_sbo_size = 64;

	/**
	 * \brief Encapsulates some of the more esoteric settings for array instances.
	 *
	 * \tparam Allocator
	 * \tparam Stability Either keep_stability_t or allow_instability_t (default).
	 * \tparam SBOExtent SBO max size (in bytes)
	 * \tparam SBOAlias Should the SBO alias its internal storage with external storage (pointer)?
	 */
	template <typename Allocator, typename Stability, size_t SBOExtent = default_sbo,
			  IsSBOAlias SBOAlias = sbo_alias<false>>
	struct array_settings_t
	{
		static_assert(std::is_same_v<Stability, keep_stability_t> || std::is_same_v<Stability, allow_instability_t>,
					  "Stability has to be of the type keep_stability_t or allow_instability_t");
		using stability_type = Stability;
		using allocator_type = Allocator;
		using sbo_alias		 = SBOAlias;
		static inline constexpr size_t sbo_extent{SBOExtent};
		static inline constexpr bool keep_stable{std::is_same_v<Stability, keep_stability_t>};
	};


	/**
	 * \brief Contiguous storage container type
	 * \details This container specializes in storing items in contiguous memory. Optionally a max extent can be
	 * provided, at which case it will work like an `std::array` if no Allocator is provided, otherwise it will work
	 * like a `psl:array` that cannot grow beyond Extent.
	 * \warning iteraror validity, and data ordering is different than `std::vector`. Some operations will change the
	 * ordering of the data (like erase).
	 *
	 * \tparam T type to store
	 * \tparam Extent max extent of the storage device. defaults to `psl::dynamic_extent`
	 * \tparam Allocator allocator to use internally.
	 */
	template <typename T, size_t Extent = dynamic_extent,
			  typename Settings = array_settings_t<config::default_allocator_t, allow_instability_t>>
	class array
	{
	  public:
		/**
		 * \brief Exception type for when an array index is accessed that falls outside the range of actual constructed
		 * elements.
		 *
		 */
		using out_of_bounds			 = bad_access<array, "accessed the array outside of the valid range">;
		using value_type			 = T;
		using allocator_type		 = Settings::allocator_type;
		using size_type				 = size_t;
		using difference_type		 = std::ptrdiff_t;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using pointer				 = T*;
		using const_pointer			 = T* const;
		using iterator				 = psl::contiguous_range_iterator<value_type>;
		using const_iterator		 = psl::contiguous_range_iterator<const value_type>;
		using reverse_iterator		 = psl::contiguous_range_iterator<value_type, -1>;
		using const_reverse_iterator = psl::contiguous_range_iterator<const value_type, -1>;

		constexpr reference operator[](size_type index) noexcept { return m_Storage[index]; }
		constexpr const_reference operator[](size_type index) const noexcept { return m_Storage[index]; }

		constexpr bool is_stored_inlined() const noexcept { return m_Storage.is_stored_inlined(); }
		constexpr bool empty() const noexcept { return m_Storage.size() == 0; }
		constexpr size_type size() const noexcept { return m_Storage.size(); }
		constexpr size_type max_size() const noexcept { return Extent; }
		constexpr size_type capacity() const noexcept { return m_Storage.capacity(); }
		constexpr void reserve(size_type count) noexcept(false);
		constexpr void shrink_to_fit();
		constexpr void trim_excess();

		constexpr void clear();
		// constexpr iterator insert(const_iterator pos, const T& value);
		// constexpr iterator insert(const_iterator pos, T&& value);
		// constexpr iterator insert( const_iterator pos, size_type count,
		//               const T& value );
		// template< class InputIt >
		// constexpr iterator insert(const_iterator pos, InputIt first, InputIt last );

		// template< class... Args >
		// constexpr iterator emplace( const_iterator pos, Args&&... args );

		constexpr iterator erase(const_iterator pos);
		constexpr iterator erase(const_iterator first, const_iterator last);
		constexpr iterator erase(keep_stability_t, const_iterator pos);
		constexpr iterator erase(keep_stability_t, const_iterator first, const_iterator last);
		constexpr iterator erase(allow_instability_t, const_iterator pos);
		constexpr iterator erase(allow_instability_t, const_iterator first, const_iterator last);
		constexpr void push_back(T&& value);
		constexpr void push_back(const T& value);

		template <typename... Args>
		constexpr auto emplace_back(Args&&... args) -> reference;

		constexpr void pop_back();
		constexpr void resize(size_type count) requires std::is_constructible_v<value_type>;
		constexpr void resize(size_type count, const value_type& value);
		constexpr void swap(array& other) noexcept(/* see below */ false);

		constexpr size_type sbo_size() const noexcept { return m_Storage.sbo_size(); }

		constexpr iterator begin() noexcept { return m_Storage.begin(); }
		constexpr const_iterator begin() const noexcept { return m_Storage.begin(); }
		constexpr const_iterator cbegin() const noexcept { return m_Storage.cbegin(); }
		constexpr iterator end() noexcept { return m_Storage.end(); }
		constexpr const_iterator end() const noexcept { return m_Storage.end(); }
		constexpr const_iterator cend() const noexcept { return m_Storage.cend(); }

		constexpr reverse_iterator rbegin() noexcept { return m_Storage.rbegin(); }
		constexpr const_reverse_iterator rbegin() const noexcept { return m_Storage.rbegin(); }
		constexpr const_reverse_iterator crbegin() const noexcept { return m_Storage.crbegin(); }
		constexpr reverse_iterator rend() noexcept { return m_Storage.rend(); }
		constexpr const_reverse_iterator rend() const noexcept { return m_Storage.rend(); }
		constexpr const_reverse_iterator crend() const noexcept { return m_Storage.crend(); }

		constexpr reference front() noexcept { return *m_Storage.data(); }
		constexpr const_reference front() const noexcept { return *m_Storage.data(); }
		constexpr reference back() noexcept { return *(m_Storage.data() + m_Storage.m_Size - 1); }
		constexpr const_reference back() const noexcept { return *(m_Storage.data() + m_Storage.m_Size - 1); }

		constexpr reference at(size_type index) noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(index > m_Storage.m_Size, out_of_bounds);
			return *(m_Storage.data() + m_Storage.m_Size);
		}
		constexpr const_reference at(size_type index) const noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(index > m_Storage.m_Size, out_of_bounds);
			return *(m_Storage.data() + m_Storage.m_Size);
		}

	  private:
		constexpr auto calculate_growth_for(size_type count) const noexcept -> size_type;
		constexpr void grow_if_necessary(size_type newElements = 1);
		dynamic_sbo_storage<T,
							greatest_contained_count(sizeof(value_type), (Settings::sbo_extent == default_sbo)
																			 ? default_sbo_size
																			 : Settings::sbo_extent * sizeof(T)),
							allocator_type, typename Settings::sbo_alias>
			m_Storage{};
	};
} // namespace psl

#pragma endregion definition

#pragma region implementation

namespace psl
{
	namespace _priv
	{
		template <typename T>
		struct is_array : std::false_type
		{};

		template <typename T, size_t Extent, typename Settings>
		struct is_array<array<T, Extent, Settings>> : std::true_type
		{};
	} // namespace _priv

	template <typename T>
	struct is_array : public _priv::is_array<std::remove_cvref_t<T>>
	{};

	template <typename T>
	concept IsArray = is_array<T>::value;

	template <typename T>
	inline constexpr auto is_array_v = is_array<T>::value;
} // namespace psl

template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::calculate_growth_for(size_type count) const noexcept -> size_type
{
	auto cap = (capacity() * 3 + 1) / 2;
	if(cap >= count) return cap;

	// todo: calculate most significant bit and set all after to 1?
	return count;
}

template <typename T, size_t Extent, typename Settings>
constexpr void psl::array<T, Extent, Settings>::resize(size_type count) requires std::is_constructible_v<value_type>
{
	m_Storage.reallocate(count, [newSize = count](pointer oldData, pointer newData, size_type oldSize) {
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
	pointer ptr = m_Storage.data();
	for(auto i = m_Storage.m_Size; i < count; ++i) new(ptr + i) value_type();

	m_Storage.m_Size = count;
}


template <typename T, size_t Extent, typename Settings>
constexpr void psl::array<T, Extent, Settings>::resize(size_type count, const value_type& value)
{
	m_Storage.reallocate(count, [newSize = count](pointer oldData, pointer newData, size_type oldSize) {
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
	pointer ptr = m_Storage.data();
	for(auto i = m_Storage.m_Size; i < count; ++i) new(ptr + i) value_type(value);

	m_Storage.m_Size = count;
}


template <typename T, size_t Extent, typename Settings>
constexpr void psl::array<T, Extent, Settings>::reserve(size_type count) noexcept(false)
{
	if(count <= capacity()) return;

	m_Storage.reallocate(count, [newSize = count](pointer oldData, pointer newData, size_type oldSize) {
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
}

template <typename T, size_t Extent, typename Settings>
constexpr void psl::array<T, Extent, Settings>::grow_if_necessary(size_type newElements)
{
	if(m_Storage.size() + newElements > m_Storage.capacity())
	{
		reserve(calculate_growth_for(m_Storage.size() + newElements));
	}
}

template <typename T, size_t Extent, typename Settings>
template <typename... Args>
constexpr auto psl::array<T, Extent, Settings>::emplace_back(Args&&... args) -> reference
{
	grow_if_necessary();
	pointer ptr = m_Storage.data() + m_Storage.size();
	new(ptr) value_type(std::forward<Args>(args)...);
	++m_Storage.m_Size;
	return *ptr;
}


template <typename T, size_t Extent, typename Settings>
constexpr void psl::array<T, Extent, Settings>::shrink_to_fit()
{
	if(size() != capacity()) resize(size());
}
template <typename T, size_t Extent, typename Settings>
constexpr void psl::array<T, Extent, Settings>::trim_excess()
{
	shrink_to_fit();
}

template <typename T, size_t Extent, typename Settings>
constexpr void psl::array<T, Extent, Settings>::clear()
{
	for(auto& value : m_Storage) value.~T();
	m_Storage.m_Size = 0;
}
template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(const_iterator pos) -> iterator
{
	return erase(allow_instability, pos);
}

template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(const_iterator first, const_iterator last) -> iterator
{
	return erase(allow_instability, first, last);
}

template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(allow_instability_t, const_iterator pos) -> iterator
{
	using std::move;
	PSL_EXCEPT_IF(pos >= cend() || pos < cbegin(), out_of_bounds);
	size_type index{(size_type)(pos - cbegin())};
	auto it = begin() + index;
	if(index + 1 != m_Storage.m_Size)
		*it = move(back());
	else
		it->~T();
	--m_Storage.m_Size;
	return it;
}

template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(allow_instability_t, const_iterator first, const_iterator last)
	-> iterator
{
	using std::move, std::next, std::prev, std::min;
	PSL_EXCEPT_IF(first > last || first > cend() || last > cend() || first < cbegin() || last < cbegin(),
				  out_of_bounds);

	size_type first_i{(size_type)(first - cbegin())};
	size_type last_i{(size_type)(last - cbegin())};
	auto count			= last_i - first_i;
	size_type remainder = min(m_Storage.m_Size - last_i, count);
	move(prev(end(), remainder), end(), begin() + first_i);
	for(auto it = next(begin(), first_i + remainder), end_it = next(begin(), last_i); it != end_it; it = next(it))
	{
		it->~T();
	}
	m_Storage.m_Size -= count;
	return begin() + first_i;
}


template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(keep_stability_t, const_iterator pos) -> iterator
{
	using std::move, std::next;
	PSL_EXCEPT_IF(pos >= cend() || pos < cbegin(), out_of_bounds);
	size_type index{(size_type)(pos - cbegin())};
	auto it = begin() + index;
	if(index + 1 != m_Storage.m_Size)
	{
		move(next(it), end(), it);
	}
	else
		it->~T();
	--m_Storage.m_Size;
	return it;
}

template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(keep_stability_t, const_iterator first, const_iterator last)
	-> iterator
{
	using std::move, std::next, std::prev, std::min;
	PSL_EXCEPT_IF(first > last || first > cend() || last > cend() || first < cbegin() || last < cbegin(),
				  out_of_bounds);

	size_type first_i{(size_type)(first - cbegin())};
	size_type last_i{(size_type)(last - cbegin())};
	auto count			= last_i - first_i;
	size_type remainder = min(m_Storage.m_Size - last_i, count);
	move(next(begin(), last_i), end(), begin() + first_i);
	for(auto it = next(begin(), first_i + remainder), end_it = next(begin(), last_i); it != end_it; it = next(it))
	{
		it->~T();
	}
	m_Storage.m_Size -= count;
	return begin() + first_i;
}


template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::push_back(T&& value) -> void
{
	emplace_back(std::move(value));
}


template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::push_back(const T& value) -> void
{
	emplace_back(value);
}

template <typename T, size_t Extent, typename Settings>
constexpr auto psl::array<T, Extent, Settings>::pop_back() -> void
{
	PSL_EXCEPT_IF(size() == 0, psl::exception, "tried erasing an element on an empty array");
	erase(end() - 1);
}

#pragma endregion implementation