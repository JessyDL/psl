#pragma once
#include "psl/details/fixed_string.h"
#include <span>

namespace psl
{
	namespace details
	{
		struct log_message;
	}
	class sink
	{
	  public:
		sink() noexcept					 = default;
		virtual ~sink() noexcept		 = default;
		sink(const sink& other) noexcept = default;
		sink(sink&& other) noexcept		 = default;
		sink& operator=(const sink& other) noexcept = default;
		sink& operator=(sink&& other) noexcept = default;

		virtual void log(std::span<const details::log_message> messages) = 0;

	  protected:
	};
} // namespace psl