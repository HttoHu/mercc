/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include "lexer.hpp"
#include <utility>
#include <memory>
namespace Mer
{
	class ParserNode;
	using UptrPNode = std::unique_ptr<ParserNode>;
	using PosPtr = std::shared_ptr<size_t>;
	extern std::string output_buff;
	std::string run_interpreter(const std::string&file_content);
	extern std::map<std::string, void(*)()>repository;
	extern std::vector<size_t *> _pcs;
	extern std::vector<std::pair<PosPtr, PosPtr>> _nearest_loop_pos;
	/*
		when you call a struct-member function as follows (sps<-structure_parent_stack)
		co.
		* sps.push_back(co);
		call	distance(sps.back());
		* sps.pop_back();
	*/
	extern std::vector<ParserNode*> structure_parent_stack;
	void new_loop(PosPtr a,PosPtr b);
	void end_loop();
	PosPtr& loop_start();
	PosPtr& loop_end();
}