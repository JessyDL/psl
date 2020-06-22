#pragma once
#include <stddef.h> // size_t
#include <utility>  // std::index_sequence
#include <type_traits>
#include "psl/string.h"
#include "psl/details/fixed_string.h"

namespace psl::versioning
{
	template <psl::details::fixed_string From, psl::details::fixed_string To>
	struct rename_t
	{
		static const constexpr psl::string_view from{From};
		static const constexpr psl::string_view to{To};
	};

	template <psl::details::fixed_string Name>
	struct disconnect_t
	{
		static const constexpr psl::string_view name{Name};
	};

	template <psl::details::fixed_string Name>
	struct skip_upgrade_if_possible_t
	{
		static const constexpr psl::string_view name{Name};
	};

	template <typename T, size_t Version>
	struct packet;

	template <typename T, size_t V>
	concept HasVersion = !requires
	{
		packet<T, V>::packet_version_identifier_key;
	};

	template <typename T>
	concept IsVersioned = HasVersion<T, 0>;


	template <typename T, size_t Version>
	concept IsPacket = std::is_standard_layout_v<packet<T, Version>>&& std::is_aggregate_v<packet<T, Version>>;

	template <typename T, size_t Version0, size_t Version1>
	concept IsPacketUniqueEnough = true; /*todo: wait till this is implemented !std::is_layout_compatible_v<packet<T,
										   Version0>, packet<T, Version1>>;*/

	/// ----------------
	/// version fetching
	/// ----------------
	namespace details
	{
		template <IsVersioned T, size_t Offset, size_t... Is>
		consteval size_t latest_version_for_impl(std::index_sequence<Is...>)
		{
			if constexpr(Offset >= 1024) // artificial limit, this can be expanded depending on the compiler
			{
				static_assert(!HasVersion<T, Offset>, "This exceeds the maximum supported versions");
				return 0;
			}
			else if constexpr(HasVersion<T, Offset>) // short circuit for compiler expansion
			{
				size_t result = 0;
				(
					[](auto& output) {
						if constexpr(HasVersion<T, Offset + Is>)
						{
							static_assert(IsPacket<T, Offset + Is>,
										  "did not satisfy the constraint for a valid packet implementation");

							if constexpr(Offset + Is > 0)
							{
								static_assert(IsPacketUniqueEnough<T, Offset + Is - 1, Offset + Is>,
											  "not enough difference to actually warrant a new version");
							}
							output = Offset + Is;
						}
					}(result),
					...);

				if(result == Offset + sizeof...(Is))
					return latest_version_for_impl<T, Offset + sizeof...(Is)>(std::index_sequence<Is...>{});
				return result;
			}
			else
			{
				return Offset;
			}
		}
	} // namespace details

	template <IsVersioned T>
	consteval size_t latest_version_for()
	{
		return details::latest_version_for_impl<T, 0>(std::make_index_sequence<64>());
	}

	template <typename T, size_t Override = 0>
	consteval size_t latest_version_for_or()
	{
		if constexpr(IsVersioned<T>)
		{
			return latest_version_for<T>();
		}
		else
			return Override;
	}

	template <typename T, size_t Version = latest_version_for<T>()>
	struct packet
	{
		static constexpr bool packet_version_identifier_key{true};
	};

	template <typename T, size_t Version0, size_t Version1>
	consteval bool has_changes()
	{
		return false;
	}

} // namespace psl::versioning