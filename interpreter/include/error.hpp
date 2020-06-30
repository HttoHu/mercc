/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include <exception>
#include <string>
#include <vector>
#include <iostream>
#define DEBUG_TOOLS(msg) printf("FILE: %s, LINE: %d,FUNCTION: %s,CONTENT: %s",\
__FILE__, __LINE__ , __FUNCTION__,msg)
namespace Mer
{
	size_t get_line_no();
	class Error
	{
	public:
		Error(const std::string &str) :ems(str) {}
		virtual std::string what()
		{
			const std::string error_msg = "<line " + std::to_string(get_line_no()) + ">" + ems;
			return error_msg + "\n";
		}
	protected:
		std::string ems;
	};
	class LexerError:public Error
	{
	public:
		LexerError(const std::string& str);
		std::string what()override;
	private:
		int line_no=0;
	};
}