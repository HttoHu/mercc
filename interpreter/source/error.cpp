/*
GNU GENERAL PUBLIC LICENSE
					   Version 3, 29 June 2007

 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.
*/
#include "../include/error.hpp"
#include "../include/lexer.hpp"
namespace Mer
{
	size_t get_line_no()
	{
		auto content = token_stream._get_content();
		auto pos = token_stream._get_pos();
		while (content[pos]->get_tag() != ENDL)
		{
			pos--;
		}
		return Endl::get_value(content[pos]);
	}

	LexerError::LexerError(const std::string& str):Error(str)
	{
		line_no = Endl::current_line;
	}
	std::string LexerError::what()
	{
		return "<Lexer Error:Line " + std::to_string(line_no) + " " + ems+">";
	}
}
