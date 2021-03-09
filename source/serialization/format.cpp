#include <psl/serialization/format.hpp>

#include <array>

#include <psl/containers/string_view.hpp>

using namespace psl;
using namespace psl::serialization;

static constexpr std::array<string_view, 4> EMPTY_SPACE{" "_sv, "\n"_sv, "\t"_sv, "\r"_sv};
static constexpr std::array<string_view, 5> EMPTY_SPACE_OR_TERMINATOR{" "_sv, "\n"_sv, "\t"_sv, "\r"_sv, ";"_sv};
static constexpr std::array EMPTY_SPACE_OR_SCOPE{" "_sv, "\n"_sv, "\t"_sv, "\r"_sv, "{"_sv};
static constexpr string_view ASSIGNMENT{"="_sv};
static constexpr std::array<string_view, 5> ASSIGNMENT_OR_EMPTY_SPACE{" "_sv, "\n"_sv, "\t"_sv, "\r"_sv, "="_sv};


string_view::const_iterator format::parse_field(string_view data)
{
	auto token_begin = data.find_first_not_of(EMPTY_SPACE);
	if(token_begin == data.end() || *token_begin == '}') return data.end();
	auto token_end = data.find_first_of(ASSIGNMENT_OR_EMPTY_SPACE, token_begin);

	auto next_token = data.find_first_not_of(EMPTY_SPACE, token_end);

	string_view name{};
	string_view type{};
	string_view complex_type{};
	string_view value{};

	// check if type is a complex type. this might indicate we still haven't escaped the type's definition.
	auto temp = data.substr(token_begin, token_end);

	if(temp.find("<"_sv) != std::end(temp) || *next_token == '<')
	{
		token_end	  = data.find("<"_sv, token_begin) + 1;
		complex_type  = data.substr(token_begin, token_end);
		auto cut_data = data.substr(token_end);
		size_t depth  = 1u;
		for(auto c : cut_data)
		{
			if(c == '<')
				++depth;
			else if(c == '>')
				--depth;

			++token_end;
			if(depth == 0) break;
		}
		PSL_EXCEPT_IF(depth != 1, std::runtime_error, "did not find the end of the complex type");

		next_token = data.find_first_not_of(EMPTY_SPACE, token_end);
	}

	// if the next_token is not an assignment, that means we have both a type and a name.
	if(string_view{next_token, next_token + 1} != "="_sv)
	{
		type	  = data.substr(token_begin, token_end);
		token_end = data.find_first_of(EMPTY_SPACE, next_token);
		name	  = data.substr(next_token, token_end);
		token_end = data.find_first_of(ASSIGNMENT, token_end);
	}
	else
	{
		name	  = data.substr(token_begin, token_end);
		token_end = next_token;
		type	  = {};
	}

	// we aren't sure yet if it's a complex type, so let's check right now.
	token_begin = data.find_first_not_of(EMPTY_SPACE, token_end + 1);
	if(*token_begin == '{')
	{
		if(complex_type.size() > 0)
		{
			PSL_NOT_IMPLEMENTED(0);
		}
		else
		{
			PSL_NOT_IMPLEMENTED(0);
			token_end = parse_generic_object(data.substr(token_begin + 1));
		}
	}
	else
	{
		const bool is_literal = *token_begin == '"' || *token_begin == '\'';
		if(is_literal)
		{
			if(*(token_begin) == '\'' && *(token_begin + 1) == '\'' && *(token_begin + 2) == '\'')
			{
				token_begin += 3;
				token_end = token_begin;
				while(true)
				{
					if(*(token_end) == '\'' && *(token_end + 1) == '\'' && *(token_end + 2) == '\'') break;
					++token_end;
				}
			}
			else
			{
				const auto literal_char = *token_begin;
				token_begin += 1;
				token_end = token_begin;
				while(true)
				{
					if(*token_end == literal_char) break;
					++token_end;
				}
			}
		}
		else
		{
			token_begin += 1;
			token_end = data.find_first_of(EMPTY_SPACE_OR_TERMINATOR, token_begin);
		}
		value	  = data.substr(token_begin, token_end);
		token_end = data.find(";"_sv, token_end) + 1;

		m_Nodes.emplace_back(format::value{value});
	}

	m_NodeInfo.emplace_back(name);
	m_NodeMeta.emplace_back(type);

	return token_end;
}

string_view::const_iterator format::parse_generic_object(string_view data)
{
	auto it = data.cbegin();
	while([&] {
		it = parse_field(data);
		return it != data.end();
	}())
	{
		data = data.substr(it);
	}

	return data.end();
}

format::format([[maybe_unused]] psl::string_view_impl<string_encoding::UTF8> data)
{
	if(data.starts_with(FORMAT_FILE_SIGNATURE))
	{
		PSL_NOT_IMPLEMENTED(0);
	}

	std::vector<string_view> templates{};

	auto token_begin = data.find_first_not_of(EMPTY_SPACE);
	auto token_end	 = data.find_first_of(EMPTY_SPACE, token_begin);

	auto word = data.substr(token_begin, token_end);

	if(word == "object"_sv)
	{
		token_begin = data.find_first_not_of(EMPTY_SPACE, token_end);
		token_end	= data.find_first_of(EMPTY_SPACE_OR_SCOPE, token_begin);

		auto name = data.substr(token_begin, token_end);
		parse_generic_object(data.substr(data.find_first_of("{"_sv, token_end) + 1));
	}
	else if(word == "template"_sv)
	{}
	else if(std::find(std::begin(templates), std::end(templates), word) != std::end(templates))
	{}
	else
	{
		// PSL_EXCEPT(std::runtime_error, "could not find the template [" + word + "] in the template library");
	}
}
