#pragma once

#include <algorithm>
#include <type_traits>

#include <psl/allocator.hpp>
#include <psl/allocator_traits.hpp>
#include <psl/bytes.hpp>
#include <psl/config.hpp>
#include <psl/details/sbo_storage.hpp>
#include <psl/exceptions.hpp>
#include <psl/iterators.hpp>
#include <psl/types.hpp>

#pragma region definition

namespace psl {
/**
 * \brief Keeps relative ordering elements stable during the operation.
 * \note For example when used in `array.erase()` it will keep the relative order of elements.
 *
 */
struct keep_stability_t : _priv::id_token<keep_stability_t> {
	explicit constexpr keep_stability_t(identifier) noexcept {}
};
/**  \copydoc keep_stability_t */
inline constexpr keep_stability_t keep_stability {keep_stability_t::identifier::token};

/**
 * \brief Allows elements to be re-ordered to minimize operations.
 * \note For example when used in `array.erase()` it will move the last elements to fill the created hole, rather
 * than move the entire sequence from the erasion.
 *
 */
struct allow_instability_t : _priv::id_token<allow_instability_t> {
	explicit constexpr allow_instability_t(identifier) noexcept {}
};
/**  \copydoc allow_instability_t */
inline constexpr allow_instability_t allow_instability {allow_instability_t::identifier::token};


namespace _priv {
	template <typename T, psl::bytes_t Value>
	consteval size_t get_sbo_size() {
		return *Value;
	}
}	 // namespace _priv

namespace settings {
	inline constexpr auto default_sbo_size = psl::bytes_t {64};

	template <typename Setting, typename Default>
	struct override_or_default {
		using type = conditional_t<std::is_same_v<Setting, default_t>, Default, Setting>;
	};

	template <typename Setting, typename Default>
	using override_or_default_t = override_or_default<Setting, Default>::type;
	/**
	 * \brief Encapsulates some of the more esoteric settings for array instances.
	 *
	 * \tparam Allocator
	 * \tparam Stability Either keep_stability_t or allow_instability_t (default).
	 * \tparam SBOExtent SBO max size
	 * \tparam SBOAlias Should the SBO alias its internal storage with external storage (pointer)?
	 * \note if the arrays's extent is lower than
	 */
	template <typename Allocator	 = default_t,
			  typename Stability	 = default_t,
			  psl::bytes_t SBOExtent = default_sbo_size,
			  IsSBOAlias SBOAlias	 = sbo_alias<false>>
	struct array {
		using allocator_type = override_or_default_t<Allocator, config::default_allocator_t>;
		using stability_type = override_or_default_t<Stability, allow_instability_t>;

		template <typename T, size_t Extent>
		using sbo_alias = std::conditional_t<Extent <= _priv::get_sbo_size<T, SBOExtent>(), sbo_alias<true>, SBOAlias>;

		template <typename T, size_t Extent>
		inline constexpr static size_t sbo_extent =
		  (Extent <= _priv::get_sbo_size<T, SBOExtent>()) ? Extent : _priv::get_sbo_size<T, SBOExtent>();
	};
}	 // namespace settings

namespace _priv {
	template <typename T>
	struct is_array_settings_t : std::false_type {};
	template <typename Allocator, typename Stability, ::psl::bytes_t SBOExtent, IsSBOAlias SBOAlias>
	struct is_array_settings_t<settings::array<Allocator, Stability, SBOExtent, SBOAlias>> : std::true_type {};
}	 // namespace _priv

template <typename T>
concept IsArraySettings = _priv::is_array_settings_t<std::remove_cvref_t<T>>::value;

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
template <typename T, size_t Extent = dynamic_extent, IsArraySettings Settings = settings::array<>>
class array {
  public:
	/**
	 * \brief Exception type for when an array index is accessed that falls outside the range of actual constructed
	 * elements.
	 *
	 */
	using out_of_bounds			 = bad_access<array, "accessed the array outside of the valid range">;
	using overallocation		 = static_exception<"allocated beyond the max extent of the array">;
	using value_type			 = T;
	using allocator_type		 = Settings::allocator_type;
	using size_type				 = size_t;
	using difference_type		 = std::ptrdiff_t;
	using reference				 = value_type&;
	using const_reference		 = value_type const&;
	using pointer				 = T*;
	using const_pointer			 = T* const;
	using iterator				 = psl::contiguous_range_iterator<value_type>;
	using const_iterator		 = psl::contiguous_range_iterator<value_type const>;
	using reverse_iterator		 = psl::contiguous_range_iterator<value_type, -1>;
	using const_reverse_iterator = psl::contiguous_range_iterator<value_type const, -1>;

