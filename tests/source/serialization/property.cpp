#include <psl/serialization/property.hpp>

#include <litmus/suite.hpp>
#include <litmus/section.hpp>
#include <litmus/expect.hpp>

#include <psl/serialization/serializer.hpp>
#include <psl/serialization/versioning.hpp>

using namespace psl;
using namespace psl::serialization;

using namespace litmus;

auto sproperty_test0 = suite<"property", "psl", "psl::serialization">()
						   .templates<tpack<anonymous_property<int>, property<"T", int>>,
									  tpack<int, anonymous_property<int>, property<"Y", int>>>() =
	[]<typename T, typename Y>()
{
	Y initial_value{10};
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

	section<"assignment">() = [] {
		T prop_a{};
		Y prop_b{};

		auto result = prop_a = prop_b;
		expect(std::is_same_v<std::remove_cvref_t<decltype(result)>, T>) == true;
	};

	static_assert(std::is_same_v<decltype(static_cast<int>(prop)), int>, "expected it to be castable to int");
};


struct foo
{
	constexpr auto properties() const noexcept { return make_properties(name, health, is_hero); }
	property<"name", std::string> name{"Dusan"};
	property<"health", int> health{100};
	property<"is_hero", bool> is_hero{true};
};

struct foo_container
{
	constexpr auto properties() const noexcept { return make_properties(foos, id, is_active); }
	property<"foos", std::vector<foo>> foos{};
	property<"id", int> id{100};
	property<"is_active", bool> is_active{false};
};

inline std::string to_string(const std::string& value) noexcept { return value; }
inline std::string to_string(const auto& value) noexcept
{
	using type = std::remove_cvref_t<decltype(value)>;
	if constexpr(std::is_same_v<bool, type>)
	{
		return (value) ? std::string("true") : std::string("false");
	}
	else
		return std::to_string(value);
}

#include <iostream>
#include <stack>
struct cout_json_writer
{
	void object_begin(std::string_view name = "")
	{
		if(!m_FirstValue) std::cout << ",";
		m_FirstValue = false;
		std::cout << '\n' << std::string(m_Depth * 2, ' ');
		if(!name.empty()) std::cout << '"' << name << "\":";
		std::cout << "{";
		++m_Depth;
		m_FirstValue = true;
		m_InArray.push(false);
	}

	void object_end([[maybe_unused]] std::string_view name = "")
	{
		;
		--m_Depth;
		std::cout << '\n' << std::string(m_Depth * 2, ' ') << "}";
		m_FirstValue = false;
		m_InArray.pop();
	}

	void value(std::string_view name, const auto& value)
	{
		;
		using type = std::remove_cvref_t<decltype(value)>;
		if(!m_FirstValue) std::cout << ",";
		m_FirstValue = false;
		std::cout << '\n' << std::string(m_Depth * 2, ' ');

		auto value_str = to_string(value);

		if constexpr(std::is_integral_v<type> || std::is_same_v<bool, type>)
		{
		}
		else
			value_str = "\"" + value_str + "\"";

		if(m_InArray.top())
		{
			std::cout << "{ " << value_str << " }";
		}
		else
		{
			std::cout << '"' << name << "\": " << value_str;
		}
	}

	void range_begin(std::string_view name)
	{
		if(!m_FirstValue) std::cout << ",";
		m_InArray.push(true);

		std::cout << '\n' << std::string(m_Depth * 2, ' ') << '"' << name << "\": [";
		m_FirstValue = true;
		++m_Depth;
		;
	}

	void range_end([[maybe_unused]] std::string_view name)
	{
		m_FirstValue = false;
		m_InArray.pop();
		--m_Depth;
		std::cout << '\n' << std::string(m_Depth * 2, ' ') << "]";
	};
	size_t m_Depth{0u};
	bool m_FirstValue{true};
	std::stack<bool> m_InArray{};
};


struct reader
{
	template <typename T>
	auto read(std::string_view name) -> T
	{
		return T{};
	}
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

	cout_json_writer writer;
	serialize(writer, f);
	std::cout << std::endl;

	// reader reader{};
	// deserialize<foo>(reader);
};

struct bar;

namespace psl::serialization
{
	template <>
	struct version_settings_default<bar>
	{
		static constexpr version_type_t value{1u};
	};

	template <>
	struct version<bar, 0>
	{
		property<"health", int> health{};
		property<"damage_multiplier", int> damage_multiplier{};
	};

	template <>
	struct version<bar, 1>
	{
		version() = default;
		version(const version<bar, 0>& other) : health(other.health) {}
		property<"health", int> health{};
		property<"name", std::string> name{};
	};

	template <>
	struct version<bar, 2>
	{
		version() = default;
		version(const version<bar, 0>& other) : health(other.health) {}
		version(const version<bar, 1>& other) : health(other.health) {}

		property<"health", int> health{};
	};

} // namespace psl::serialization

struct bar : public version<bar>
{};

auto versioning_test0 = suite<"versioning", "psl", "psl::serialization">() = [] {
	bar b{};
	b.name = "hello";
};
