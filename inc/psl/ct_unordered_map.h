#pragma once
#include <unordered_map> // used for runtime lookup

// https://www.youtube.com/watch?v=X_p9X5RzBJE

#include "psl/details/metaprogramming.h"
#include "psl/details/fixed_string.h"
#include <functional>
#include <tuple>

namespace psl::ct
{
	template <psl::details::fixed_string Name, typename T>
	struct named_function
	{
		static constexpr psl::string_view name{Name};
		using type = std::function<T>;

		type value;
	};

	template <typename Value, psl::details::fixed_string... keys>
	class cache_map
	{
		// using key_types = std::tuple<unique_type<Keys>...>;

		using Key = psl::string_view;

	  public:
		cache_map() { ((m_Runtime[static_cast<Key>(keys)] = {}), ...); }

		consteval bool contains(Key key) const noexcept
		{
			return psl::metaprogramming::any_of([&key]() { return key == static_cast<Key>(keys); }...);
		}

		template <psl::details::fixed_string key, typename T>
		constexpr void assign(T&& value)
		{
			key_check<key>();
			m_Runtime[key] = std::forward<T>(value);
		}

		std::unordered_map<Key, Value>* operator->() { return &m_Runtime; }

		auto begin() noexcept { return m_Runtime.begin(); }
		auto end() noexcept { return m_Runtime.end(); }

		template <psl::details::fixed_string key>
		constexpr Value& at() noexcept
		{
			key_check<key>();
			return m_Runtime.at(key);
		}

		template <psl::details::fixed_string key>
		constexpr const Value& at() const noexcept
		{
			key_check<key>();
			return m_Runtime.at(key);
		}

	  private:
		template <psl::details::fixed_string key>
		constexpr void key_check() const noexcept
		{
			static_assert(psl::metaprogramming::any_of([]() { return key == static_cast<Key>(keys); }...),
						  "key was not present");
		}

		std::unordered_map<Key, Value> m_Runtime;
	};

	namespace details
	{
		template <psl::details::fixed_string Name, typename... Entries, size_t... Is>
		consteval size_t index_of_impl(std::index_sequence<Is...>)
		{
			using tuple_t = std::tuple<Entries...>;
			size_t i	  = sizeof...(Entries);
			(
				[&i]() {
					if(std::tuple_element_t<Is, tuple_t>::name == Name) i = Is;
				}(),
				...);

			return i;
		}
		template <psl::details::fixed_string Name, typename... Entries>
		consteval size_t index_of()
		{
			return index_of_impl<Name, Entries...>(std::make_index_sequence<sizeof...(Entries)>{});
		}

	} // namespace details

	template <typename... Entries>
	class named_cache_map
	{
	  public:
		template <psl::details::fixed_string Name, typename... Args>
		auto invoke(Args&&... args)
		{
			return std::invoke(find<Name>(), std::forward<Args>(args)...);
		}

		template <psl::details::fixed_string Name>
		void assign(auto fn)
		{
			find<Name>() = fn;
		}


	  private:
		template <psl::details::fixed_string Name>
		inline auto& find()
		{
			return std::get<details::index_of<Name, Entries...>()>(m_Entries).value;
		}

		std::tuple<Entries...> m_Entries;
	};
} // namespace psl::ct
