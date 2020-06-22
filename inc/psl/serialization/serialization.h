#pragma once
#include <tuple>
#include "psl/serialization/property.h"
#include "psl/serialization/versioning.h"
#include "psl/config.h"
#include "psl/array.h"
#include "psl/exception.h"
#include "psl/details/crc.h"

#include <cstring>
#include <span>

#define POLYMORPHIC_INFO(name)                                                                                         \
	namespace psl                                                                                                      \
	{                                                                                                                  \
		template <>                                                                                                    \
		struct polymorphic_info<psl::crc64<u8"" #name "">()>                                                           \
		{                                                                                                              \
			auto properties(auto target) noexcept { return reinterpret_cast<name*>(target)->properties(); };           \
		};                                                                                                             \
	}

#define POLYMORPHIC_ID(name)                                                                                           \
	static constexpr psl::polymorphic_id<u8"" #name ""> polymorphic_id_value{};                                        \
	virtual uint64_t polymorphic_id() { return polymorphic_id_value; }

#define PROPERTY_PACK(...)                                                                                             \
	auto properties() noexcept { return psl::make_property_pack(__VA_ARGS__); }

/**
 * This file contains the methods to achieve both serialization and deserialization.
 *
 * De/Serializer:
 * There are few requirements needed, the major one being an accessible "properties()" method being present in your
 * objects. If not present, it is assumed the object is a 'value type', which implies it can be serialized (take for
 * example int, float, string, etc..).
 *
 * You are allowed to 'hide' the properties method, but you will have to befriend the 'struct
 * psl::serialization_accessor'. The easiest way to write your properties method is the following:
 * "auto properties() noexcept { return psl::make_property_pack([YOUR PROPERTIES HERE]);}"
 *
 * All serializable properties need to be of the type 'psl::property<>'.
 *
 * The psl::serialize method expects an object to kick-off the serialization process. You cannot start serialization on
 * a 'value type'.
 *
 * By default versioning is on, you can disable this in 'psl::config::serialization_versioned'.
 *
 * Writer/Reader:
 *
 */

namespace psl
{
	template <uint64_t ID>
	struct polymorphic_info
	{};

	template <psl::details::fixed_string Name>
	struct polymorphic_id
	{
		constexpr operator uint64_t() const noexcept { return value; }

	  private:
		static constexpr uint64_t value{psl::crc64<Name>()};
	};

	namespace config
	{
		template <typename T>
		constexpr static bool serialization_versioned = true;
	}

	template <typename>
	constexpr bool is_property_pack = false;

	template <template <typename...> typename Tuple, typename... Ts>
	requires(IsNamedProperty<std::remove_cvref_t<Ts>>&&...) constexpr bool is_property_pack<Tuple<Ts...>> = true;

	template <typename T>
	concept IsPropertyPack = is_property_pack<T>;

	namespace internal
	{
		/**
		 * Defined for usage in serialization_accessor, due to the nested nature this concept is redefined several
		 * times.
		 *
		 * For end-users, please use psl::IsSerializable instead, don't call into this manually, it's unneeded, verbose,
		 * and error prone. It only exists to remove circular dependencies.
		 */
		template <typename T, typename A>
		concept IsSerializable = requires(std::remove_pointer_t<T> t, A a)
		{
			{
				a.template properties<std::remove_pointer_t<T>>(&t)
			}
			->IsPropertyPack;
		};

		template <typename T, typename A>
		concept IsObjectProperty = IsNamedProperty<T>&& IsSerializable<typename T::value_type, A>;
	} // namespace internal

	template <typename T>
	concept IsWriter = true;

	template <typename T>
	concept IsReader = true;

	template <typename T>
	concept CanWriterFail = IsWriter<T>&&
		std::is_same_v<bool, std::invoke_result_t<decltype(T::write), psl::string_view, psl::string_view>>;

	template <typename... Ts>
	auto make_property_pack(Ts&... properties)
	{
		return std::tuple<Ts&...>{properties...};
	}

	template <typename... Ts, typename... Ys>
	auto make_property_pack(std::tuple<Ys&...> base, Ts&... properties)
	{
		return std::tuple_cat(base, make_property_pack(properties...));
	}

	struct serialization_accessor;

	template <typename T>
	requires internal::IsSerializable<T, serialization_accessor> bool serialize(T& target, IsWriter auto& ostream,
																				psl::string_view root_name = u8"root");

	inline namespace details
	{
		template <typename T>
		requires internal::IsObjectProperty<T, serialization_accessor> bool serialize(T& target,
																					  IsWriter auto& ostream);
	} // namespace details

	struct serialization_accessor
	{
	  public:
		template <typename T>
		consteval static bool is_serializable()
		{
			return requires(T t)
			{
				{
					t.properties()
				}
				->IsPropertyPack;
			};
		}

