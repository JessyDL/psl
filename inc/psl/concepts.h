#pragma once

namespace psl
{
	template <typename T>
	concept IsRange = requires(T t)
	{
		t.begin());
		t.end());
	};
} // namespace psl