	constexpr auto operator[](size_type index) noexcept -> reference { return m_Storage[index]; }
	constexpr auto operator[](size_type index) const noexcept -> const_reference { return m_Storage[index]; }

	constexpr auto is_stored_inlined() const noexcept -> bool { return m_Storage.is_stored_inlined(); }
	constexpr auto empty() const noexcept -> bool { return m_Storage.size() == 0; }
	constexpr auto size() const noexcept -> size_type { return m_Storage.size(); }
	constexpr auto max_size() const noexcept -> size_type { return Extent; }
	constexpr auto capacity() const noexcept -> size_type { return m_Storage.capacity(); }
	constexpr auto reserve(size_type count) noexcept(false) -> void;
	constexpr auto shrink_to_fit() -> void;
	constexpr auto trim_excess() -> void;

	constexpr auto clear() -> void;
	// constexpr iterator insert(const_iterator pos, const T& value);
	// constexpr iterator insert(const_iterator pos, T&& value);
	// constexpr iterator insert( const_iterator pos, size_type count,
	//               const T& value );
	// template< class InputIt >
	// constexpr iterator insert(const_iterator pos, InputIt first, InputIt last );

	// template< class... Args >
	// constexpr iterator emplace( const_iterator pos, Args&&... args );

	/**
	 * @brief Erases (deletes) the given entry in the array
	 *
	 * @param pos Iterator marking the location of the entry
	 * @return constexpr iterator
	 */
	constexpr iterator erase(const_iterator pos);
	/**
	 * @brief Erases (deletes) the given entries in the array, the range is defined as [first, last)
	 *
	 * @param first first element to erase (inclusive)
	 * @param last last element to erase (exclusive)
	 * @return constexpr iterator
	 */
	constexpr auto erase(const_iterator first, const_iterator last) -> iterator;
	constexpr auto erase(keep_stability_t, const_iterator pos) -> iterator;
	constexpr auto erase(keep_stability_t, const_iterator first, const_iterator last) -> iterator;
	constexpr auto erase(allow_instability_t, const_iterator pos) -> iterator;
	constexpr auto erase(allow_instability_t, const_iterator first, const_iterator last) -> iterator;
	constexpr auto push_back(T&& value) -> void;
	constexpr auto push_back(T const& value) -> void;

	template <typename... Args>
	constexpr auto emplace_back(Args&&... args) -> reference;

	constexpr auto pop_back() -> void;
	constexpr auto resize(size_type count) -> void
		requires std::is_constructible_v<value_type>;
	constexpr auto resize(size_type count, value_type const& value) -> void;
	constexpr auto swap(array& other) noexcept(/* see below */ false) -> void;

	constexpr auto sbo_size() const noexcept -> size_type { return m_Storage.sbo_size(); }

	constexpr auto begin() noexcept -> iterator { return m_Storage.begin(); }
	constexpr auto begin() const noexcept -> const_iterator { return m_Storage.begin(); }
	constexpr auto cbegin() const noexcept -> const_iterator { return m_Storage.cbegin(); }
	constexpr auto end() noexcept -> iterator { return m_Storage.end(); }
	constexpr auto end() const noexcept -> const_iterator { return m_Storage.end(); }
	constexpr auto cend() const noexcept -> const_iterator { return m_Storage.cend(); }

	constexpr auto rbegin() noexcept -> reverse_iterator { return m_Storage.rbegin(); }
	constexpr auto rbegin() const noexcept -> const_reverse_iterator { return m_Storage.rbegin(); }
	constexpr auto crbegin() const noexcept -> const_reverse_iterator { return m_Storage.crbegin(); }
	constexpr auto rend() noexcept -> reverse_iterator { return m_Storage.rend(); }
	constexpr auto rend() const noexcept -> const_reverse_iterator { return m_Storage.rend(); }
	constexpr auto crend() const noexcept -> const_reverse_iterator { return m_Storage.crend(); }

