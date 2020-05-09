
#include "gtest/gtest.h"

#include "psl/uid.h"
#include "psl/range.h"
#include "psl/allocator.h"
#include "psl/array.h"

#include <iostream>
#include "psl/thread.h"
#include "psl/profiler.h"
#include "psl/sinks/stream.h"

int pow2(int x)
{
	psl::profile_scope block{};
	return x * x;
}
int sum(int x, int y)
{
	psl::profile_scope block{};
	return pow2(x) + y;
}

int mul(int x, int y)
{
	psl::profile_scope block{};
	return x * y;
}

class foo
{
  public:
	void set_sum(int x)
	{
		psl::profile_scope block{};
		sum = x;
	}

	int sum;
};

template <typename T>
concept IsA = true;

template <typename T>
concept IsB = true;

template <typename T>
requires IsA<T>&& IsB<T> class bar
{};

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	// auto size = psl::default_resource.size();
	psl::profile_scope block{};
	psl::thread::NAME = u8"main";
	psl::log.add<psl::ostream>(std::cout);
	foo f{};
	{
		psl::profile_scope block{};
		f.set_sum(mul(sum(sum(5, 10), 8), 4));
	}
	//::testing::InitGoogleTest(&argc, argv);
	// return RUN_ALL_TESTS();
}