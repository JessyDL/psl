#pragma once
#include "psl/string.h"
#include "psl/config.h"
#include "psl/uid.h"
#include "psl/array.h"

namespace psl::config
{
	namespace details
	{
		template <typename T>
		static psl::string_view meta_extension = "meta";
	}
	static psl::string_view meta_extension = details::meta_extension<default_settings_t>;
} // namespace psl::config

namespace psl::meta
{


	class data
	{
	  public:
		data(psl::uid uid) noexcept : m_UID(uid){};
		data() noexcept					 = default;
		~data() noexcept				 = default;
		data(const data& other) noexcept = delete;
		data(data&& other) noexcept		 = delete;
		data& operator=(const data& other) noexcept = delete;
		data& operator=(data&& other) noexcept = delete;

		const psl::uid& uid() const noexcept { return m_UID; }
		std::span<const psl::string> tags() const noexcept { return m_Tags; }

	  private:
		uid m_UID{};
		psl::array<psl::string> m_Tags{};
	};
} // namespace psl::meta