#pragma once
#include "psl/string.h"
#include "psl/algorithms.h"
#include <iostream>

namespace psl
{
	template <template <typename> typename Converter = psl::converter>
	class stream_writer
	{
	  public:
		stream_writer(std::ostream& stream) : stream(stream){};

		template <typename T>
		void write_value(psl::string_view name, const T& value)
		{
			stream << std::string(m_Depth * 2, ' ') << psl::to_pstring(name) << " { "
				   << psl::to_pstring(Converter<T>{}.to_string(value)) << " };\n";
		}

		template <typename T, typename It>
		void write_range(psl::string_view name, It&& begin, It&& end)
		{
			stream << std::string(m_Depth * 2, ' ') << psl::to_pstring(name) << " { ";
			auto count = std::distance(begin, end);
			for(auto it = begin; it != end; ++it)
			{
				if constexpr(std::is_same_v<T, psl::string> || std::is_same_v<T, std::string>)
					stream << '"' << psl::to_pstring(Converter<T>{}.to_string(*it)) << '"';
				else
					stream << psl::to_pstring(Converter<T>{}.to_string(*it));
				if(--count != 0) stream << ", ";
			}
			stream << " };\n";
		}

		void begin_scope(psl::string_view name, size_t version = 0) noexcept
		{
			stream << std::string(m_Depth * 2, ' ') << psl::to_pstring(name)
				   << std::string(std::min<size_t>(name.size(), 1u), ' ') << "[version: " << std::to_string(version)
				   << "]\n"
				   << std::string(m_Depth * 2, ' ') << "{\n";
			++m_Depth;
		}

		void begin_scope(psl::string_view name, size_t version, uint64_t polymorphic_id) noexcept
		{
			stream << std::string(m_Depth * 2, ' ') << psl::to_pstring(name)
				   << std::string(std::min<size_t>(name.size(), 1u), ' ') << "[version: " << std::to_string(version)
				   << "][polymorphic_id: " << std::to_string(polymorphic_id) << "]\n"
				   << std::string(m_Depth * 2, ' ') << "{\n";
			++m_Depth;
		}

		void end_scope([[maybe_unused]] psl::string_view name) noexcept
		{
			--m_Depth;
			stream << std::string(m_Depth * 2, ' ') << "};\n";
		}

	  private:
		int m_Depth{0};
		std::ostream& stream;
	};

	template <template <typename> typename Converter = psl::converter>
	class stream_reader
	{
	  public:
		stream_reader(std::istream& stream) : stream(stream) {}

		template <typename T>
		bool read_value(psl::string_view name, T& output) noexcept
		{
			return false;
		}

		void begin_scope(psl::string_view name) noexcept {}

		void end_scope(psl::string_view name) noexcept {}

	  private:
		int m_Depth{0};
		std::istream& stream;
	};
} // namespace psl