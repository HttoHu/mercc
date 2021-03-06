/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
#include "../include/environment.hpp"
#include "../include/parser.hpp"
#include "../include/clib/maths.hpp"
#include "../include/clib/containers.hpp"
#include "../include/clib/dictionary.hpp"
#include "../include/clib/utility.hpp"
#include "../include/function.hpp"
#include "../include/memory.hpp"
#include "../include/compound_box.hpp"


int err_line = 0;
std::string err_msg;
extern std::string pre_input_content;

namespace Mer
{
	void set_cstring();
	std::map<std::string, void(*)()> repository{
		{ "vector",Container::using_vector },{ "deque",Container::using_deque },
		{"math.h",set_maths},
		{ "set",using_set },{ "map",using_map },{ "stdio.h",set_io },{"string.h",set_cstring}
	};


	std::vector<size_t*> _pcs;
	std::string output_buff = "";
	std::vector<std::pair<PosPtr, PosPtr>> _nearest_loop_pos;
	std::vector<Mer::ParserNode*> structure_parent_stack;

	std::string input_buf = "";
	std::stringstream my_stringstream;
	extern std::map<type_code_index, std::map<std::string, FunctionBase*>> member_function_table;
	extern std::map<std::string, UStructure*> ustructure_map;
	namespace Mem
	{
		void _clear_type_info();
	}
	namespace
	{
		void clear()
		{
			Mer::token_stream.clear();

			delete Mer::root_namespace;
			Mer::root_namespace = new Mer::Namespace(nullptr);
			delete mstd;
			mstd = new Mer::Namespace(nullptr);

			Mer::this_namespace = Mer::root_namespace;
			std::vector<FunctionBase*> vec;
			for (auto a : rem_functions)
			{
				delete a;
			}
			Mer::rem_functions.clear();
			Mer::pre_stmt.clear();
			mem.reset();
			Mem::_clear_type_info();

			compare_map.clear();

			// remove initialization function
			for (auto a : type_init_function_map)
				delete a.second;
			type_init_function_map.clear();
			// remove member function call
			for (auto& a : member_function_table)
			{
				for (auto& b : a.second)
				{
					delete b.second;
				}
			}
			member_function_table.clear();
			for (auto& a : ustructure_map)
			{
				delete a.second;
			}
			ustructure_map.clear();
			// pre_input clear
			my_stringstream.clear();
			pre_input_content = "";
		}
		void _merdog_init_()
		{
			using namespace Mer;
			using namespace Mem;
			Mer::root_namespace->children.insert({ "std", mstd });
			auto compare_op = [](Mem::Object a, Mem::Object b) {
				return std::static_pointer_cast<Mem::Bool>(a->operator<(b))->_value(); };
			compare_map.insert({ Mem::INT,compare_op });
			compare_map.insert({ Mem::BOOL,compare_op });
			compare_map.insert({ Mem::DOUBLE,compare_op });
			compare_map.insert({ Mem::STRING,compare_op });
			compare_map.insert({ Mem::CHAR,compare_op });
		}
	}

	std::string run_interpreter(const std::string& file_content)
	{
		output_buff = "";
		err_msg = "";

		_merdog_init_();

		try
		{
			Mer::new_build_token_stream(file_content);
			Parser::program();
			for (auto& a : pre_stmt)
			{
				a->execute();
			}
			clear();
		}
		catch (const std::exception& e)
		{
			auto ret = e.what();
			token_stream.clear();
			clear();
			return ret;
		}
		catch (Mer::Error& e)
		{
			auto ret = e.what();
			err_line = get_line_no();
			err_msg = e.what();
			token_stream.clear();
			clear();
			return ret;
		}
		return output_buff;
	}
	void new_loop(PosPtr a, PosPtr b)
	{
		_nearest_loop_pos.push_back(std::make_pair(a, b));
	}
	void end_loop()
	{
		_nearest_loop_pos.pop_back();
	}
	Mer::PosPtr& loop_start()
	{
		return _nearest_loop_pos.back().first;
	}
	Mer::PosPtr& loop_end()
	{
		return _nearest_loop_pos.back().second;
	}

}
