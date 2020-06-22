#pragma once
#include "psl/exception.h"
#include "psl/config.h"
#include "psl/meta/data.h"

#include <unordered_map>
#include <memory>

namespace psl::config
{
	namespace details
	{
		template <typename T>
		static psl::string_view library_extension = u8"lib";
	}
	static psl::string_view library_extension = details::library_extension<default_setting_t>;
} // namespace psl::config

namespace psl::meta
{
	/**
	 * \brief Keeps track of resource and their metadata
	 *
	 */
	class library
	{
		struct disk_info
		{
			// the data present in the file.
			psl::string data;
			// location of the source file in relation to the library.
			psl::string location;
			// location of the meta file in relation to the library.
			psl::string meta;
			// which environments does this file apply to, i.e. if you have files specific for Windows and Linux, this
			// would contain that.
			psl::string environments;
			// timestamp of the last alteration of either the source or meta file (whichever was latest). This is used
			// to figure out if a file needs to be reloaded.
			uint64_t timestamp;
		};

		struct entry
		{
			bool is_disk_file() const noexcept { return info != nullptr; }
			std::unique_ptr<psl::meta::data> data{nullptr};
			std::unique_ptr<disk_info> info{nullptr};
			psl::array<psl::uid> referencing{};
			psl::array<psl::uid> referenced_by{};
		};

	  public:
		library() noexcept					   = default;
		~library() noexcept					   = default;
		library(const library& other) noexcept = default;
		library(library&& other) noexcept	  = default;
		library& operator=(const library& other) noexcept = default;
		library& operator=(library&& other) noexcept = default;

		template <IsMetaData T = psl::meta::data, typename... Args>
		auto& add(psl::uid uid = psl::uid::generate(), std::span<const tag> tags = {},
				  Args&&... args) requires std::is_constructible_v<T, psl::uid, std::span<const tag>, Args...>
		{
			auto res = m_Entries.emplace(uid, entry{std::make_unique<T>(uid, tags, std::forward<Args>(args)...)});
			PSL_EXCEPT_IF(!res.second, "uid was already present in the library", std::logic_error);
			for(const auto& tag : tags)
			{
				auto res_tag = m_Tags.emplace(tag, uid);
				PSL_EXCEPT_IF(!res_tag.second, "tag was already present in the library", std::logic_error);
			}
			return *res.first->second.data;
		}

		template <typename T, typename... Args>
		meta_traits<T>::meta_type& add(psl::uid uid = psl::uid::generate(), std::span<const tag> tags = {},
									   Args&&... args)
		{
			return add<meta_traits<T>::meta_type, Args...>(std::move(uid), tags, std::forward<Args>(args)...);
		}

		template <IsMetaData T>
		T& find(const psl::uid& uid) noexcept(!config::exceptions)
		{
			auto it = m_Entries.find(uid);
			PSL_EXCEPT_IF(it == std::end(m_Entries), "uid was not present in the library", std::logic_error);

			return *reinterpret_cast<T*>(&*it->second.data);
		}

		template <typename T>
		meta_traits<T>::meta_type& find(const psl::uid& uid) noexcept(!config::exceptions)
		{
			return find<meta_traits<T>::meta_type>(uid);
		}

		auto try_find(const psl::uid& uid, IsMetaData auto& out)
		{
			auto it = m_Entries.find(uid);
			if(it != std::end(m_Entries))
			{
				out = *it->second.data;
				return true;
			}
			return false;
		}

		template <IsMetaData T = psl::meta::data, typename... Args>
		std::pair<T&, bool> try_emplace(const psl::uid& uid, Args&&... args)
		{
			auto res = m_Entries.emplace(uid, entry{std::make_unique<T>(uid, {}, std::forward<Args>(args)...)});
			return {*res.first->second.data, res.second};
		}

		template <typename T, typename... Args>
		std::pair<typename meta_traits<T>::meta_type&, bool> try_emplace(const psl::uid& uid, Args&&... args)
		{
			return try_emplace<meta_traits<T>::meta_type, Args...>(uid, std::forward<Args>(args)...);
		}

		bool remove(const psl::uid& uid) noexcept { return m_Entries.erase(uid) > 0; }
		bool is_disk_file(const psl::uid& uid) const noexcept { return m_Entries.find(uid)->second.is_disk_file(); }
		const auto& location(const psl::uid& uid) const noexcept { return m_Entries.find(uid)->second.info->location; }

	  private:
		std::unordered_map<psl::uid, entry> m_Entries;
		std::unordered_map<psl::meta::tag, psl::uid> m_Tags;
	};
} // namespace psl::meta