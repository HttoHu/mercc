/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include <iostream>
#include <memory>
#include "parser_node.hpp"

namespace Mer
{
	enum Tag;
	namespace Mem
	{
		class Value;
		using Object = std::shared_ptr<Value>;
	}
	namespace optimizer
	{
		extern std::map<Mer::Tag, Mem::Object(*) (const Mem::Object&, const Mem::Object&)> op_table;
		extern std::map<std::pair<Mer::Tag,type_code_index>, Mem::Object(*)(const Mem::Object&)> unary_op_table;
		ParserNode* optimize_bin_op(ParserNode* left, ParserNode* right, Token* tok);
		ParserNode* optimize_unary_op(ParserNode* left, Token* tok);
		// try to obtain the address directly
		ParserNode* optimize_array_subscript(ParserNode* arr, ParserNode* subscript,type_code_index default_type=0);
	}
}