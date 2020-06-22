#pragma once
#include "psl/meta/library.h"
#include "psl/allocator.h"
#include "psl/exception.h"
#include "psl/config.h"

#include <filesystem>
/**
 * By default, when a resource from disk is requested, it is deserialized. A copy of this is stored internally, with a
 * timestamp, and a copy-count (i.e. the count it has been requested since storing). This is then used for subsequent
 * requests for the same resource, it invokes the copy constructor instead of going through the deserialization process
 * again.
 *
 * When memory limits are hit, it will go through the duplicated resources and remove those that are big/old/not
 * requested, to free up memory
 *
 * This behaviour can be selectively disabled for specific types, or globally disabled. Its aim is to provide a
 * fast-path for resources that might be loaded several times.
 *
 */

namespace psl::config
{
	template <typename T>
	static constexpr bool resources_store_copy = true;

	template <typename T>
	static constexpr bool resources_store_copy_for = resources_store_copy<default_setting_t>;
} // namespace psl::config

namespace psl::resource
{
	template <typename T, typename Args...>
	concept IsResource = std::is_constructible_v<T, psl::uid, psl::meta::data*, Args...>;

	class cache
	{
	  public:
		cache() noexcept = default;
		cache(std::shared_ptr<psl::meta::library> library, psl::allocator& allocator) noexcept
			: m_Library(library), m_Allocator(&allocator)
		{}
		cache(std::shared_ptr<psl::meta::library> library) noexcept : m_Library(library) {}
		cache(psl::allocator& allocator) noexcept : m_Allocator(allocator) {}

		template <typename T, typename... Args>
		requires IsResource<T, Args...> std::shared_ptr<T> load(const psl::uid& uid = psl::uid::generate(),
																Args&&... args)
		{
			auto [meta, added] = m_Library->try_emplace<T>(uid);


			if(m_Library->is_disk_file(uid))
			{
				const auto& location = m_Library->location(uid);
				PSL_NOT_IMPLEMENTED(0);

				// handle deserialization

				if constexpr(resources_store_copy_for<T>)
				{
					if(added)
					{
						PSL_NOT_IMPLEMENTED(0);
					}
				}
			}

			return std::make_shared<T>(uid, meta, std::forward<Args>(args)...);
		}


	  private:
		std::shared_ptr<psl::meta::library> m_Library{std::make_shared<psl::meta::library>()};
		psl::allocator* m_Allocator{&psl::default_allocator};
		std::unordered_map<psl::uid, void*> m_ResourceMap;
		std::unordered_map<void*, psl::uid> m_UidMap;
	};
} // namespace psl::resource