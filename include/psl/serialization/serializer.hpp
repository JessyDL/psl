#pragma once
#include <tuple>
#include <functional>
#include <psl/serialization/property.hpp>
#include <psl/serialization/versioning.hpp>

namespace psl::serialization
{
	constexpr auto make_properties(IsProperty auto&... props) noexcept { return std::tuple{std::ref(props)...}; }

	inline namespace details
	{
		template <typename T>
		concept IsSerializeableObject = requires(T t)
		{
			t.properties();
		};

		constexpr bool serialize_impl(auto& serializer, const auto& root)
		{
			using T = std::remove_cvref_t<decltype(root)>;
			if constexpr(IsProperty<T>)
			{
				if constexpr(SupportsRangeOperations<T>)
				{
					serializer.range_begin(root.name());
					for(const auto& entry : root.value())
					{
						if constexpr(IsSerializeableObject<decltype(entry)>)
						{
							serializer.object_begin();
							std::apply(
								[&serializer](auto&&... properties) {
									(serialize_impl(serializer, properties.get()), ...);
								},
								entry.properties());
							serializer.object_end();
						}
						else
							serialize_impl(serializer, entry);
					}
					serializer.range_end(root.name());
				}
				else
				{
					serializer.value(root.name(), root.value());
				}
			}
			else
			{
				std::apply([&serializer](auto&&... properties) { (serialize_impl(serializer, properties.get()), ...); },
						   root.properties());
			}
			return true;
		};

		template <typename T>
		constexpr auto deserialize_impl(auto& deserializer) -> T
		{
			if constexpr(IsProperty<T>)
			{
				return deserializer.template read<T>(T::name);
			}
			else
			{
				return {};
			}
		}
	} // namespace details

	constexpr bool serialize(auto& serializer, const auto& root)
	{
		serializer.object_begin();
		serialize_impl(serializer, root);
		serializer.object_end();
		return true;
	};

	template <typename T>
	constexpr T deserialize(auto& deserializer)
	{
		deserializer.read_object_begin();
		auto res = deserialize_impl<T>(deserializer);
		deserializer.read_object_end();
		return res;
	};

	constexpr void deserialize(auto& deserializer, auto& target)
	{
		using T = std::remove_cvref_t<decltype(target)>;
		deserializer.read_object_begin();
		target = deserialize_impl<T>(deserializer);
		deserializer.read_object_end();
	};
} // namespace psl::serialization
