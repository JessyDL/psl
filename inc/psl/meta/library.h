#pragma once
#include "psl/meta/data.h"

namespace psl::config
{
	namespace details
	{
		template <typename T>
		static psl::string_view library_extension = "lib";
	}
	static psl::string_view library_extension = details::library_extension<default_settings_t>;
} // namespace psl::config

namespace psl::meta
{
	/**
	 * \brief Keeps track of resource and their metadata
	 *
	 */
	class library
	{
	  public:
		library() noexcept					   = default;
		~library() noexcept					   = default;
		library(const library& other) noexcept = default;
		library(library&& other) noexcept	  = default;
		library& operator=(const library& other) noexcept = default;
		library& operator=(library&& other) noexcept = default;

	  private:
	};
} // namespace psl::meta