#include <stddef.h> // size_t
#include <string>
#include <string_view>

#include <functional>
#include <tuple>

#include "benchmark/benchmark.h"

namespace psl::details
{
	template <size_t N>
	struct fixed_string
	{
		char buf[N + 1]{};
		consteval fixed_string(char const *s)
		{
			for(size_t i = 0; i != N; ++i) buf[i] = s[i];
		}
		auto operator<=>(const fixed_string &) const = default;

		constexpr char operator[](size_t index) const noexcept { return buf[index]; }

		constexpr operator std::string_view() const noexcept { return std::string_view{buf, N}; }
		constexpr operator char const *() const { return buf; }

		constexpr size_t size() const noexcept { return N; }

		template <size_t start, size_t end>
		consteval fixed_string<end - start> substr() const noexcept
		{
			static_assert(start <= end);
			static_assert(end <= N + 1);
			return fixed_string<end - start>{&buf[start]};
		}
	};
	template <unsigned N>
	fixed_string(char const (&)[N])->fixed_string<N - 1>;
} // namespace psl::details

namespace psl::ct
{
	template <psl::details::fixed_string Name, typename T>
	struct named_value
	{
		static constexpr std::string_view name{Name};
		using type = T;

		type value;
	};

	inline namespace details
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

	template <typename UniqueTag, typename... Entries>
	class named_static_cache_map
	{
		template <size_t... Is>
		constexpr void re_assign(const named_static_cache_map &other, std::index_sequence<Is...>)
		{
			(void(m_FN<Is> = other.m_FN<Is>), ...);
		}

	  public:
		constexpr named_static_cache_map() noexcept = default;
		constexpr ~named_static_cache_map()			= default;

		named_static_cache_map &operator=(const named_static_cache_map &other)
		{
			if(this != &other)
			{
				re_assign(other, std::make_index_sequence<sizeof...(Entries)>());
			}
			return *this;
		}

		template <psl::details::fixed_string Name>
		static constexpr void assign(auto fn) noexcept
		{
			static_assert(index_of<Name, Entries...>() < sizeof...(Entries), "key was not found");
			m_FN<index_of<Name, Entries...>()> = fn;
		}
		template <psl::details::fixed_string Name>
		static constexpr auto &get()
		{
			static_assert(index_of<Name, Entries...>() < sizeof...(Entries), "key was not found");
			return m_FN<index_of<Name, Entries...>()>;
		}

	  private:
		template <size_t I>
		static typename std::tuple_element_t<I, std::tuple<Entries...>>::type m_FN{};
	};

	template <typename UniqueTag, typename... Entries>
	template <size_t I>
	typename std::tuple_element_t<I, std::tuple<Entries...>>::type named_static_cache_map<UniqueTag, Entries...>::m_FN;
} // namespace psl::ct

constexpr void start() {}
constexpr size_t accumulate(size_t frame) { return 10 + frame; }
constexpr void destroy() {}

static void named_static_cache_map_std_function(benchmark::State &state)
{
	struct tag
	{};
	constexpr psl::ct::named_static_cache_map<tag, psl::ct::named_value<"start", std::function<void()>>,
											  psl::ct::named_value<"accumulate", std::function<size_t(size_t)>>,
											  psl::ct::named_value<"destroy", std::function<void()>>>
		ct_map{};

	ct_map.assign<"start">(&start);
	ct_map.assign<"accumulate">(&accumulate);
	ct_map.assign<"destroy">(&destroy);

	auto second = ct_map;

	for(auto _ : state)
	{
		ct_map.get<"start">()();
		size_t res{0};
		benchmark::DoNotOptimize(res);
		for(size_t i = 0u; i < 10u; ++i)
		{
			res += ct_map.get<"accumulate">()(i);
		}
		ct_map.get<"destroy">()();
	}
}


BENCHMARK(named_static_cache_map_std_function);

struct member_fn
{
	void start(){

	};

	size_t accumulate(size_t frame) { return 10 + frame; }
};

static void named_static_cache_map_func_ptr(benchmark::State &state)
{
	struct tag
	{};
	psl::ct::named_static_cache_map<tag, psl::ct::named_value<"start", void (*)()>,
									psl::ct::named_value<"accumulate", size_t (*)(size_t)>,
									psl::ct::named_value<"destroy", void (*)()>>
		ct_map{};

	ct_map.assign<"start">(&start);
	ct_map.assign<"accumulate">(&accumulate);
	ct_map.assign<"destroy">(&destroy);

	for(auto _ : state)
	{
		ct_map.get<"start">()();
		size_t res{0};
		benchmark::DoNotOptimize(res);
		for(size_t i = 0u; i < 10u; ++i)
		{
			res += ct_map.get<"accumulate">()(i);
		}
		ct_map.get<"destroy">()();
	}
}


BENCHMARK(named_static_cache_map_func_ptr);

#include <unordered_map>
#include <string_view>

static void unordered_map_std_function(benchmark::State &state)
{
	using namespace std::string_literals;
	std::unordered_map<std::string_view, std::function<void()>> m_Map{};
	size_t res{0};
	benchmark::DoNotOptimize(res);

	m_Map.emplace("start", []() {});
	m_Map.emplace("accumulate", [&res]() { res += accumulate(0); });
	m_Map.emplace("destroy", []() {});

	for(auto _ : state)
	{
		m_Map["start"]();
		res = 0;
		for(size_t i = 0u; i < 10u; ++i)
		{
			m_Map["accumulate"]();
		}
		m_Map["destroy"]();
	}
}

BENCHMARK(unordered_map_std_function);

static void unordered_map_func_ptr(benchmark::State &state)
{
	using namespace std::string_literals;
	std::unordered_map<std::string_view, void (*)()> m_Map{};
	size_t res{0};
	benchmark::DoNotOptimize(res);

	m_Map.emplace("start", []() {});
	m_Map.emplace("accumulate", []() { accumulate(0); });
	m_Map.emplace("destroy", []() {});
	benchmark::DoNotOptimize(m_Map);
	for(auto _ : state)
	{
		m_Map["start"]();
		res = 0;
		for(size_t i = 0u; i < 10u; ++i)
		{
			m_Map["accumulate"]();
		}
		m_Map["destroy"]();
	}
}

BENCHMARK(unordered_map_func_ptr);

static void baseline_std_function(benchmark::State &state)
{
	std::function<void()> st{&start};
	std::function<size_t(size_t)> acc{&accumulate};
	std::function<void()> dest{&destroy};
	benchmark::DoNotOptimize(st);
	benchmark::DoNotOptimize(acc);
	benchmark::DoNotOptimize(dest);

	for(auto _ : state)
	{
		st();
		size_t res{0};
		benchmark::DoNotOptimize(res);
		for(size_t i = 0; i < 10; ++i)
		{
			res += acc(i);
		}
		dest();
	}
}

BENCHMARK(baseline_std_function);

static void baseline_func_ptr(benchmark::State &state)
{
	void (*st)(){&start};
	size_t (*acc)(size_t){&accumulate};
	void (*dest)(){&destroy};
	benchmark::DoNotOptimize(st);
	benchmark::DoNotOptimize(acc);
	benchmark::DoNotOptimize(dest);

	for(auto _ : state)
	{
		st();
		size_t res{0};
		benchmark::DoNotOptimize(res);
		for(size_t i = 0; i < 10; ++i)
		{
			res += acc(i);
		}
		dest();
	}
}

BENCHMARK(baseline_func_ptr);

BENCHMARK_MAIN();