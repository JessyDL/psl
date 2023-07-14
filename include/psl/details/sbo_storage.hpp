#pragma once
#include <array>
#include <psl/types.hpp>
#include <psl/iterators.hpp>
#include <psl/algorithms.hpp>
#include <psl/enum.hpp>

namespace psl
{
	template <bool Value>
	struct sbo_alias
	{
		static inline constexpr bool value = Value;
	};

	template <bool Value>
	inline constexpr auto sbo_alias_v = sbo_alias<Value>::value;

	template <typename T>
	concept IsSBOAlias = std::is_same_v<T, sbo_alias<true>> || std::is_same_v<T, sbo_alias<false>>;

	template <typename T>
	concept SBOAlias = std::is_same_v<T, sbo_alias<true>>;

	/**
	 * \brief container type that can store aliased types correctly for sbo_storage
	 *
	 */
	template <typename T, size_t N>
	struct aligned_storage_static_array
	{
		constexpr auto& operator[](size_t index) noexcept { return *std::launder((T*)&impl[index * sizeof(T)]); }
		constexpr const auto& operator[](size_t index) const noexcept
		{
			return *std::launder((T*)&impl[index * sizeof(T)]);
		}

		constexpr T* data() noexcept { return std::launder((T*)&impl[0]); }
		constexpr const T* data() const noexcept { return std::launder((const T*)&impl[0]); }

		constexpr T* data(size_t index) noexcept { return std::launder((T*)&impl[index * sizeof(T)]); }
		constexpr const T* data(size_t index) const noexcept
		{
			return std::launder((const T*)&impl[index * sizeof(T)]);
		}

	  private:
		std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, N> impl;
	};

	template <typename T>
	struct aligned_storage_static_array<T, 0>
	{
		constexpr T* data() noexcept { return nullptr; }
		constexpr const T* data() const noexcept { return nullptr; }

		constexpr T* data(size_t index) noexcept { return nullptr; }
		constexpr const T* data(size_t index) const noexcept { return nullptr; }
	};

	namespace _priv
	{
		/**
		 * \brief small buffer optimization primitive, only handles the declaration, correct usage is left to the
		 * user.
		 *
		 * \details facilitates in writing small buffer optimization enabled classes. This primitive provides a safe
		 * route to having non-trivial types stored in a sbo_storage.
		 *
		 * \warning It's up to the user to correctly construct/destruct and keep track of objects stored in the
		 * union's 'local' member. This object does not guarantee safety, it requires the user to be mindful of the
		 * requirements.
		 *
		 * \tparam T value type to store in the sbo
		 * \tparam N amount of elements to store in the sbo
		 * \tparam Alias changes the internal structure to be aliased when true.
		 */
		template <typename T, size_t N, IsSBOAlias Alias = sbo_alias<true>>
		struct sbo_storage
		{
			inline constexpr static size_t SBO = N;
			constexpr size_t size() const noexcept { return N; }
			union
			{
				T* ext;
				aligned_storage_static_array<T, N> local;
			};
		};

		template <typename T>
		struct sbo_storage<T, 0, sbo_alias<true>>
		{
			inline constexpr static size_t SBO = 0;
			constexpr sbo_storage() noexcept {}
			T* ext{nullptr};
		};

		template <typename T, size_t N>
		struct sbo_storage<T, N, sbo_alias<false>>
		{
			inline constexpr static size_t SBO = N;
			constexpr size_t size() const noexcept { return ((N * sizeof(T)) - sizeof(T*)) / sizeof(T); }

			constexpr sbo_storage() noexcept { ext = local.data(); }

			T* ext{nullptr};
			aligned_storage_static_array<T, N> local{};
		};

		template <typename T>
		struct sbo_storage<T, 0, sbo_alias<false>>
		{
			inline constexpr static size_t SBO = 0;
			constexpr sbo_storage() noexcept {}
			T* ext{nullptr};
		};

		template <size_t N0, size_t N1>
		struct sbo_storage_calculate
		{
			inline constexpr static size_t value{(N1 > N0) ? 0 : N0 - N1};
		};

	} // namespace _priv


