#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <psl/array.hpp>
#include <tests/types.hpp>

using namespace psl;

TEST_CASE("internal storage constraints", "[psl::sbo_storage]")
{
	int results[]{
		sbo_storage<int, 4, sbo_alias<true>>::SBO,	   sbo_storage<int, 4, sbo_alias<false>>::SBO,
		sbo_storage<int, 0, sbo_alias<true>>::SBO,	   sbo_storage<int, 0, sbo_alias<false>>::SBO,
		sbo_storage<int, 1, sbo_alias<true>>::SBO,	   sbo_storage<int, 1, sbo_alias<false>>::SBO,
		sbo_storage<int8_t, 12, sbo_alias<true>>::SBO, sbo_storage<int8_t, 12, sbo_alias<false>>::SBO,
		sbo_storage<int8_t, 0, sbo_alias<true>>::SBO,  sbo_storage<int8_t, 0, sbo_alias<false>>::SBO,
		sbo_storage<int8_t, 1, sbo_alias<true>>::SBO,  sbo_storage<int8_t, 1, sbo_alias<false>>::SBO,
	};
	REQUIRE(results[0] == 4);
	REQUIRE(results[1] == 2);
	REQUIRE(results[2] == 0);
	REQUIRE(results[3] == 0);
	REQUIRE(results[4] == 1);
	REQUIRE(results[5] == 0);
	REQUIRE(results[6] == 12);
	REQUIRE(results[7] == 4);
	REQUIRE(results[8] == 0);
	REQUIRE(results[9] == 0);
	REQUIRE(results[10] == 1);
	REQUIRE(results[11] == 0);
}

