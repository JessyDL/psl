#pragma once
#include <psl/exceptions.hpp>
#include <psl/type_concepts.hpp>
#include <psl/types.hpp>

namespace psl
{
	struct nullopt_t : public _priv::id_token<nullopt_t>
	{
		explicit constexpr nullopt_t(identifier) {}
	};
	inline constexpr nullopt_t nullopt{nullopt_t::identifier::token};

	inline namespace details
	{
		template <typename T>
		struct optional_value_storage;

		template <typename T>
		struct is_optional_value : std::false_type
		{};
		template <typename T>
		struct is_optional_value<optional_value_storage<T>> : std::true_type
		{};

		template <typename T>
		concept IsOptionalValue = is_optional_value<std::remove_cvref_t<T>>::value;

		/**
		 * \brief Models an optional storage of a specific type.
		 *
		 * \tparam T
		 */
		template <typename T>
		struct optional_value_storage
		{
			explicit constexpr optional_value_storage(nullopt_t) noexcept : empty_marker({}), engaged(false) {}
			template <typename... Args>
			requires(disable_perfect_forward_illegal_type_v<optional_value_storage,
															Args...>) constexpr optional_value_storage(Args&&... args)
				: value(std::forward<Args>(args)...), engaged(true)
			{}

			constexpr optional_value_storage(const optional_value_storage& rhs) : empty_marker({}), engaged(rhs.engaged)
			{
				if(engaged)
				{
					value = rhs.value;
				}
			}
			constexpr optional_value_storage(optional_value_storage&& rhs) : empty_marker({}), engaged(rhs.engaged)
			{
				if(engaged)
				{
					value = std::move(rhs.value);
				}
			}
			constexpr optional_value_storage& operator=(const optional_value_storage& rhs)
			{
				if(this != &rhs) [[likely]]
					{
						engaged = rhs.engaged;
						if(engaged)
						{
							value = rhs.value;
						}
					}
				return *this;
			}
			constexpr optional_value_storage& operator=(optional_value_storage&& rhs)
			{
				if(this != &rhs) [[likely]]
					{
						engaged = rhs.engaged;
						// rhs.engaged = false;
						if(engaged)
						{
							value = rhs.value;
						}
					}
				return *this;
			}

			constexpr ~optional_value_storage() { reset(); }

			template <typename... Args>
			constexpr void set(Args&&... args)
			{
				reset();
				engaged = true;
				value	= std::move(T{std::forward<Args>(args)...});
			}

			constexpr void reset() noexcept(std::is_nothrow_destructible_v<T>)
			{
				if(engaged) [[likely]]
					{
						value.~T();
						empty_marker = {};
					}
				engaged = false;
			}

			explicit constexpr operator bool() const noexcept { return engaged; }
			constexpr bool has_value() const noexcept { return engaged; }
			constexpr const T& get() const noexcept { return value; }
			constexpr T& get() noexcept { return value; }

			constexpr auto take() noexcept
			{
				auto ret = std::move(value);
				reset();
				return ret;
			}

		  private:
			struct empty
			{};
			union
			{
				empty empty_marker{};
				T value;
			};
			bool engaged{false};
		};
	} // namespace details
} // namespace psl