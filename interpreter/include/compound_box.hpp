/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <map>
#include "basic_objects.hpp"

#include "parser.hpp"
/* 
	from 1-7 is basic type 
	when the type code is even, it means that the type is a pointer
	for instance 1 -> int, 2 -> int *.
*/
#define USER_TYPE_INDEX 10
namespace Mer
{
	class InitList;
	class Id;
	class FunctionBase;
	using TVar=std::pair<size_t, Mem::Object>;
	using TName=std::pair<std::string, size_t>;
	// the class to record the struct infomation. 
	class UStructure
	{
	public:
		using SIM=std::map<std::string, int>;
		// add a new array member/or member;
		void push_new_children(type_code_index t, std::string id_name, size_t count=1);
		// first: member type second: member pos
		std::pair<type_code_index, size_t > get_member_info(std::string member_name);
		std::vector<Mem::Object> init();
		// get the member position by the name of it.
		SIM mapping;
		// get the member type by the name of it. Take care, you should be aware that mapping and STMapping is totally different.
		std::map<std::string, type_code_index> STMapping;
		void push_init(Mem::Object obj) { init_vec.push_back(obj); }
		void print();
		int get_size()const { return be; }
		const std::vector<size_t>& get_type_structure()const { return type_structure; }
		WordRecorder* find_id_info(const std::string &id);
		std::vector<Mem::Object> init_vec;
		std::map<std::string, WordRecorder*> structure_member_table;
	private:
		std::vector<size_t> type_structure;
		friend void _structure_member_def(type_code_index var_type, std::vector<Mem::Object> & init, UStructure * structure_content);
		friend void build_ustructure();
		// the index to new member is used in build phase. 
		int be = 0;
	};
	
	class MemberIndex :public ParserNode
	{
	public:
		MemberIndex(ParserNode *p, ParserNode *b, type_code_index ty) :parent(p), bias(b), type(ty) {}
		type_code_index get_type()override { return type; }
		size_t get_pos()override;
		Mem::Object execute()override;
		ParserNode *clone()override {
			return new MemberIndex(parent->clone(), bias->clone(), type);
		}
		~MemberIndex() { delete parent; delete bias; }
	private:
		ParserNode* parent;
		ParserNode* bias;
		type_code_index type;
	};
	// to execute assign operator of struct
	class StructWriter :public ParserNode
	{
	public:
		StructWriter(size_t ty, size_t _pos, ParserNode* p);
		Mem::Object execute()override;
	private:
		size_t type;
		int len;
		size_t pos;
		ParserNode* rhs;
	};
	class GStructWriter :public ParserNode
	{
	public:
		GStructWriter(size_t ty, size_t _pos, ParserNode* p);
		Mem::Object execute()override;
	private:
		size_t type;
		int len;
		size_t pos;
		ParserNode* rhs;
	};
	class StructCopyer :public ParserNode
	{
	public:
		StructCopyer(type_code_index ty, ParserNode* l, ParserNode* r);
		Mem::Object execute()override;
		~StructCopyer() {
			delete lhs;
			delete rhs;
		}

	private:
		int len;
		ParserNode* lhs;
		ParserNode* rhs;
	};
	// you can get the struct_info by its name
	extern std::map<std::string, UStructure*> ustructure_map;
	// get the struct name by its type code.

	extern std::map<type_code_index, std::map<std::string, FunctionBase*>> member_function_table;
	// to build the struct , records different information and push the struct to the ustructure_map
	void build_ustructure();
	void build_enum();
	InitList* parse_struct_init_list(type_code_index _ty);
	bool is_a_structure_type(type_code_index t);
	std::pair<type_code_index, ParserNode *> count_bias(UStructure *us);
	extern std::map<type_code_index, std::string> type_name_mapping;
	/*
		MemberVar need a Object to execute operator[] , which obtain from parents_vec.push_back()
		the parents_vec managed by member function call, when they call, they push a new Object which represent the parent 
		ready to call its member function.
	*/
	extern std::vector<Mem::Object> parents_vec;
	// if type not found, throw an exception, if you don't want to throw an exception please search the type by type_name_mapping.
	Mer::UStructure* find_ustructure_t(type_code_index type);
	// the map which store the compare operator to 
	extern std::map<type_code_index, FunctionBase*> comparison_map;
}