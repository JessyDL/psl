#pragma once
#include "psl/array.h"
#include <algorithm>
namespace psl
{
	template <typename Key, typename Value>
	class flat_unordered_map
	{
	  public:
		auto find(const auto& key)
		{
			return std::find_if(std::begin(m_Data), std::end(m_Data),
								[key](const auto& pair) { return pair.first == key; });
		}

		bool contains(const auto& key) const { return find(key) != end(); }

		auto begin() { return m_Data.begin(); }
		auto end() { return m_Data.end(); }

		template <typename... Args>
		std::pair<auto, bool> emplace(const auto& key, Args&&... args)
		{
			auto it = find(key);
			if(it == end())
			{
				m_Data.emplace_back(std::pair{key, Value{std::forward<Args>(args)...}});
				return std::pair{end() - 1, true};
			}
			return std::pair{it, false};
		}

		size_t erase(const auto& key)
		{
			auto it = find(key);
			if(it != end())
			{
				m_Data.erase(it);
				return 1u;
			}
			return 0u;
		}

		size_t size() const noexcept { return m_Data.size(); }

	  private:
		psl::array<std::pair<Key, Value>> m_Data;
	};
} // namespace psl