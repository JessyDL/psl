#include <psl/array.hpp>
#include <tests/types.hpp>

#include <litmus/suite.hpp>
#include <litmus/section.hpp>
#include <litmus/expect.hpp>
#include <litmus/generator/range.hpp>

using namespace psl;
using namespace litmus;

auto array_test0 = suite<"internal storage constraints", "psl", "psl::sbo_storage">()
					   .templates<tpack<int, int8_t>, vpack<12, 4, 0, 1>, vpack<true, false>>() =
	[]<typename T0, typename V0, typename V1>()
{
	using T = sbo_storage<T0, V0::value, sbo_alias<V1::value>>;

	static constexpr size_t pointer_size = (V1::value) ? 0u : sizeof(void*);

	static constexpr size_t max_size		   = V0::value * sizeof(T0);
	static constexpr size_t actual_size		   = (max_size >= pointer_size) ? max_size - pointer_size : 0u;
	static constexpr size_t expected_sbo_value = (actual_size - (actual_size % sizeof(T0))) / sizeof(T0);

	expect(T::SBO) == expected_sbo_value;
};

auto array_test1 = suite<"array", "psl", "psl::array", "containers">(generator::array<0, 8, 128>{})
					   .templates<tpack<int>, vpack<psl::dynamic_extent, 512>>() =
	[]<typename T, typename V0>(size_t start_elements)
{
	using std::next, std::prev;
	using array_t = psl::array<T, V0::value>;
	using value_t = array_t::value_type;

	array_t arr{};
	expect(arr.size()) == 0u;
	expect((arr.size()) <= arr.capacity());
	expect((arr.is_stored_inlined() || arr.sbo_size() == 0)) == true;
	static_assert(is_array_v<const array_t>);
	static_assert(IsArray<array_t>);

	section<"emplace">() = [&] {
		for(int i = 0; i < (int)arr.sbo_size(); ++i)
		{
			arr.emplace_back(i);
		}
		expect((arr.is_stored_inlined() || arr.sbo_size() == 0)) == true;
		expect(arr.size()) == arr.sbo_size();
		section<"force external storage">() = [&] {
			arr.emplace_back(arr.size());
			expect(arr.is_stored_inlined()) == false;
			expect(arr.size()) > arr.sbo_size();
		};

		for(int i = 0; i < (int)arr.size(); ++i)
		{
			expect(i) == arr[i];
		}
	};

	while(arr.size() < start_elements) arr.emplace_back(arr.size());

	section<"erase">() = [&] {
		while(arr.size() < 15) arr.emplace_back(arr.size());

		section<"remove one element at the start">() = [&] {
			auto original_size				 = arr.size();
			section<"allow_instability_t">() = [&] {
				arr.erase(allow_instability, arr.begin());
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i > 0) ? i : arr.size();
					expect(value) == arr[i];
				}
			};
			section<"keep_stability_t">() = [&] {
				arr.erase(keep_stability, arr.begin());
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					expect(i + 1) == arr[i];
				}
			};
			expect(original_size) == (arr.size() + 1);
		};

		section<"remove several elements at the start">() = [&] {
			auto original_size				 = arr.size();
			section<"allow_instability_t">() = [&] {
				arr.erase(allow_instability, arr.begin(), next(arr.begin(), 2));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i > 1) ? i : arr.size() + i;
					expect(value) == (int)arr[i];
				}
			};
			section<"keep_stability_t">() = [&] {
				arr.erase(keep_stability, arr.begin(), next(arr.begin(), 2));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					expect(i + 2) == arr[i];
				}
			};
			expect(original_size) == arr.size() + 2;
		};

		section<"remove one element in the middle">() = [&] {
			auto original_size				 = arr.size();
			section<"allow_instability_t">() = [&] {
				arr.erase(next(arr.begin()));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i != 1) ? i : arr.size() + i - 1;
					expect(value) == arr[i];
				}
			};
			section<"keep_stability_t">() = [&] {
				arr.erase(keep_stability, next(arr.begin()));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i >= 1) ? i + 1 : i;
					expect(value) == arr[i];
				}
			};
			expect(original_size) == arr.size() + 1;
		};

		section<"remove several elements in the middle">() = [&] {
			auto original_size				 = arr.size();
			section<"allow_instability_t">() = [&] {
				arr.erase(allow_instability, next(arr.begin()), next(arr.begin(), 3));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i == 0 || i > 2) ? i : arr.size() + i - 1;
					expect(value) == arr[i];
				}
			};
			section<"keep_stability_t">() = [&] {
				arr.erase(keep_stability, next(arr.begin()), next(arr.begin(), 3));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					expect value = (i > 0) ? i + 2 : i;
					expect(value) == arr[i];
				}
			};
			expect(original_size) == arr.size() + 2;
		};
	};

	section<"clear removes all elements">() = [&] {
		while(arr.size() < 15) arr.emplace_back(arr.size());
		expect(15u) <= arr.size();
		arr.clear();
		expect(0u) == arr.size();
	};

	section<"reserve increases capacity">() = [&] {
		auto start_size = arr.size();
		auto start_cap  = arr.capacity();

		section<"larger capacity increases">() = [&] {
			arr.reserve(start_cap * 2);
			expect(start_cap * 2) == arr.capacity();
			expect(arr.size()) == start_size;

			auto new_cap = arr.capacity();

			section<"shrinking changes nothing">() = [&] {
				arr.reserve(start_cap);
				expect(new_cap) == arr.capacity();
			};
		};

		section<"smaller capacity is a no-op">() = [&] {
			arr.reserve(start_cap - 1);
			expect(start_cap) == arr.capacity();
			expect(arr.size()) == start_size;
		};
	};
	section<"trim excess equalizes size and capacity (or sets capacity to sbo_size, if larger than size)">() = [&] {
		auto start_size		 = arr.size();
		auto lower_bound_cap = std::max(arr.size(), arr.sbo_size());
		auto upper_bound_cap = arr.size() + 100;
		arr.reserve(upper_bound_cap);
		expect(arr.size()) < arr.capacity();
		expect(arr.size()) == start_size;
		arr.trim_excess();
		expect(lower_bound_cap) == arr.capacity();
		expect(arr.size()) == start_size;
		arr.reserve(upper_bound_cap);
		expect(arr.size()) < arr.capacity();
		expect(arr.size()) == start_size;
		arr.shrink_to_fit();
		expect(lower_bound_cap) == arr.capacity();
		expect(arr.size()) == start_size;
	};
};

auto array_test2 =
	suite<"static_array", "psl", "psl::array", "containers">().templates<tpack<int>, vpack<8, 32, 512, 1024>>() =
		[]<typename T, typename V0>()
{
	using std::next, std::prev;
	using array_t = psl::array<T, V0::value>;

	static_assert(IsStaticArray<array_t>);
	array_t array{};
	constexpr auto max_size = array.max_size();
	// auto sbo_size			= array.sbo_size();

	expect(array.size()) < max_size;
	expect(array.max_size()) >= array.sbo_size();

	section<"fill">() = [&] {
		array.resize(max_size);
		expect(array.size() == max_size);
		expect(array.capacity() == max_size);

		section<"overallocate">() = [&] {
			expect([&] { array.resize(max_size + 1); }) == throws<>();
			expect([&] { array.emplace_back(); }) == throws<>();
		};

		section<"erase add 2 (overallocate)">() = [&] {
			array.erase(array.begin());
			expect([&] { array.resize(array.size() + 2); }) == throws<>();
			section<"fill again">() = [&] {
				array.emplace_back();
				expect(array.size()) == max_size;
				expect(array.capacity()) == max_size;
			};
		};
	};
};
