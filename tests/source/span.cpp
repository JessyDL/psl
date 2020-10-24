#include <gtest/gtest.h>
#include <psl/span.hpp>
#include <psl/algorithms.hpp>

using static_span		   = psl::span<int, 5>;
using dynamic_span		   = psl::span<int>;
using static_strided_span  = psl::span<int, 5, 3>;
using dynamic_strided_span = psl::span<int, psl::dynamic_extent, 3>;

int DATA[15] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

void test(const auto& span)
{
	for(auto data_i = 0u, span_i = 0u; span_i < span.size(); data_i += span.stride(), ++span_i)
	{
		ASSERT_EQ(DATA[data_i], span[span_i])
			<< "with data index: " << data_i << " and span index: " << span_i << " and stride: " << span.stride();
	}

	for(auto i = 0u; auto value : span)
	{
		ASSERT_EQ(DATA[i], value) << "failed forwards iterator at index: " << i;
		i += span.stride();
	}

	for(auto i = (span.size() - 1) * span.stride(); auto value : psl::reverse(span))
	{
		ASSERT_EQ(DATA[i], value) << "failed reverse iterator at index: " << i;
		i -= span.stride();
	}
}

TEST(static_span, operations) { test(static_span{DATA}); }
TEST(dynamic_span, operations) { test(dynamic_span{DATA, 5}); }
TEST(static_strided_span, operations) { test(static_strided_span{DATA}); }
TEST(dynamic_strided_span, operations) { test(dynamic_strided_span{DATA, 5}); }