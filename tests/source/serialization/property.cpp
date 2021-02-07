#include <psl/serialization/property.hpp>

#include <litmus/suite.hpp>
#include <litmus/section.hpp>
#include <litmus/expect.hpp>

using namespace psl;
using namespace psl::serialization;

using namespace litmus;

auto sproperty_test0 = suite<"property", "psl", "psl::serialization">()
						   .templates<tpack<anonymous_property<int>, property<"T", int>>,
									  tpack<int, anonymous_property<int>, property<"Y", int>>>() =
	[]<typename T, typename Y>()
{
	constexpr Y initial_value{10};
	T prop{initial_value};

	section<"operators">() = [&] {
		section<"comparison">() = [&] {
			expect(prop) == initial_value;
			expect(prop) != (initial_value - 1);
			expect(prop) != (initial_value + 1);
			expect(prop) <= initial_value;
			expect(prop) <= (initial_value + 1);
			expect(prop) >= initial_value;
			expect(prop) >= (initial_value - 1);
			expect(prop) < (initial_value + 1);
			expect(prop) > (initial_value - 1);
		};

		section<"pre increments">() = [&] {
			expect(++prop) == (initial_value + 1);
			expect(prop) == (initial_value + 1);
		};
		section<"post increments">() = [&] {
			expect(prop++) == initial_value;
			expect(prop) == (initial_value + 1);
		};
		section<"pre decrements">() = [&] {
			expect(--prop) == (initial_value - 1);
			expect(prop) == (initial_value - 1);
		};
		section<"post decrements">() = [&] {
			expect(prop--) == initial_value;
			expect(prop) == (initial_value - 1);
		};
		section<"arithmetic">() = [&] {
			section<"addition">() = [&] {
				expect(prop + 5) == (initial_value + 5);
				expect(prop) == initial_value;
				expect(prop += 5) == (initial_value + 5);
				expect(prop) == (initial_value + 5);
			};
			section<"subtraction">() = [&] {
				expect(prop - 5) == (initial_value - 5);
				expect(prop) == initial_value;
				expect(prop -= 5) == (initial_value - 5);
				expect(prop) == (initial_value - 5);
			};
			section<"multiply">() = [&] {
				expect(prop * 5) == (initial_value * 5);
				expect(prop) == initial_value;
				expect(prop *= 5) == (initial_value * 5);
				expect(prop) == (initial_value * 5);
			};
			section<"division">() = [&] {
				expect(prop / 5) == (initial_value / 5);
				expect(prop) == initial_value;
				expect(prop /= 5) == (initial_value / 5);
				expect(prop) == (initial_value / 5);
			};
			section<"modulus">() = [&] {
				expect(prop % 5) == (initial_value % 5);
				expect(prop) == initial_value;
				expect(prop % 3) == (initial_value % 3);
				expect(prop) == initial_value;
				expect(prop %= 5) == (initial_value % 5);
				expect(prop) == (initial_value % 5);
			};
		};
	};

	static_assert(std::is_same_v<decltype(static_cast<int>(prop)), int>, "expected it to be castable to int");
};


struct foo
{
	property<"name", std::string> name{"Dusan"};
	property<"health", int> health{100};
};

struct foo_container
{
	property<"foos", std::vector<foo>> foos{};
};


auto sproperty_test1 = suite<"property-struct", "psl", "psl::serialization">() = []() {
	static_assert(sizeof(int) == sizeof(property<"name", int>), "size is expected to be the same");

	foo_container f{};
	f.foos->resize(10);
	f.foos[2].name = "Susan";

	expect(f.foos[0].name) == "Dusan";
	expect(f.foos[2].name) == "Susan";


	struct fake_property
	{
		using underlying_t = void;
	};

	struct real_property : public anonymous_property<int>
	{
		using underlying_t = int;
	};

	static_assert(IsProperty<property<"hello", int>>);
	static_assert(!IsProperty<fake_property>);
	static_assert(IsProperty<real_property>);
};
