#include <psl/iterators.hpp>
#include <psl/types.hpp>

#include <litmus/expect.hpp>
#include <litmus/section.hpp>
#include <litmus/suite.hpp>

namespace litmus {
template <typename T, auto Value>
struct value_to_string_t<psl::contiguous_range_iterator<T, Value>> {
	using type = psl::contiguous_range_iterator<T, Value>;
	constexpr std::string operator()(type const& value) const noexcept { return std::to_string(*value); };
};
}	 // namespace litmus


using namespace psl;
using namespace litmus;


void test(auto begin, auto end) {
	expect(begin.is_valid_pair(end)) == true;
	auto stride	 = begin.stride();
	auto data_it = begin.ptr();
	for(auto it = begin; it != end; ++it, data_it += stride) {
		expect(*it) == *data_it;	// << "data is invalid";
	}
	data_it = begin.ptr();
	for(auto i = 0u; begin + i != end; ++i, data_it += stride) {
		expect(begin[i]) == *data_it;	 // << "data is invalid";
	}

	// comparison operators
	expect(begin) != end;

	expect(begin) >= begin;
	expect(end) >= begin;

	expect(begin) <= end;
	expect(end) <= end;

	expect(begin) < end;
	expect(end) > begin;

	// arithmetic
	{
		auto it	 = begin++;
		auto it2 = begin;
		expect(it) == ++it2;
		expect(begin) == --it2;
		expect(it) == it2++;
		expect(begin) == it2;
		expect(it--) == begin;
	}

	// expect(end - begin) == data.size();
}

template <typename T>
constexpr static T data[] {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

auto iterators_test0 = suite<"contiguous_range_iterator", "psl">().templates<tpack<int>, vpack<1, -1, 3, -3>>() =
  []<typename T, typename V0>() {
	  constexpr size_t data_size = sizeof(data<T>) / sizeof(data<T>[0]);
	  using iterator			 = psl::contiguous_range_iterator<T, V0::value>;

	  constexpr auto max_elements = (data_size - (data_size % iterator::abs_stride())) / iterator::abs_stride();
	  bool forwards				  = iterator::stride() > 0;

	  iterator begin {const_cast<T*>((forwards) ? &data<T>[0] : &data<T>[0] + iterator::abs_stride() * max_elements)};
	  iterator end {const_cast<T*>((!forwards) ? &data<T>[0] : &data<T>[0] + iterator::abs_stride() * max_elements)};
	  test(begin, end);

	  expect(begin + max_elements) == end;
	  expect(end - begin) == (ssize_t)max_elements;

	  if(iterator::abs_stride() > 1) {
		  expect(iterator {const_cast<T*>(&data<T>[1])}.is_valid_pair(end)) == false;
	  } else {
		  expect(iterator {const_cast<T*>(&data<T>[1])}.is_valid_pair(end)) == true;
	  }
  };