	constexpr auto front() noexcept -> reference { return *m_Storage.data(); }
	constexpr auto front() const noexcept -> const_reference { return *m_Storage.data(); }
	constexpr auto back() noexcept -> reference { return *(m_Storage.data() + m_Storage.m_Size - 1); }
	constexpr auto back() const noexcept -> const_reference { return *(m_Storage.data() + m_Storage.m_Size - 1); }

	constexpr auto at(size_type index) noexcept(!config::exceptions) -> reference {
		PSL_EXCEPT_IF(index > m_Storage.m_Size, out_of_bounds);
		return *(m_Storage.data() + m_Storage.m_Size);
	}
	constexpr auto at(size_type index) const noexcept(!config::exceptions) -> const_reference {
		PSL_EXCEPT_IF(index > m_Storage.m_Size, out_of_bounds);
		return *(m_Storage.data() + m_Storage.m_Size);
	}

  private:
	constexpr auto calculate_growth_for(size_type count) const noexcept(!config::exceptions) -> size_type;
	constexpr auto grow_if_necessary(size_type newElements = 1) -> void;
	dynamic_sbo_storage<T,
						Settings::template sbo_extent<T, Extent>,
						allocator_type,
						typename Settings::template sbo_alias<T, Extent>>
	  m_Storage {};
};

template <typename T, IsArraySettings Settings>
class array<T, dynamic_extent, Settings> {
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
	using const_reference		 = value_type const&;
	using pointer				 = T*;
	using const_pointer			 = T* const;
	using iterator				 = psl::contiguous_range_iterator<value_type>;
	using const_iterator		 = psl::contiguous_range_iterator<value_type const>;
	using reverse_iterator		 = psl::contiguous_range_iterator<value_type, -1>;
	using const_reverse_iterator = psl::contiguous_range_iterator<value_type const, -1>;

	constexpr auto operator[](size_type index) noexcept -> reference { return m_Storage[index]; }
	constexpr auto operator[](size_type index) const noexcept -> const_reference { return m_Storage[index]; }

	constexpr auto is_stored_inlined() const noexcept -> bool { return m_Storage.is_stored_inlined(); }
	constexpr auto empty() const noexcept -> bool { return m_Storage.size() == 0; }
	constexpr auto size() const noexcept -> size_type { return m_Storage.size(); }
	constexpr auto capacity() const noexcept -> size_type { return m_Storage.capacity(); }
	constexpr auto max_size() const noexcept -> size_type { return dynamic_extent; }
	constexpr auto reserve(size_type count) noexcept(false) -> void;
	constexpr auto shrink_to_fit() -> void;
	constexpr auto trim_excess() -> void;

	constexpr void clear();
	// constexpr iterator insert(const_iterator pos, const T& value);
	// constexpr iterator insert(const_iterator pos, T&& value);
	// constexpr iterator insert( const_iterator pos, size_type count,
	//               const T& value );
	// template< class InputIt >
	// constexpr iterator insert(const_iterator pos, InputIt first, InputIt last );

	// template< class... Args >
	// constexpr iterator emplace( const_iterator pos, Args&&... args );

	constexpr auto erase(const_iterator pos) -> iterator;
	constexpr auto erase(const_iterator first, const_iterator last) -> iterator;
	constexpr auto erase(keep_stability_t, const_iterator pos) -> iterator;
	constexpr auto erase(keep_stability_t, const_iterator first, const_iterator last) -> iterator;
	constexpr auto erase(allow_instability_t, const_iterator pos) -> iterator;
	constexpr auto erase(allow_instability_t, const_iterator first, const_iterator last) -> iterator;
	constexpr auto push_back(T&& value) -> void;
	constexpr auto push_back(T const& value) -> void;

	template <typename... Args>
	constexpr auto emplace_back(Args&&... args) -> reference;

	constexpr auto pop_back() -> void;
	constexpr auto resize(size_type count) -> void
		requires std::is_constructible_v<value_type>;
	constexpr auto resize(size_type count, value_type const& value) -> void;
	constexpr auto swap(array& other) noexcept(/* see below */ false) -> void;

	constexpr auto sbo_size() const noexcept -> size_type { return m_Storage.sbo_size(); }

