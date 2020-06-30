/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include "expr.hpp"
#include "function.hpp"
#define tsymbol_table root_namespace->sl_table
namespace Mer
{
	class Structure;
	class SymbolTable;
	class Namespace
	{
	public:
		Namespace(Namespace* pare);
		Namespace(Namespace* pare, 
			const std::map<std::string,Namespace*>&cs):parent(pare), children(cs) {}
		Namespace *parent;
		std::map<std::string, std::pair<Structure*, type_code_index>> structures;
		std::map<std::string, Namespace*> children;
		std::vector<Namespace*> using_namespaces;
		SymbolTable* sl_table = nullptr;
		void set_new_func(const std::string & name, FunctionBase * func);
		void set_new_var(const std::string &name,size_t type, Mem::Object obj);
		Mem::Object find_var(const std::string &name);
		~Namespace();
	};
	// 5-3 
	/*
	Task : 
	1. Namespace 1.0
	*/
	extern Namespace *root_namespace;
	extern Namespace *this_namespace;
	/* 
	* a class which is aimed to change the value of namespace_var,
	* the global variable also called namespace var in merdog, is differnt from local var. At the stage of 
	* syntax analysis, the local var will convert to a postion, however, global var will created in parser 
	* stage, so global var always be a object, and the Assign classes don't handle such situation. 
	*/
	namespace Parser
	{
		Mer::Namespace * _find_namespace_driver(Mer::Namespace *current, const std::string &name);
		Namespace *find_namespace(const std::string &name);
	}

}