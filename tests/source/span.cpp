#include <catch2/catch_template_test_macros.hpp>
#include <psl/span.hpp>
#include <psl/algorithms.hpp>

using static_span		   = psl::span<int, 5>;
using dynamic_span		   = psl::span<int>;
using static_strided_span  = psl::span<int, 5, 3>;
using dynamic_strided_span = psl::span<int, psl::dynamic_extent, 3>;

using namespace psl;

template <typename T>
constexpr auto init(int* data, [[maybe_unused]] auto max_elements)
{
	if constexpr(is_static_span_v<T>)
		return T{data};
	else
		return T{data, max_elements};
}

TEMPLATE_TEST_CASE("span suite", "[psl::span]", static_span, dynamic_span, static_strided_span, dynamic_strided_span)
{
	using T						= TestType;
	int data[15]				= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
	constexpr auto data_size	= sizeof(data) / sizeof(data[0]);
	constexpr auto max_elements = data_size / T::abs_stride();
	T span						= init<T>(data, max_elements);

	for(auto data_i = 0u, span_i = 0u; span_i < span.size(); data_i += span.stride(), ++span_i)
	{
		REQUIRE(data[data_i] == span[span_i]);
		//	<< "with data index: " << data_i << " and span index: " << span_i << " and stride: " << span.stride();
	}

	for(auto i = 0u; auto value : span)
	{
		REQUIRE(data[i] == value); // << "failed forwards iterator at index: " << i;
		i += span.stride();
	}

	for(auto i = (span.size() - 1) * span.stride(); auto value : reverse(span))
	{
		REQUIRE(data[i] == value); // << "failed reverse iterator at index: " << i;
		i -= span.stride();
	}
}