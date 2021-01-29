#include <litmus/suite.hpp>
#include <litmus/section.hpp>
#include <litmus/expect.hpp>

#include <psl/span.hpp>
#include <psl/algorithms.hpp>


using namespace psl;
using namespace litmus;

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
	suite<"span suite", "psl", "psl::span">().templates<tpack<int>, vpack<5, psl::dynamic_extent>, vpack<1, 3, 5>>() =
		[]<typename T0, typename V0, typename V1>()
{
	using T						= span<T0, V0::value, V1::value>;
	constexpr auto data_size	= sizeof(data<T0>) / sizeof(data<T0>[0]);
	constexpr auto max_elements = data_size / T::abs_stride();
	T span						= init<T>(const_cast<T0*>(data<T0>), max_elements);

	section<"for-loop">() = [&] {
		for(auto data_i = 0u, span_i = 0u; span_i < span.size(); data_i += span.stride(), ++span_i)
		{
			expect(data<T0>[data_i]) == span[span_i];
		}
	};
	section<"range-for-loop">() = [&] {
		for(auto i = 0u; auto value : span)
		{
			expect(data<T0>[i]) == value; // << "failed forwards iterator at index: " << i;
			i += span.stride();
		}
	};

	section<"reverse-for-loop">() = [&] {
		for(auto i = (span.size() - 1) * span.stride(); auto value : reverse(span))
		{
			expect(data<T0>[i]) == value; // << "failed reverse iterator at index: " << i;
			i -= span.stride();
		}
	};
};