#include <litmus/suite.hpp>
#include <litmus/section.hpp>
#include <litmus/expect.hpp>

#include <psl/span.hpp>
#include <psl/algorithms.hpp>


using namespace psl;
using namespace litmus;

#if defined(__GNUC__) || defined(__GNUG__)
// issue: GCC incorrectly detects out-of-bounds access of the _end_ iterator in a range-for for the reverse(span)
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
template <typename T>
constexpr auto init(auto* data, [[maybe_unused]] auto max_elements)
{
	if constexpr(is_static_span_v<T>)
		return T{data};
	else
		return T{data, max_elements};
}

template <typename T>
static const T data[15]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

auto span_test0 =
	suite<"span", "psl", "psl::span">().templates<tpack<int>, vpack<5, psl::dynamic_extent>, vpack<1, 3, 2>>() =
		[]<typename T0, typename V0, typename V1>()
{
	using T						= span<T0, V0::value, V1::value>;
	constexpr auto data_size	= sizeof(data<T0>) / sizeof(data<T0>[0]);
	constexpr auto max_elements = data_size / T::abs_stride();
	T span						= init<T>(const_cast<T0*>(data<T0>), max_elements);

	section<"for-loop">() = [&] {
		i64 data_i = 0;
		for(auto span_i = 0u; span_i < span.size(); data_i += span.stride(), ++span_i)
		{
			expect(data_i) >= 0u;
			expect(data_i) < 15u;
			expect(data<T0>[data_i]) == span[span_i];
		}
	};
	section<"range-for-loop">() = [&] {
		for(i64 i = 0; auto value : span)
		{
			expect(i) >= 0;
			expect(i) < 15;
			expect(data<T0>[(size_t)i]) == value; // << "failed forwards iterator at index: " << i;
			i += span.stride();
		}
	};

	section<"reverse-for-loop">() = [&] {
		auto rev_span = reverse(span);
		for(auto i = (span.size()) * span.abs_stride(); auto value : rev_span)
		{
			i -= span.stride();
			expect(i) >= 0u;
			expect(i) < 15u;
			expect(data<T0>[i]) == value; // << "failed reverse iterator at index: " << i;
		}
	};
};
