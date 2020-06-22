#pragma once
#include "psl/string.h"
#include "psl/config.h"
#include "psl/uid.h"
#include "psl/array.h"
#include "psl/serialization/property.h"
#include "psl/serialization/versioning.h"
#include "psl/serialization/serialization.h"

#include <type_traits>

namespace psl::config
{
	namespace details
	{
		template <typename T>
		static psl::string_view meta_extension = u8"meta";
	}
	static psl::string_view meta_extension = details::meta_extension<default_setting_t>;
} // namespace psl::config

namespace psl::meta
{
	using tag = psl::string;
	class data;
} // namespace psl::meta

namespace psl::versioning
{
	template <>
	struct packet<psl::meta::data, 0>
	{
		auto properties() noexcept { return make_property_pack(m_UID, m_Tags); }

		psl::property<uid, u8"uid"> m_UID{};
		psl::property<psl::array<psl::meta::tag>, u8"tags"> m_Tags{};
	};
} // namespace psl::versioning

namespace psl::meta
{
	class data : protected psl::versioning::packet<data>
	{
		friend struct psl::serialization_accessor;

	  public:
		data(psl::uid uid, std::span<const tag> tags = {}) noexcept
			: psl::versioning::packet<data>(std::move(uid), tags){};
		data(psl::uid uid, psl::array<tag>&& tags) noexcept
			: psl::versioning::packet<data>(std::move(uid), std::move(tags)){};
		data() noexcept					 = default;
		~data() noexcept				 = default;
		data(const data& other) noexcept = delete;
		data(data&& other) noexcept		 = delete;
		data& operator=(const data& other) noexcept = delete;
		data& operator=(data&& other) noexcept = delete;

		const psl::uid& uid() const noexcept { return m_UID; }
		std::span<const tag> tags() const noexcept { return {m_Tags->data(), m_Tags->size()}; }

	  private:
	};

	template <typename T>
	concept IsMetaData = std::is_base_of_v<data, T>;

	template <typename T>
	struct meta_traits
	{
		using meta_type = data;
	};
} // namespace psl::meta