	/**
	 * \copydoc _priv::sbo_storage
	 */
	template <typename T, size_t N, IsSBOAlias Alias = sbo_alias<true>>
	using sbo_storage = _priv::sbo_storage<
		T, (SBOAlias<Alias>) ? N : _priv::sbo_storage_calculate<N, align_to(sizeof(T*), sizeof(T)) / sizeof(T)>::value,
		Alias>;

	/**
	 * \brief Uses sbo_storage when available, otherwise will default to using an allocator.
	 *
	 * \tparam T element type to store
	 * \tparam SBO size of the SBO (in element count)
	 * \tparam Allocator Allocator to use as fallback
	 * \tparam Alias should the SBO alias or not? read psl::details::sbo_storage for more info.
	 * \warning This type does not run destructors for its contained elements, it's up to whoever uses it to make
	 * sure the memory is properly cleared before deallocation happens.
	 */
	template <typename T, size_t SBO_count, typename Allocator, IsSBOAlias Alias = sbo_alias<true>>
	struct dynamic_sbo_storage
	{
		using value_type = std::remove_cv_t<T>;
		using storage_t	 = sbo_storage<value_type, SBO_count, Alias>;
		static inline constexpr size_t SBO{storage_t::SBO};
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using size_type				 = size_t;
		using iterator				 = psl::contiguous_range_iterator<value_type>;
		using const_iterator		 = psl::contiguous_range_iterator<const value_type>;
		using reverse_iterator		 = psl::contiguous_range_iterator<value_type, -1>;
		using const_reverse_iterator = psl::contiguous_range_iterator<const value_type, -1>;

		using allocator_type = Allocator;

		constexpr dynamic_sbo_storage(const allocator_type& allocator) noexcept(!config::exceptions)
			: dynamic_sbo_storage(size_type{0}, allocator)
		{}

		constexpr dynamic_sbo_storage(
			size_type size = 0u, const allocator_type& allocator = psl::default_allocator) noexcept(!config::exceptions)
			: m_Size(size), m_Allocator(allocator)
		{
			if constexpr(SBO == 0)
				m_Storage.ext = nullptr;
			else if constexpr(!SBOAlias<Alias>)
				m_Storage.ext = m_Storage.local.data();

			if(size <= SBO)
			{
				m_Capacity = SBO;
			}
			else
			{
				auto res = m_Allocator.template allocate_n<value_type>(size);
				PSL_EXCEPT_IF(!res, std::runtime_error, "could not allocate");

				m_Storage.ext = res.data;
				m_Capacity	  = (pointer)ralign_to((size_type)res.tail, sizeof(value_type)) - m_Storage.ext;
			}
		}

		constexpr ~dynamic_sbo_storage()
		{
			if(!is_stored_inlined())
			{
				m_Allocator.deallocate(m_Storage.ext);
			}
		}

		/**
		 * \returns if the used storage is on the SBO (true) or using the allocator (false)
		 */
		constexpr bool is_stored_inlined() const noexcept;
		constexpr size_type size() const noexcept { return m_Size; }
		constexpr size_type capacity() const noexcept { return m_Capacity; }
		constexpr auto data() noexcept -> pointer;
		constexpr auto data() const noexcept -> const_pointer;
		constexpr auto cdata() const noexcept -> const_pointer;

		void reset_to_capacity() noexcept
		{
			if(!is_stored_inlined())
			{
				m_Storage.ext = nullptr;
				if constexpr(!SBOAlias<Alias>) m_Storage.ext = m_Storage.local.data();
			}
			m_Capacity = SBO;
			m_Size	   = 0;
		}

		void deallocate()
		{
			if(!is_stored_inlined())
			{
				m_Allocator.deallocate(m_Storage.ext);
			}
			m_Capacity = SBO;
			m_Size	   = 0;
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
					m_Capacity	  = 0;
					return;
				}
				auto res = m_Allocator.template allocate_n<value_type>(size);
				PSL_EXCEPT_IF(!res, std::runtime_error, "could not allocate");

