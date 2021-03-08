#pragma once
#include <psl/serialization/property.hpp>

namespace psl::serialization
{
	using version_type_t = size_t;
	inline namespace details
	{
		struct no_version_info_tag
		{};

		struct sentinel_version_tag
		{};

		template <typename T>
		consteval version_type_t current_version();
	} // namespace details


	static constexpr auto unversioned_v = std::numeric_limits<version_type_t>::max();

	template <typename T>
	struct version_settings_minimum
	{
		static constexpr version_type_t value{0u};
	};

	template <typename T>
	static constexpr auto version_settings_minimum_v = version_settings_minimum<T>::value;

	template <typename T>
	struct version_settings_default
	{
		static constexpr version_type_t value{((current_version<T>() >= version_settings_minimum_v<T>)
												   ? current_version<T>()
												   : version_settings_minimum_v<T>)};
	};

	template <typename T>
	static constexpr auto version_settings_default_v = version_settings_default<T>::value;


	template <typename T, version_type_t Version = version_settings_default_v<T>>
	struct version : public no_version_info_tag
	{};

	template <typename T>
	struct version<T, unversioned_v> : public sentinel_version_tag
	{};


	inline namespace
	{
		template <typename T>
		struct is_version : public std::false_type
		{};
		template <typename T, version_type_t Version>
		struct is_version<version<T, Version>> : public std::true_type
		{};

		template <typename T>
		struct version_underlying_type
		{};
		template <typename T, version_type_t Version>
		struct version_underlying_type<version<T, Version>>
		{
			using type = T;
		};
	} // namespace

	template <typename T>
	concept IsVersion = is_version<std::remove_cvref_t<T>>::value;

	template <typename T, version_type_t Version>
	concept HasVersion = IsVersion<version<T, Version>> && !std::is_base_of_v<no_version_info_tag, version<T, Version>>;

	template <typename T>
	concept IsUnversioned = IsVersion<version<T, 0>> && !HasVersion<T, 0>;

	inline namespace
	{
		template <typename T, version_type_t l = version_settings_minimum_v<T>,
				  version_type_t r = std::numeric_limits<version_type_t>::max()>
		consteval version_type_t current_version_impl()
		{
			if(r >= l)
			{
				constexpr version_type_t mid = l + (r - l) / 2;
				constexpr bool current		 = HasVersion<T, mid>;
				constexpr bool next = mid != std::numeric_limits<version_type_t>::max() && HasVersion<T, mid + 1>;
				if constexpr(current && !next)
					return mid;
				else if constexpr(!next)
					return current_version_impl<T, l, mid - 1>();
				else
					return current_version_impl<T, mid + 1, r>();
			}

			return unversioned_v;
		}

		template <typename T>
		consteval version_type_t current_version()
		{
			static_assert(std::is_base_of_v<sentinel_version_tag, version<T, unversioned_v>>,
						  "you cannot specialize the maximum version value, it is reserved as a sentinel value");
			// short circuit binary search.
			// the following will try to find the the highest version available with
			// early outs for smaller versions to lower iteration pressure.
			if constexpr(IsUnversioned<T>)
				return unversioned_v;
			else if constexpr(!HasVersion<T, 2 ^ 4>)
				return current_version_impl<T, version_settings_minimum_v<T>, 2 ^ 4>();
			else if constexpr(!HasVersion<T, 2 ^ 8>)
				return current_version_impl<T, version_settings_minimum_v<T>, 2 ^ 8>();
			else if constexpr(!HasVersion<T, 2 ^ 16>)
				return current_version_impl<T, version_settings_minimum_v<T>, 2 ^ 16>();
			else if constexpr(!HasVersion<T, 2 ^ 32>)
				return current_version_impl<T, version_settings_minimum_v<T>, 2 ^ 32>();
			else
				return current_version_impl<T>();
		}
	} // namespace

	template <typename T, version_type_t TargetVersion>
	constexpr auto value_upgrade(T& value) noexcept
	{}

	template <IsVersion T, version_type_t TargetVersion = version_settings_default_v<version_underlying_type<T>::type>>
	constexpr auto version_upgrade(const T& value) noexcept
	{
		using contained_type = version_underlying_type<T>::type;
		version<contained_type, TargetVersion> res;


		return res;
	}
} // namespace psl::serialization
