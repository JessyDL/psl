#pragma once
#include "psl/details/fixed_string.h"
#include "psl/concepts.h"

namespace psl
{
	template <typename T>
	concept IsProperty = requires(T t)
	{
		T::name;
		typename T::value_type;
	}
	&&(sizeof(T) == sizeof(typename T::value_type));

	template <typename T>
	concept IsUnnamedProperty = IsProperty<T>&& T::name.size() == 0;
	template <typename T>
	concept IsNamedProperty = IsProperty<T>&& T::name.size() != 0;

	template <typename T, typename Y>
	concept IsSimilarProperty =
		IsProperty<T>&& IsProperty<Y>&& std::is_same_v<typename T::value_type, typename Y::value_type>;
	template <typename T, typename Y>
	concept IsConvertibleProperty =
		IsProperty<T>&& IsProperty<Y>&& std::is_convertible_v<typename Y::value_type, typename T::value_type>;

	template <typename T, typename Y>
	concept IsNothrowConvertibleProperty =
		IsProperty<T>&& IsProperty<Y>&& std::is_nothrow_convertible_v<typename Y::value_type, typename T::value_type>;

	template <typename T>
	concept IsRangeProperty = IsProperty<T>&& IsRange<typename T::value_type>;

	template <typename T>
	concept IsOptionalProperty = IsProperty<T>&& IsOptional<typename T::value_type>;

	template <typename T>
	concept IsReferenceProperty = IsProperty<T> && false;

	template <typename T, details::fixed_string Name = u8"">
	class property final
	{
	  public:
		using value_type	  = T;
		using reference		  = value_type&;
		using const_reference = const value_type&;
		using pointer		  = value_type*;
		using const_pointer   = const value_type*;

		static_assert(!IsProperty<value_type>, "cannot contain nested properties");
		static constexpr psl::string_view name{Name};

		template <typename... Ts>
		constexpr property(Ts&&... value) noexcept(
			std::is_nothrow_constructible_v<value_type, Ts...>) requires std::is_constructible_v<value_type, Ts...>
			: data(std::forward<Ts>(value)...)
		{}

		template <IsProperty U>
		constexpr property(const U& rhs) noexcept(
			std::is_nothrow_copy_constructible_v<T>&& IsNothrowConvertibleProperty<
				property, U>) requires std::is_copy_constructible_v<T>&& IsConvertibleProperty<property, U>
			: data(static_cast<value_type>(rhs))
		{}

		template <IsProperty U>
		constexpr property(U&& rhs) noexcept(
			std::is_nothrow_move_constructible_v<T>&& IsNothrowConvertibleProperty<
				property, U>) requires std::is_move_constructible_v<T>&& IsConvertibleProperty<property, U>
			: data(std::move(static_cast<value_type>(rhs)))
		{}

		template <IsProperty U>
		constexpr property& operator=(const U& rhs) noexcept(
			std::is_nothrow_copy_assignable_v<T>&& IsNothrowConvertibleProperty<
				property, U>) requires std::is_copy_assignable_v<T>&& IsConvertibleProperty<property, U>
		{
			if(this == (void*)&rhs) return *this;
			data = static_cast<value_type>(rhs);
			return *this;
		}
		template <IsProperty U>
		constexpr property& operator=(U&& rhs) noexcept(
			std::is_nothrow_move_assignable_v<T>&& IsNothrowConvertibleProperty<
				property, U>) requires std::is_move_assignable_v<T>&& IsConvertibleProperty<property, U>
		{
			if(this == (void*)&rhs) return *this;
			data = std::move(static_cast<value_type>(rhs));
			return *this;
		}

		constexpr ~property() noexcept(std::is_nothrow_destructible_v<T>) = default;

		constexpr pointer operator->() noexcept { return &data; }
		constexpr const_pointer operator->() const noexcept { return &data; }

		constexpr operator T&() noexcept { return data; }
		constexpr operator const T&() const noexcept { return data; }

		constexpr reference operator*() noexcept { return data; }
		constexpr const_reference operator*() const noexcept { return data; }

		template <typename Y>
		constexpr operator Y() const noexcept(std::is_nothrow_convertible_v<T, Y>) requires std::is_convertible_v<T, Y>
		{
			return static_cast<Y>(data);
		}

		// misc operators
		constexpr auto operator[](IsIntegral auto index) noexcept(
			noexcept(std::declval<value_type>()[std::declval<decltype(index)>()]))
			-> decltype(std::declval<value_type>()[std::declval<decltype(index)>()]) requires requires(T t)
		{
			t[std::declval<decltype(index)>()];
		}
		{
			return data[index];
		}

		constexpr auto operator[](IsIntegral auto index) const
			noexcept(noexcept(std::declval<const value_type>()[std::declval<decltype(index)>()])) -> decltype(
				std::declval<const value_type>()[std::declval<decltype(index)>()]) requires requires(const T t)
		{
			t[std::declval<decltype(index)>()];
		}
		{
			return data[index];
		}

		// begin/end for array-likes
		constexpr auto begin() noexcept requires IsRange<value_type> { return data.begin(); }
		constexpr auto begin() const noexcept requires IsRange<value_type> { return data.begin(); }
		constexpr auto end() noexcept requires IsRange<value_type> { return data.end(); }
		constexpr auto end() const noexcept requires IsRange<value_type> { return data.end(); }

		constexpr operator bool() const noexcept requires requires(value_type t) { t.operator bool(); }
		{
			return data.operator bool();
		}

		constexpr value_type& value() noexcept { return data; }
		constexpr const value_type& value() const noexcept { return data; }

	  private:
		value_type data;
	};
} // namespace psl