TEMPLATE_TEST_CASE("generic array suite", "[psl::array][containers]", (psl::array<int>), (psl::array<int, 512>))
{
	using std::next, std::prev;
	using array_t = TestType;
	using value_t = TestType::value_type;
	array_t arr{};
	REQUIRE(arr.size() == 0);
	REQUIRE((arr.size() <= arr.capacity()));
	REQUIRE((arr.is_stored_inlined() || arr.sbo_size() == 0));
	STATIC_REQUIRE(is_array_v<const array_t>);
	STATIC_REQUIRE(IsArray<array_t>);

	SECTION("emplace")
	{
		for(int i = 0; i < (int)arr.sbo_size(); ++i)
		{
			arr.emplace_back(i);
		}
		REQUIRE((arr.is_stored_inlined() || arr.sbo_size() == 0));
		REQUIRE(arr.size() == arr.sbo_size());
		SECTION("force external storage")
		{
			arr.emplace_back(arr.size());
			REQUIRE_FALSE(arr.is_stored_inlined());
			REQUIRE((arr.size() > arr.sbo_size()));
		}

		for(int i = 0; i < (int)arr.size(); ++i)
		{
			REQUIRE(i == arr[i]);
		}
	}

	size_t start_elements = GENERATE(0, 8, 128);
	while(arr.size() < start_elements) arr.emplace_back(arr.size());

	SECTION("erase")
	{
		while(arr.size() < 15) arr.emplace_back(arr.size());

		SECTION("remove one element at the start")
		{
			auto original_size = arr.size();
			SECTION("allow_instability_t")
			{
				arr.erase(allow_instability, arr.begin());
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i > 0) ? i : arr.size();
					REQUIRE((value == arr[i]));
				}
			}
			SECTION("keep_stability_t")
			{
				arr.erase(keep_stability, arr.begin());
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					REQUIRE(i + 1 == arr[i]);
				}
			}
			REQUIRE(original_size == arr.size() + 1);
		}

		SECTION("remove several elements at the start")
		{
			auto original_size = arr.size();
			SECTION("allow_instability_t")
			{
				arr.erase(allow_instability, arr.begin(), next(arr.begin(), 2));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i > 1) ? i : arr.size() + i;
					REQUIRE((value == (int)arr[i]));
				}
			}
			SECTION("keep_stability_t")
			{
				arr.erase(keep_stability, arr.begin(), next(arr.begin(), 2));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					REQUIRE(i + 2 == arr[i]);
				}
			}
			REQUIRE(original_size == arr.size() + 2);
		}

		SECTION("remove one element in the middle")
		{
			auto original_size = arr.size();
			SECTION("allow_instability_t")
			{
				arr.erase(next(arr.begin()));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i != 1) ? i : arr.size() + i - 1;
					REQUIRE((value == arr[i]));
				}
			}
			SECTION("keep_stability_t")
			{
				arr.erase(keep_stability, next(arr.begin()));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i >= 1) ? i + 1 : i;
					REQUIRE(value == arr[i]);
				}
			}
			REQUIRE(original_size == arr.size() + 1);
		}

		SECTION("remove several elements in the middle")
		{
			auto original_size = arr.size();
			SECTION("allow_instability_t")
			{
				arr.erase(allow_instability, next(arr.begin()), next(arr.begin(), 3));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i == 0 || i > 2) ? i : arr.size() + i - 1;
					REQUIRE((value == arr[i]));
				}
			}
			SECTION("keep_stability_t")
			{
				arr.erase(keep_stability, next(arr.begin()), next(arr.begin(), 3));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					value_t value = (i > 0) ? i + 2 : i;
					REQUIRE((value == arr[i]));
				}
			}
			REQUIRE(original_size == arr.size() + 2);
		}
	}

	SECTION("clear removes all elements")
	{
		while(arr.size() < 15) arr.emplace_back(arr.size());
		REQUIRE(15 <= arr.size());
		arr.clear();
		REQUIRE(0 == arr.size());
	}

	SECTION("reserve increases capacity")
	{
		auto start_size = arr.size();
		auto start_cap	= arr.capacity();

		SECTION("larger capacity increases")
		{
			arr.reserve(start_cap * 2);
			REQUIRE(start_cap * 2 == arr.capacity());
			REQUIRE(arr.size() == start_size);

			auto new_cap = arr.capacity();

			SECTION("shrinking changes nothing")
			{
				arr.reserve(start_cap);
				REQUIRE(new_cap == arr.capacity());
			}
		}

		SECTION("smaller capacity is a no-op")
		{
			arr.reserve(start_cap - 1);
			REQUIRE(start_cap == arr.capacity());
			REQUIRE(arr.size() == start_size);
		}
	}

	SECTION("trim excess equalizes size and capacity (or sets capacity to sbo_size, if larger than size)")
	{
		auto start_size		 = arr.size();
		auto lower_bound_cap = std::max(arr.size(), arr.sbo_size());
		auto upper_bound_cap = arr.size() + 100;
		arr.reserve(upper_bound_cap);
		REQUIRE(arr.size() < arr.capacity());
		REQUIRE(arr.size() == start_size);
		arr.trim_excess();
		REQUIRE(lower_bound_cap == arr.capacity());
		REQUIRE(arr.size() == start_size);
		arr.reserve(upper_bound_cap);
		REQUIRE(arr.size() < arr.capacity());
		REQUIRE(arr.size() == start_size);
		arr.shrink_to_fit();
		REQUIRE(lower_bound_cap == arr.capacity());
		REQUIRE(arr.size() == start_size);
	}
}

TEMPLATE_TEST_CASE("static array suite", "[psl::array][containers]", (psl::array<int, 8>), (psl::array<int, 32>),
				   (psl::array<int, 1024>))
{
	using array_t = TestType;
	// using value_t = TestType::value_type;

	static_assert(IsStaticArray<array_t>);
	array_t array{};
	constexpr auto max_size = array.max_size();
	// auto sbo_size			= array.sbo_size();

	REQUIRE(array.size() < max_size);
	REQUIRE(array.max_size() >= array.sbo_size());

	SECTION("fill")
	{
		array.resize(max_size);
		REQUIRE(array.size() == max_size);
		REQUIRE(array.capacity() == max_size);

		SECTION("overallocate")
		{
			REQUIRE_THROWS(array.resize(max_size + 1));
			REQUIRE_THROWS(array.emplace_back());
		}

		SECTION("erase add 2 (overallocate)")
		{
			array.erase(array.begin());
			REQUIRE_THROWS(array.resize(array.size() + 2));
			SECTION("fill again")
			{
				array.emplace_back();
				REQUIRE(array.size() == max_size);
				REQUIRE(array.capacity() == max_size);
			}
		}
	}
}