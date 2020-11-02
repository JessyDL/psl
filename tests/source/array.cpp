#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
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

TEMPLATE_TEST_CASE("array suite", "[psl::array][containers]", int)
{
	using std::next, std::prev;
	psl::array<TestType> arr{};
	REQUIRE(arr.size() == 0);
	REQUIRE((arr.size() <= arr.capacity()));
	REQUIRE((arr.is_stored_inlined() || arr.sbo_size() == 0));
	STATIC_REQUIRE(is_array_v<const array<TestType>>);
	STATIC_REQUIRE(IsArray<array<TestType>>);

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
					TestType value = (i > 0) ? i : arr.size();
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
					TestType value = (i > 1) ? i : arr.size() + i;
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
					TestType value = (i != 1) ? i : arr.size() + i - 1;
					REQUIRE((value == arr[i]));
				}
			}
			SECTION("keep_stability_t")
			{
				arr.erase(keep_stability, next(arr.begin()));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					TestType value = (i >= 1) ? i + 1 : i;
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
					TestType value = (i == 0 || i > 2) ? i : arr.size() + i - 1;
					REQUIRE((value == arr[i]));
				}
			}
			SECTION("keep_stability_t")
			{
				arr.erase(keep_stability, next(arr.begin()), next(arr.begin(), 3));
				for(int i = 0; i < (int)arr.size(); ++i)
				{
					TestType value = (i > 0) ? i + 2 : i;
					REQUIRE((value == arr[i]));
				}
			}
			REQUIRE(original_size == arr.size() + 2);
		}
	}
}