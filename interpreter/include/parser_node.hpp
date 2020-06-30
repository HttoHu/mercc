/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include <string>
#include "basic_objects.hpp"
using type_code_index = int;
namespace Mer
{
	class ParserNode
	{
	public:
		ParserNode()
		{
		}
		virtual size_t get_pos() { return 0; }
		virtual ~ParserNode() {}
		virtual std::string to_string()
		{
			return "<empty_node>";
		}
		// to distinguish some global variables or arrays and so on... 
		virtual bool global()const
		{
			return false;
		}
		virtual type_code_index get_type()
		{
			return 0;
		}
		virtual Mem::Object execute() 
		{
			throw std::runtime_error("called by an indefinite var");
		}
		// clone is not overrided by all children. 
		virtual ParserNode* clone() {
			throw Error("cloned in ParserNode");
		}
	};
	using UptrPNode = std::unique_ptr<ParserNode>;

}
