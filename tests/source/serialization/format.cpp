#include <psl/serialization/format.hpp>

#include <litmus/suite.hpp>
#include <litmus/section.hpp>
#include <litmus/expect.hpp>

#include "../data/simple.pff"
//#include "../data/structured.pff"

using namespace psl;
using namespace psl::serialization;

using namespace litmus;

auto format_test0 = suite<"format", "psl", "psl::serialization">() = [] { psl::serialization::format format{SIMPLE}; };