	  private:
		template <typename T>
		// we repeat the concept declaration of 'IsSerializable in here, as we need it to disambiguate between
		// Serializable objects, and value objects; and because of the nature of not being able to access the properies
		// member fn when it is not public, we have to resolve it like this.
		requires(is_serializable<T>()) auto properties(T* target)
		{
			return target->properties();
		}


		template <typename T>
		requires(is_serializable<T>()) auto properties(T& target)
		{
			return target.properties();
		}

		template <typename T>
		requires internal::IsSerializable<T, serialization_accessor> friend bool
		serialize(T& target, IsWriter auto& ostream, psl::string_view root_name);
		template <typename T>
		requires internal::IsObjectProperty<T, serialization_accessor> friend bool
		details::serialize(T& target, IsWriter auto& ostream);
	};

	inline namespace details
	{
		template <typename T>
		requires internal::IsObjectProperty<T, serialization_accessor> bool serialize(T& target, IsWriter auto& ostream)
		{
			size_t version{0};
			if constexpr(config::serialization_versioned<config::default_setting_t>)
			{
				version = psl::versioning::latest_version_for_or<std::remove_cvref_t<decltype(target)>>();
			}
			if constexpr(std::is_pointer_v<T>)
			{
				ostream.begin_scope(target.name(), version, target->polymorphic_id());
				bool result = true;
				std::apply(
					[&result, &ostream](auto&... property) {
						(..., void(result &= psl::details::serialize(property, ostream)));
					},
					serialization_accessor{}.properties(target));
				ostream.end_scope(target.name());
				return result;
			}
			else
			{
				ostream.begin_scope(target.name(), version);
				bool result = true;
				std::apply(
					[&result, &ostream](auto&... property) {
						(..., void(result &= psl::details::serialize(property, ostream)));
					},
					serialization_accessor{}.properties(&target));
				ostream.end_scope(target.name());
				return result;
			}
		}

		bool serialize_value(psl::string_view name, auto const& target, IsWriter auto& ostream)
		{
			using value_type = std::remove_cvref_t<decltype(target)>;
			if constexpr(CanWriterFail<std::remove_cvref_t<decltype(ostream)>>)
			{
				return ostream.template write_value<value_type>(name, target);
			}
			else
			{
				ostream.template write_value<value_type>(name, target);
				return true;
			}
		}
		bool serialize_range(psl::string_view name, auto const& target, IsWriter auto& ostream)
		{
			using value_type = std::remove_cvref_t<decltype(target)>;
			if constexpr(CanWriterFail<std::remove_cvref_t<decltype(ostream)>>)
			{
				return ostream.template write_range<value_type>(name, target.begin(), target.end());
			}
			else
			{
				ostream.template write_range<value_type>(name, target.begin(), target.end());
				return true;
			}
		}

		bool serialize(IsNamedProperty auto const& target,
					   IsWriter auto& ostream) requires IsReferenceProperty<std::remove_cvref_t<decltype(target)>>
		{
			PSL_NOT_IMPLEMENTED(0);

			return false;
		}
		bool serialize(IsNamedProperty auto const& target,
					   IsWriter auto& ostream) requires IsOptionalProperty<std::remove_cvref_t<decltype(target)>>
		{
			if(target)
			{
				using property_type = std::remove_cvref_t<decltype(target)>;
				if constexpr(IsRangeProperty<property_type>)
				{
					return serialize_range(target.name, target->value(), ostream);
				}
				else
				{
					return serialize_value(target.name, target->value(), ostream);
				}
			}
			return true;
		}

		bool serialize(IsNamedProperty auto const& target, IsWriter auto& ostream)
		{
			using property_type = std::remove_cvref_t<decltype(target)>;
			if constexpr(IsRangeProperty<property_type>)
			{
				return serialize_range(target.name, target.value(), ostream);
			}
			else
			{
				return serialize_value(target.name, target.value(), ostream);
			}
		}

	} // namespace details

	template <typename T>
	requires internal::IsSerializable<T, serialization_accessor> bool serialize(T& target, IsWriter auto& ostream,
																				psl::string_view root_name)
	{
		size_t version{0};
		if constexpr(config::serialization_versioned<config::default_setting_t>)
		{
			version = psl::versioning::latest_version_for_or<std::remove_cvref_t<T>>();
		}
		if constexpr(std::is_pointer_v<T>)
		{
			ostream.begin_scope(root_name, version, target->polymorphic_id());
			bool result = true;
			std::apply([&result, &ostream](
						   auto&... property) { (..., void(result &= psl::details::serialize(property, ostream))); },
					   serialization_accessor{}.properties(target));
			ostream.end_scope(root_name);

			return false;
		}
		else
		{
			ostream.begin_scope(root_name, version);
			bool result = true;
			std::apply([&result, &ostream](
						   auto&... property) { (..., void(result &= psl::details::serialize(property, ostream))); },
					   serialization_accessor{}.properties(&target));
			ostream.end_scope(root_name);
			return result;
		}
	}

	template <typename T>
	concept IsSerializable = serialization_accessor::is_serializable<T>();
} // namespace psl