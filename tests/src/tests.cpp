
#include "gtest/gtest.h"

#include "psl/uid.h"
#include "psl/range.h"
#include "psl/allocator.h"
#include "psl/array.h"

#include <iostream>
#include "psl/thread.h"

int main(int argc, char** argv)
{
	psl::thread::NAME = u8"main";
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}