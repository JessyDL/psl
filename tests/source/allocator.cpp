#include <array>
#include <psl/allocator.hpp>
#include <psl/span.hpp>

#include <litmus/expect.hpp>
#include <litmus/section.hpp>
#include <litmus/suite.hpp>

using namespace psl;

using namespace litmus;

auto allocator_test0 =
  suite<"allocator", "psl">().templates<tpack<int>, tpack<config::default_allocator_t>, tpack<new_resource>>() =
	[]<typename T, typename Allocator, typename MemoryResource>() {
		MemoryResource* memory_resource = new MemoryResource(alignof(int));
		Allocator allocator {memory_resource};

		expect(sizeof(Allocator)) == sizeof(MemoryResource*);
		auto intAlloc = construct<T>(allocator, 5);

		expect(*intAlloc.data) == T {5};

		destroy(allocator, *intAlloc.data);

		auto range = construct_n<T>(allocator, T {5}, 0);
		psl::span<T, 5> range_span {range.data};

		for(T i = T {0}; auto& it : range_span) {
			it = ++i;
			expect(it) == i;
		}
		for(T i = T {0}; auto& it : range_span) {
			++i;
			expect(it) == i;
		}
	};
