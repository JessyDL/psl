#include "psl/array.h"
#include "gtest/gtest.h"
#include "psl/string.h"
#include "psl/iterators.h"
#include <ranges>

struct counter
{
	int value;
};

struct foo
{
	foo(counter& counter) : c(&counter) { c->value += 1; }
	foo(foo&& other) noexcept = default;
	foo& operator=(foo&& other) noexcept = default;
	~foo()
	{
		c->value -= 1;
		assert(c->value >= 0);
	}

	counter* c;
};


static_assert(std::contiguous_iterator<psl::array<int>::iterator>);

template <typename T>
class ArrayTest : public ::testing::Test
{
  public:
	psl::default_resource_t resource{alignof(char)};
	psl::allocator allocator{&resource};
	psl::array<T> data{allocator};
};

template <typename T = int>
using array = ArrayTest<T>;


using trivial_array = ArrayTest<int>;
using complex_array = ArrayTest<foo>;

using types = ::testing::Types<int, float, foo, size_t, psl::string>;

TYPED_TEST_SUITE(array, types);

TYPED_TEST(array, empty) { EXPECT_TRUE(this->data.empty()); }


TYPED_TEST(array, size) { EXPECT_EQ(this->data.size(), 0); }

TEST_F(trivial_array, emplace)
{
	auto& res0 = data.emplace_back(0);
	EXPECT_EQ(res0, 0);
	EXPECT_EQ(data.size(), 1);

	for(auto i = 1; i < 32; ++i)
	{
		data.emplace_back(i);
	}
	EXPECT_EQ(data.size(), 32);
	for(auto i = 0; i < 32; ++i)
	{
		EXPECT_EQ(data[i], i);
	}
}

TEST_F(trivial_array, resize)
{
	data.resize(10);
	EXPECT_EQ(data.size(), 10);
}

TEST_F(trivial_array, index_operator)
{
	EXPECT_THROW(data[0], std::out_of_range);
	data.emplace_back(1);
	data.emplace_back(2);
	EXPECT_EQ(data[1], 2);
}

TEST_F(complex_array, erase)
{
	counter c{0};
	for(auto i = 0; i < 1024; ++i)
	{
		data.emplace_back(c);
	}

	data.erase(512, 512);
	EXPECT_EQ(c.value, 512);
	data.erase(256, 256);
	EXPECT_EQ(c.value, 256);
	data.erase(0, 256);
	EXPECT_EQ(c.value, 0);
}

TEST_F(trivial_array, generic_functions)
{
	data.emplace_back(1);
	data.emplace_back(2);
	data.emplace_back(3);
	data.emplace_back(4);

	std::vector<int> vec_data{{1, 2, 3, 4}};

	std::rotate(data.begin(), data.begin() + 3, data.end());
	std::rotate(vec_data.begin(), vec_data.begin() + 3, vec_data.end());

	for(size_t i = 0; i < vec_data.size(); ++i) EXPECT_EQ(data[i], vec_data[i]);
}