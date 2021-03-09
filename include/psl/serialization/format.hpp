#pragma once
#include <vector>
#include <psl/containers/variant.hpp>
#include <psl/containers/string_view.hpp>

namespace psl::serialization
{
	inline namespace details
	{
		static constexpr string_view FORMAT_FILE_SIGNATURE = "<JDLPFF>";
	}
	class format
	{
		/**
		 * \brief A singular value, think boolean, number, string, etc..
		 *
		 */
		struct value
		{
			string_view data;
		};
		/**
		 * \brief A named category/container. Contains [0, N) children.
		 *
		 */
		struct object
		{
			size_t children;
		};
		/**
		 * \brief A N-length value of the same type.
		 *
		 */
		struct array
		{};
		/**
		 * \brief An associative value that can be N-repeated.
		 *
		 */
		struct map
		{};
		/**
		 * \brief Referential value that can reference any other node type.
		 *
		 */
		struct reference
		{};

		struct info
		{
			string_view name;
		};

		struct meta
		{
			string_view type;
		};

	  public:
		format(psl::string_view data = "");

	  private:
		string_view::const_iterator parse_generic_object(string_view data);
		string_view::const_iterator parse_field(string_view data);

		std::vector<info> m_NodeInfo{};
		std::vector<meta> m_NodeMeta{};
		std::vector<psl::variant<object, value>> m_Nodes{};
	};
} // namespace psl::serialization
