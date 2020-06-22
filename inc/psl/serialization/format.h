#pragma once
#include <memory>
#include "psl/array.h"
#include "psl/iterators.h"
#include "psl/exception.h"

namespace psl::serialization
{
	class format;

	namespace node
	{
		using node_t = uint32_t;
		struct header
		{
			node_t begin, end, depth, value_index;
		};
		struct value
		{
			bool is_object() const noexcept { return end == 0; }
			union
			{
				node_t begin;
				node_t children;
			};
			node_t end{0};
		};

		/**
		 * \brief Handle serves as an indirection to the underlaying node's data. It's considered a view, and so has
		 * limited editing capabilities.
		 * \warning It is undefined behaviour to invoke any method if the handle is not valid (operator bool() will
		 * return false on invalid handles).
		 * \warning Handles become invalid when the node they pointed to is destroyed. After which, invoking anything is
		 * undefined.
		 */
		struct handle
		{
		  public:
			handle(header* header, value* begin, value* end) noexcept : m_Header(header), m_Begin(begin), m_End(end){};
			handle(header* header, value* begin) noexcept : handle(header, begin, begin + 1){};
			handle() noexcept			= default;
			~handle()					= default;
			handle(const handle& other) = default;
			handle(handle&& other)		= delete;
			handle& operator=(const handle& other) = default;
			handle& operator=(handle&& other) = delete;

			operator bool() const noexcept { return m_Header != nullptr; }

			bool operator==(const handle& other) const noexcept { return m_Header == other.m_Header; }
			bool operator!=(const handle& other) const noexcept { return m_Header != other.m_Header; }
			bool operator<=(const handle& other) const noexcept { return m_Header <= other.m_Header; }
			bool operator>=(const handle& other) const noexcept { return m_Header >= other.m_Header; }
			bool operator<(const handle& other) const noexcept { return m_Header < other.m_Header; }
			bool operator>(const handle& other) const noexcept { return m_Header > other.m_Header; }

			psl::string_view name() const noexcept(!config::exceptions)
			{
				PSL_EXCEPT_IF(*this == false, "This is an invalid handle", std::logic_error);
				return u8"";
			}
			auto begin() noexcept(!config::exceptions)
			{
				PSL_EXCEPT_IF(*this == false, "This is an invalid handle", std::logic_error);
				return psl::contiguous_range_iterator<value>{m_Begin};
			}
			auto end() noexcept(!config::exceptions)
			{
				PSL_EXCEPT_IF(*this == false, "This is an invalid handle", std::logic_error);
				return psl::contiguous_range_iterator<value>{m_End};
			}
			auto size() const noexcept(!config::exceptions)
			{
				PSL_EXCEPT_IF(*this == false, "This is an invalid handle", std::logic_error);
				return m_End - m_Begin;
			}

			bool is_object() const noexcept(!config::exceptions)
			{
				PSL_EXCEPT_IF(*this == false, "This is an invalid handle", std::logic_error);
				return m_Begin->is_object();
			}
			bool is_value() const noexcept(!config::exceptions)
			{
				PSL_EXCEPT_IF(*this == false, "This is an invalid handle", std::logic_error);
				return !m_Begin->is_object();
			}

		  private:
			format* m_Format{nullptr};
			header* m_Header{nullptr};
			value* m_Begin{nullptr};
			value* m_End{nullptr};
		};
	} // namespace node

	/**
	 * \brief Basic format that can represent serialized objects on disk.
	 * \details A format that internally is optimized for sequential access, ideally used in conjunction with the rest
	 * of the serialization implementation which has an access pattern that suits this (append/sequential read).
	 * \note Performance degrades with non-sequential access as the tree-like structure is flattened into an array-like
	 * object when in memory.
	 * \warning On the same depth level, nodes must remain uniquely identified.
	 * \warning Performance suffers greatly from doing manual editing of nodes (i.e. inserting new nodes anywhere other
	 * than the end, and editing values).
	 */
	class format
	{

	  public:
		format() = default;

		void add([[maybe_unused]] psl::string_view name, [[maybe_unused]] node::handle& parent) {}


		node::handle* find([[maybe_unused]] psl::string_view name) { return nullptr; };
		bool try_find(psl::string_view name, node::handle& out)
		{
			auto res = find(name);
			if(res) out = *res;
			return res;
		};

	  private:
		psl::array<std::unique_ptr<node::handle>> m_Handles;
		psl::array<node::header> m_Names;
		psl::array<node::value> m_Values;
	};
} // namespace psl::serialization