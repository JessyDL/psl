#pragma once
#include <thread>
#include "psl/string.h"

namespace psl::thread
{
	static thread_local psl::string NAME{psl::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()))};
} // namespace psl::thread