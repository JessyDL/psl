#pragma once

#include <ostream>
#include "psl/string.h"
#include "psl/log.h"

namespace psl
{
	class ostream : public sink
	{
	  public:
		ostream(std::ostream& stream) : m_Stream(&stream){};

	  private:
		void log(std::span<const details::log_message> messages)
		{
			for(auto& message : messages)
			{
				*m_Stream << to_pstring(message.preformat) << "\n";
			}

			m_Stream->flush();
		}
		std::ostream* m_Stream;
	};
} // namespace psl