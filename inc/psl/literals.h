#pragma once
#include <cstdint>

namespace psl::literals
{
	constexpr size_t operator"" _sz(unsigned long long int x) { return x; }
} // namespace psl::literals