	constexpr auto begin() noexcept -> iterator { return m_Storage.begin(); }
	constexpr auto begin() const noexcept -> const_iterator { return m_Storage.begin(); }
	constexpr auto cbegin() const noexcept -> const_iterator { return m_Storage.cbegin(); }
	constexpr auto end() noexcept -> iterator { return m_Storage.end(); }
	constexpr auto end() const noexcept -> const_iterator { return m_Storage.end(); }
	constexpr auto cend() const noexcept -> const_iterator { return m_Storage.cend(); }

	constexpr auto rbegin() noexcept -> reverse_iterator { return m_Storage.rbegin(); }
	constexpr auto rbegin() const noexcept -> const_reverse_iterator { return m_Storage.rbegin(); }
	constexpr auto crbegin() const noexcept -> const_reverse_iterator { return m_Storage.crbegin(); }
	constexpr auto rend() noexcept -> reverse_iterator { return m_Storage.rend(); }
	constexpr auto rend() const noexcept -> const_reverse_iterator { return m_Storage.rend(); }
	constexpr auto crend() const noexcept -> const_reverse_iterator { return m_Storage.crend(); }

	constexpr auto front() noexcept -> reference { return *m_Storage.data(); }
	constexpr auto front() const noexcept -> const_reference { return *m_Storage.data(); }
	constexpr auto back() noexcept -> reference { return *(m_Storage.data() + m_Storage.m_Size - 1); }
	constexpr auto back() const noexcept -> const_reference { return *(m_Storage.data() + m_Storage.m_Size - 1); }

	constexpr auto at(size_type index) noexcept(!config::exceptions) -> reference {
		PSL_EXCEPT_IF(index > m_Storage.m_Size, out_of_bounds);
		return *(m_Storage.data() + m_Storage.m_Size);
	}
	constexpr auto at(size_type index) const noexcept(!config::exceptions) -> const_reference {
		PSL_EXCEPT_IF(index > m_Storage.m_Size, out_of_bounds);
		return *(m_Storage.data() + m_Storage.m_Size);
	}

  private:
	constexpr auto calculate_growth_for(size_type count) const noexcept -> size_type;
	constexpr auto grow_if_necessary(size_type newElements = 1) -> void;
	dynamic_sbo_storage<T,
						Settings::template sbo_extent<T, dynamic_extent>,
						allocator_type,
						typename Settings::template sbo_alias<T, dynamic_extent>>
	  m_Storage {};
};

#pragma endregion definition

#pragma region implementation

namespace _priv {
	template <typename T>
	struct is_array : std::false_type {};

	template <typename T, size_t Extent, IsArraySettings Settings>
	struct is_array<array<T, Extent, Settings>> : std::true_type {};

	template <typename T>
	struct is_static_array : std::false_type {};

	template <typename T, size_t Extent, IsArraySettings Settings>
	struct is_static_array<array<T, Extent, Settings>> : conditional_t<Extent != dynamic_extent> {};

	template <typename T>
	struct is_dynamic_array : std::false_type {};

