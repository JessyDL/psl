
#include "gtest/gtest.h"
#include "psl/dequeue.h"
#include "psl/allocator.h"

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

template <typename T>
struct DequeueTest : public ::testing::Test
{
	psl::default_resource_t resource{alignof(char)};
	psl::default_allocator_t allocator{&resource};
	psl::dequeue<T> data{allocator};
};

template <typename T = int>
using dequeue = DequeueTest<T>;


using trivial_dequeue = dequeue<int>;
using complex_dequeue = dequeue<foo>;

using types = ::testing::Types<int, float, foo, size_t>;


TEST_F(trivial_dequeue, size)
{
	EXPECT_EQ(data.size(), 0);
	data.emplace_back(0);
	EXPECT_EQ(data.size(), 1);
	std::ignore = data.pop_front();
	EXPECT_EQ(data.size(), 0);
	data.emplace_back(0);
	data.emplace_back(0);
	EXPECT_EQ(data.size(), 2);
	std::ignore = data.pop_front();
	EXPECT_EQ(data.size(), 1);
}

TEST_F(trivial_dequeue, resize)
{
	data.resize(12);
	EXPECT_EQ(data.size(), 12);

	data.resize(4);
	EXPECT_EQ(data.size(), 4);
}

TEST_F(trivial_dequeue, reserve)
{
	data.reserve(16);
	EXPECT_EQ(data.capacity(), 16);
	EXPECT_EQ(data.size(), 0);

	data.reserve(4);
	EXPECT_EQ(data.capacity(), 16);
	EXPECT_EQ(data.size(), 0);
}

TEST_F(trivial_dequeue, clear)
{
	data.resize(12);
	EXPECT_EQ(data.size(), 12);
	data.clear();
	EXPECT_EQ(data.size(), 0);
}

TEST_F(trivial_dequeue, index_operator)
{
	data.emplace_back(0);
	EXPECT_EQ(data[0], 0);
	data.emplace_back(9);
	EXPECT_EQ(data[1], 9);
	data.emplace_back(6);
	EXPECT_EQ(data[2], 6);
	data.emplace_back(3);
	EXPECT_EQ(data[3], 3);
	data.emplace_back(5);
	EXPECT_EQ(data[4], 5);

	EXPECT_EQ(data[0], 0);
	EXPECT_EQ(data[1], 9);
	EXPECT_EQ(data[2], 6);
	EXPECT_EQ(data[3], 3);
	EXPECT_EQ(data[4], 5);
	std::ignore = data.pop_front();
	std::ignore = data.pop_back();
	EXPECT_EQ(data[0], 9);
	EXPECT_EQ(data[1], 6);
	EXPECT_EQ(data[2], 3);
}

TEST_F(trivial_dequeue, emplace_back)
{
	std::vector<int> expected{};
	for(size_t i = 0; i < 1024; ++i)
	{
		ASSERT_EQ(data.size(), i);
		auto value = std::rand() % 1024;
		data.emplace_back(value);
		expected.emplace_back(value);

		for(size_t ip = 0; ip < expected.size(); ++ip)
			ASSERT_EQ(expected[ip], data[ip]) << "at iteration " << i << " index " << ip << " when doing emplace_back";
	}

	{
		auto i = 0;
		for(auto entry : data)
		{
			ASSERT_EQ(entry, expected[i]) << "at iteration " << i;
			++i;
		}
		ASSERT_EQ(i, data.size());
	}
}

TEST_F(trivial_dequeue, emplace_front)
{
	std::vector<int> expected{};
	for(size_t i = 0; i < 1024; ++i)
	{
		ASSERT_EQ(data.size(), i);
		auto value = std::rand() % 1024;
		data.emplace_front(value);
		expected.insert(std::begin(expected), value);

		for(size_t ip = 0; ip < expected.size(); ++ip)
			ASSERT_EQ(expected[ip], data[ip]) << "at iteration " << i << " index " << ip << " when doing emplace_back";
	}

	{
		auto i = 0;
		for(auto entry : data)
		{
			ASSERT_EQ(entry, expected[i]) << "at iteration " << i;
			++i;
		}
		ASSERT_EQ(i, data.size());
	}
}


TEST_F(trivial_dequeue, emplace_back_and_front)
{
	std::vector<int> expected{};
	for(size_t i = 0; i < 1024; ++i)
	{
		ASSERT_EQ(data.size(), i);
		auto value		 = std::rand() % 1024;
		bool insert_back = std::rand() % 2 == 0;
		if(insert_back)
		{
			data.emplace_back(value);
			expected.emplace_back(value);
		}
		else
		{
			expected.insert(std::begin(expected), value);
			data.emplace_front(value);
		}

		for(size_t ip = 0; ip < expected.size(); ++ip)
			ASSERT_EQ(expected[ip], data[ip]) << "at iteration " << i << " index " << ip << " when doing "
											  << ((insert_back) ? "emplace_back" : "emplace_front");
	}

	{
		auto i = 0;
		for(auto entry : data)
		{
			ASSERT_EQ(entry, expected[i]) << "at iteration " << i;
			++i;
		}
		ASSERT_EQ(i, data.size());
	}
}

TEST_F(trivial_dequeue, constructors)
{
	std::array values{16, 8, 12, 99};
	std::array values2{4521, 4548, 1200, 99128, 64, 332, 478};
	for(auto v : values) data.emplace_back(v);

	auto copy{data};
	ASSERT_EQ(copy.size(), data.size());
	for(size_t i = 0; i < values.size(); ++i)
	{
		ASSERT_EQ(values[i], copy[i]) << "copy constructor failed";
	}

	data.clear();
	for(auto v : values2) data.emplace_back(v);
	copy = data;
	ASSERT_EQ(copy.size(), values2.size());
	for(size_t i = 0; i < values2.size(); ++i)
	{
		ASSERT_EQ(values2[i], copy[i]) << "copy assignment failed";
	}

	auto move{std::move(copy)};
	ASSERT_EQ(move.size(), values2.size());
	for(size_t i = 0; i < values2.size(); ++i)
	{
		ASSERT_EQ(values2[i], move[i]) << "move constructor failed";
	}

	copy = data;
	move = std::move(copy);
	ASSERT_EQ(move.size(), values2.size());
	for(size_t i = 0; i < values2.size(); ++i)
	{
		ASSERT_EQ(values2[i], move[i]) << "move assignment failed";
	}

	psl::test<int>();
}