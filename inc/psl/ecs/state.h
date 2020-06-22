#pragma once
#include <span>
#include "psl/concepts.h"

#include "psl/ecs/entity.h"

#include <unordered_map>

namespace psl::ecs
{
	struct component_key_t
	{
		using key_t = void (*)();
		key_t key;


		template <typename T>
		consteval void key_function()
		{
			return;
		}

		template <typename T>
		consteval component_key_t(psl::type_t<T>) noexcept : key(key_function<std::remove_cvref_t<T>>)
		{}
	};

	class state
	{
	  public:
		template <typename... Ts>
		constexpr void add_components(std::span<entity> entities, Ts&&... types)
		{
			(add_component_impl(entities, types), ...);
		}

	  private:
		template <typename T>
		constexpr void add_component_impl(std::span<entity> entities, T&& type)
		{
			if constexpr(std::is_invocable_v<T>)
			{
				for(auto e : entities)
				{
				}
			}
			else if constexpr(IsRange<T>)
			{}
			else
			{}
		}

		mutable std::unordered_map<component_key_t, std::unique_ptr<component_storage_base>> m_ComponentStorage;
	};
} // namespace psl::ecs