	template <typename T, size_t Extent, IsArraySettings Settings>
	struct is_dynamic_array<array<T, Extent, Settings>> : conditional_t<Extent == dynamic_extent> {};
}	 // namespace _priv

template <typename T>
struct is_array : public _priv::is_array<std::remove_cvref_t<T>> {};

template <typename T>
concept IsArray = is_array<T>::value;

template <typename T>
inline constexpr auto is_array_v = is_array<T>::value;


template <IsArray T>
struct is_static_array : public _priv::is_static_array<std::remove_cvref_t<T>> {};

template <typename T>
concept IsStaticArray = IsArray<T> && is_static_array<T>::value;

template <IsArray T>
inline constexpr auto is_static_array_v = is_static_array<T>::value;

template <IsArray T>
struct is_dynamic_array : public _priv::is_dynamic_array<std::remove_cvref_t<T>> {};

template <typename T>
concept IsDynamicArray = IsArray<T> && is_dynamic_array<T>::value;

template <IsArray T>
inline constexpr auto is_dynamic_array_v = is_dynamic_array<T>::value;

template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::calculate_growth_for(size_type count) const
  noexcept(!psl::config::exceptions) -> size_type {
	PSL_EXCEPT_IF(capacity() == max_size(), overallocation);
	auto cap = std::min((capacity() * 3 + 1) / 2, max_size());
	if(cap >= count)
		return cap;

	// todo: calculate most significant bit and set all after to 1?
	PSL_EXCEPT_IF(count > max_size(), overallocation);
	return count;
}

template <typename T, size_t Extent, IsArraySettings Settings>
constexpr void psl::array<T, Extent, Settings>::resize(size_type count)
	requires std::is_constructible_v<value_type>
{
	PSL_EXCEPT_IF(count > max_size(), overallocation);
	m_Storage.reallocate(count, [newSize = count](pointer oldData, pointer newData, size_type oldSize) {
		auto size = std::min(oldSize, newSize);
		if(oldData != newData) {
			// copy over all items that are migrating to the new memory region
			for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst) {
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


template <typename T, size_t Extent, IsArraySettings Settings>
constexpr void psl::array<T, Extent, Settings>::resize(size_type count, value_type const& value) {
	PSL_EXCEPT_IF(count > max_size(), overallocation);
	m_Storage.reallocate(count, [newSize = count](pointer oldData, pointer newData, size_type oldSize) {
		auto size = std::min(oldSize, newSize);
		if(oldData != newData) {
			// copy over all items that are migrating to the new memory region
			for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst) {
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


template <typename T, size_t Extent, IsArraySettings Settings>
constexpr void psl::array<T, Extent, Settings>::reserve(size_type count) noexcept(false) {
	PSL_EXCEPT_IF(count > max_size(), overallocation);
	if(count <= capacity())
		return;

	m_Storage.reallocate(count, [newSize = count](pointer oldData, pointer newData, size_type oldSize) {
		auto size = std::min(oldSize, newSize);
		if(oldData != newData) {
			// copy over all items that are migrating to the new memory region
			for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst) {
				new(dst) value_type(std::move(*src));
				src->~value_type();
			}
		}

		// run the remaining destructors in the old memory region
		for(auto i = size; i < oldSize; ++i) (oldData + i)->~value_type();
	});
}

template <typename T, size_t Extent, IsArraySettings Settings>
constexpr void psl::array<T, Extent, Settings>::grow_if_necessary(size_type newElements) {
	if(m_Storage.size() + newElements > m_Storage.capacity()) {
		PSL_EXCEPT_IF(m_Storage.size() + newElements > max_size(), overallocation);
		reserve(calculate_growth_for(m_Storage.size() + newElements));
	}
}

template <typename T, size_t Extent, IsArraySettings Settings>
template <typename... Args>
constexpr auto psl::array<T, Extent, Settings>::emplace_back(Args&&... args) -> reference {
	grow_if_necessary();
	pointer ptr = m_Storage.data() + m_Storage.size();
	new(ptr) value_type(std::forward<Args>(args)...);
	++m_Storage.m_Size;
	return *ptr;
}


template <typename T, size_t Extent, IsArraySettings Settings>
constexpr void psl::array<T, Extent, Settings>::shrink_to_fit() {
	if(size() != capacity())
		resize(size());
}
template <typename T, size_t Extent, IsArraySettings Settings>
constexpr void psl::array<T, Extent, Settings>::trim_excess() {
	shrink_to_fit();
}

template <typename T, size_t Extent, IsArraySettings Settings>
constexpr void psl::array<T, Extent, Settings>::clear() {
	for(auto& value : m_Storage) value.~T();
	m_Storage.m_Size = 0;
}
template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(const_iterator pos) -> iterator {
	return erase(allow_instability, pos);
}

template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(const_iterator first, const_iterator last) -> iterator {
	return erase(allow_instability, first, last);
}

template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(allow_instability_t, const_iterator pos) -> iterator {
	using std::move;
	PSL_EXCEPT_IF(pos >= cend() || pos < cbegin(), out_of_bounds);
	size_type index {(size_type)(pos - cbegin())};
	auto it = begin() + index;
	if(index + 1 != m_Storage.m_Size)
		*it = move(back());
	else
		it->~T();
	--m_Storage.m_Size;
	return it;
}

template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(allow_instability_t, const_iterator first, const_iterator last)
  -> iterator {
	using std::move, std::next, std::prev, std::min;
	PSL_EXCEPT_IF(first > last || first > cend() || last > cend() || first < cbegin() || last < cbegin(),
				  out_of_bounds);

	size_type first_i {(size_type)(first - cbegin())};
	size_type last_i {(size_type)(last - cbegin())};
	auto count			= last_i - first_i;
	size_type remainder = min(m_Storage.m_Size - last_i, count);
	move(prev(end(), remainder), end(), begin() + first_i);
	for(auto it = next(begin(), first_i + remainder), end_it = next(begin(), last_i); it != end_it; it = next(it)) {
		it->~T();
	}
	m_Storage.m_Size -= count;
	return begin() + first_i;
}


template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(keep_stability_t, const_iterator pos) -> iterator {
	using std::move, std::next;
	PSL_EXCEPT_IF(pos >= cend() || pos < cbegin(), out_of_bounds);
	size_type index {(size_type)(pos - cbegin())};
	iterator it = next(begin(), index);
	if(index + 1 != m_Storage.m_Size) {
		move(next(it), end(), it);
	} else
		it->~T();
	--m_Storage.m_Size;
	return it;
}

template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::erase(keep_stability_t, const_iterator first, const_iterator last)
  -> iterator {
	using std::move, std::next, std::prev, std::min;
	PSL_EXCEPT_IF(first > last || first > cend() || last > cend() || first < cbegin() || last < cbegin(),
				  out_of_bounds);

	size_type first_i {(size_type)(first - cbegin())};
	size_type last_i {(size_type)(last - cbegin())};
	auto count			= last_i - first_i;
	size_type remainder = min(m_Storage.m_Size - last_i, count);
	move(next(begin(), last_i), end(), begin() + first_i);
	for(auto it = next(begin(), first_i + remainder), end_it = next(begin(), last_i); it != end_it; it = next(it)) {
		it->~T();
	}
	m_Storage.m_Size -= count;
	return begin() + first_i;
}


template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::push_back(T&& value) -> void {
	emplace_back(std::move(value));
}


template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::push_back(T const& value) -> void {
	emplace_back(value);
}

template <typename T, size_t Extent, IsArraySettings Settings>
constexpr auto psl::array<T, Extent, Settings>::pop_back() -> void {
	PSL_EXCEPT_IF(size() == 0, psl::exception, "tried erasing an element on an empty array");
	erase(end() - 1);
}

#pragma region dynamic_extent

template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::calculate_growth_for(size_type count) const noexcept
  -> size_type {
	auto cap = (capacity() * 3 + 1) / 2;
	if(cap >= count)
		return cap;

	// todo: calculate most significant bit and set all after to 1?
	return count;
}

template <typename T, IsArraySettings Settings>
constexpr void psl::array<T, psl::dynamic_extent, Settings>::resize(size_type count)
	requires std::is_constructible_v<value_type>
{
	m_Storage.reallocate(count, [newSize = count](pointer oldData, pointer newData, size_type oldSize) {
		auto size = std::min(oldSize, newSize);
		if(oldData != newData) {
			// copy over all items that are migrating to the new memory region
			for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst) {
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


template <typename T, IsArraySettings Settings>
constexpr void psl::array<T, psl::dynamic_extent, Settings>::resize(size_type count, value_type const& value) {
	m_Storage.reallocate(count, [newSize = count](pointer oldData, pointer newData, size_type oldSize) {
		auto size = std::min(oldSize, newSize);
		if(oldData != newData) {
			// copy over all items that are migrating to the new memory region
			for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst) {
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


template <typename T, IsArraySettings Settings>
constexpr void psl::array<T, psl::dynamic_extent, Settings>::reserve(size_type count) noexcept(false) {
	if(count <= capacity())
		return;

	m_Storage.reallocate(count, [newSize = count](pointer oldData, pointer newData, size_type oldSize) {
		auto size = std::min(oldSize, newSize);
		if(oldData != newData) {
			// copy over all items that are migrating to the new memory region
			for(auto src = oldData, end = src + size, dst = newData; src != end; ++src, ++dst) {
				new(dst) value_type(std::move(*src));
				src->~value_type();
			}
		}

		// run the remaining destructors in the old memory region
		for(auto i = size; i < oldSize; ++i) (oldData + i)->~value_type();
	});
}

template <typename T, IsArraySettings Settings>
constexpr void psl::array<T, psl::dynamic_extent, Settings>::grow_if_necessary(size_type newElements) {
	if(m_Storage.size() + newElements > m_Storage.capacity()) {
		reserve(calculate_growth_for(m_Storage.size() + newElements));
	}
}

template <typename T, IsArraySettings Settings>
template <typename... Args>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::emplace_back(Args&&... args) -> reference {
	grow_if_necessary();
	pointer ptr = m_Storage.data() + m_Storage.size();
	new(ptr) value_type(std::forward<Args>(args)...);
	++m_Storage.m_Size;
	return *ptr;
}


template <typename T, IsArraySettings Settings>
constexpr void psl::array<T, psl::dynamic_extent, Settings>::shrink_to_fit() {
	if(size() != capacity())
		resize(size());
}
template <typename T, IsArraySettings Settings>
constexpr void psl::array<T, psl::dynamic_extent, Settings>::trim_excess() {
	shrink_to_fit();
}

template <typename T, IsArraySettings Settings>
constexpr void psl::array<T, psl::dynamic_extent, Settings>::clear() {
	for(auto& value : m_Storage) value.~T();
	m_Storage.m_Size = 0;
}

template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::erase(const_iterator pos) -> iterator {
	return erase(allow_instability, pos);
}

template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::erase(const_iterator first, const_iterator last)
  -> iterator {
	return erase(allow_instability, first, last);
}

template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::erase(allow_instability_t, const_iterator pos)
  -> iterator {
	using std::move;
	PSL_EXCEPT_IF(pos >= cend() || pos < cbegin(), out_of_bounds);
	size_type index {(size_type)(pos - cbegin())};
	auto it = begin() + index;
	if(index + 1 != m_Storage.m_Size)
		*it = move(back());
	else
		it->~T();
	--m_Storage.m_Size;
	return it;
}

template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::erase(allow_instability_t,
																   const_iterator first,
																   const_iterator last) -> iterator {
	using std::move, std::next, std::prev, std::min;
	PSL_EXCEPT_IF(first > last || first > cend() || last > cend() || first < cbegin() || last < cbegin(),
				  out_of_bounds);

	size_type first_i {(size_type)(first - cbegin())};
	size_type last_i {(size_type)(last - cbegin())};
	auto count			= last_i - first_i;
	size_type remainder = min(m_Storage.m_Size - last_i, count);
	move(prev(end(), remainder), end(), begin() + first_i);
	for(auto it = next(begin(), first_i + remainder), end_it = next(begin(), last_i); it != end_it; it = next(it)) {
		it->~T();
	}
	m_Storage.m_Size -= count;
	return begin() + first_i;
}


template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::erase(keep_stability_t, const_iterator pos) -> iterator {
	using std::move, std::next;
	PSL_EXCEPT_IF(pos >= cend() || pos < cbegin(), out_of_bounds);
	size_type index {(size_type)(pos - cbegin())};
	auto it = begin() + index;
	if(index + 1 != m_Storage.m_Size) {
		move(next(it), end(), it);
	} else
		it->~T();
	--m_Storage.m_Size;
	return it;
}

template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::erase(keep_stability_t,
																   const_iterator first,
																   const_iterator last) -> iterator {
	using std::move, std::next, std::prev, std::min;
	PSL_EXCEPT_IF(first > last || first > cend() || last > cend() || first < cbegin() || last < cbegin(),
				  out_of_bounds);

	size_type first_i {(size_type)(first - cbegin())};
	size_type last_i {(size_type)(last - cbegin())};
	auto count			= last_i - first_i;
	size_type remainder = min(m_Storage.m_Size - last_i, count);
	move(next(begin(), last_i), end(), begin() + first_i);
	for(auto it = next(begin(), first_i + remainder), end_it = next(begin(), last_i); it != end_it; it = next(it)) {
		it->~T();
	}
	m_Storage.m_Size -= count;
	return begin() + first_i;
}


template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::push_back(T&& value) -> void {
	emplace_back(std::move(value));
}


template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::push_back(T const& value) -> void {
	emplace_back(value);
}

template <typename T, IsArraySettings Settings>
constexpr auto psl::array<T, psl::dynamic_extent, Settings>::pop_back() -> void {
	PSL_EXCEPT_IF(size() == 0, psl::exception, "tried erasing an element on an empty array");
	erase(end() - 1);
}
}	 // namespace psl
#pragma endregion dynamic_extent

#pragma endregion implementation