				move_fn(m_Storage.ext, res.data, m_Size);
				m_Allocator.deallocate(m_Storage.ext);

				m_Storage.ext = res.data;
				m_Capacity	  = (pointer)ralign_to((size_type)res.tail, sizeof(value_type)) - m_Storage.ext;
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
						if constexpr(!SBOAlias<Alias>) m_Storage.ext = m_Storage.local.data();
					}
				}
				else
				{
					auto res = m_Allocator.template allocate_n<value_type>(size);
					PSL_EXCEPT_IF(!res, std::runtime_error, "could not allocate");

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
					m_Capacity	  = (pointer)ralign_to((size_type)res.tail, sizeof(value_type)) - m_Storage.ext;
				}
			}
		}

		constexpr auto begin() noexcept { return iterator{data()}; }
		constexpr auto end() noexcept { return iterator{data() + m_Size}; }
		constexpr auto begin() const noexcept { return const_iterator{data()}; }
		constexpr auto end() const noexcept { return const_iterator{data() + m_Size}; }
		constexpr auto cbegin() const noexcept { return const_iterator{data()}; }
		constexpr auto cend() const noexcept { return const_iterator{data() + m_Size}; }

		constexpr auto rbegin() noexcept { return reverse_iterator{data() + m_Size}; }
		constexpr auto rend() noexcept { return reverse_iterator{data()}; }
		constexpr auto rbegin() const noexcept { return const_reverse_iterator{data() + m_Size}; }
		constexpr auto rend() const noexcept { return const_reverse_iterator{data()}; }
		constexpr auto crbegin() const noexcept { return const_reverse_iterator{data() + m_Size}; }
		constexpr auto crend() const noexcept { return const_reverse_iterator{data()}; }

		constexpr reference operator[](size_type index) noexcept { return *(data() + index); }
		constexpr const_reference operator[](size_type index) const noexcept { return *(data() + index); }
		constexpr size_type sbo_size() const noexcept { return SBO; }

		storage_t m_Storage;
		size_type m_Size{0};
		size_type m_Capacity{0};
		Allocator m_Allocator;
	};


	template <typename T, size_t SBO_count, typename Allocator, IsSBOAlias Alias>
	constexpr bool dynamic_sbo_storage<T, SBO_count, Allocator, Alias>::is_stored_inlined() const noexcept
	{
		if constexpr(SBO == 0)
			return false;
		else
			return SBO == m_Capacity;
	}

	template <typename T, size_t SBO_count, typename Allocator, IsSBOAlias Alias>
	constexpr auto dynamic_sbo_storage<T, SBO_count, Allocator, Alias>::data() noexcept -> pointer
	{
		if constexpr(SBO == 0 || !SBOAlias<Alias>)
			return m_Storage.ext;
		else
		{
			if(is_stored_inlined()) [[unlikely]]
				return m_Storage.local.data();
			else [[likely]]
				return m_Storage.ext;
		}
	}


	template <typename T, size_t SBO_count, typename Allocator, IsSBOAlias Alias>
	constexpr auto dynamic_sbo_storage<T, SBO_count, Allocator, Alias>::data() const noexcept -> const_pointer
	{
		if constexpr(SBO == 0 || !SBOAlias<Alias>)
			return m_Storage.ext;
		else
		{
			if(is_stored_inlined()) [[unlikely]]
				return m_Storage.local.data();
			else [[likely]]
				return m_Storage.ext;
		}
	}

	template <typename T, size_t SBO_count, typename Allocator, IsSBOAlias Alias>
	constexpr auto dynamic_sbo_storage<T, SBO_count, Allocator, Alias>::cdata() const noexcept -> const_pointer
	{
		if constexpr(SBO == 0 || !SBOAlias<Alias>)
			return m_Storage.ext;
		else
		{
			if(is_stored_inlined()) [[unlikely]]
				return m_Storage.local.data();
			else [[likely]]
				return m_Storage.ext;
		}
	}
} // namespace psl
