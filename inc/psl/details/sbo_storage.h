#pragma once
#include <array>	   // std::array
#include <type_traits> // std::aligned_storage

namespace psl
{
	inline namespace details
	{
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

		/**
		 * \brief small buffer optimization primitive, only handles the declaration, correct usage is left to the user.
		 *
		 * \details facilitates in writing small buffer optimization enabled classes. This primitive provides a safe
		 * route to having non-trivial types stored in a sbo_storage.
		 *
		 * \warning It's up to the user to correctly construct/destruct and keep track of objects stored in the union's
		 * 'local' member. This object does not guarantee safety, it requires the user to be mindful of the
		 * requirements.
		 *
		 * \tparam T value type to store in the sbo
		 * \tparam N amount of elements to store in the sbo
		 * \tparam Union changes the internal structure to be aliased when true.
		 */
		template <typename T, size_t N, bool Union = true>
		struct sbo_storage
		{
			union
			{
				T* ext;
				aligned_storage_static_array<T, N> local;
			};
		};

		template <typename T, size_t N>
		struct sbo_storage<T, N, false>
		{
			constexpr sbo_storage() noexcept : ext(local.data()) {}

			T* ext{nullptr};
			aligned_storage_static_array<T, N> local;
		};

		template <typename T>
		struct sbo_storage<T, 0, false>
		{
			constexpr sbo_storage() noexcept : ext(local.data()) {}

			T* ext{nullptr};
			[[no_unique_address]] aligned_storage_static_array<T, 0> local;
		};
	} // namespace details
} // namespace psl