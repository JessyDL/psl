#include <psl/containers/string_view.hpp>

#include <litmus/suite.hpp>
#include <litmus/section.hpp>
#include <litmus/expect.hpp>
#include <litmus/generator/range.hpp>


using namespace psl;
using namespace litmus;

auto string_view_test0 = suite<"string_view", "psl", "psl::containers">() = [] {
	std::array<const char*, 2> data{"", "some text"};

	for(const auto& value : data)
	{
		string_view sv{value};
		expect(sv.size() == strlen(value));
	}
};
