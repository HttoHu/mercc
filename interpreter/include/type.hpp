/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
using type_code_index = int;
#include "lexer.hpp"
#include <set>
#include <map>
#include <string>
namespace Mer
{
	class Token;
	class FunctionBase;
	extern std::map<type_code_index, std::string> type_name_mapping;
	namespace Mem
	{

		enum BasicType
		{
			ARRAY = -2, NDEF = -1, BVOID = 13, INT = 1, DOUBLE = 3, STRING = 5, BOOL = 7, CHAR = 9,
			INIT_LIST=11
		};
		class Type
		{
		public:
			enum class kind {
				single = 0, container = 1, dictionary = 2,structure=3,
			}type_kind = kind::single;
			Type(const std::string& _name, type_code_index bt, const std::set<type_code_index>& _convertible_types,size_t tc=1)
				:name(_name), type_code(bt), convertible_types(_convertible_types),type_size(tc) {}
			bool convertible(const type_code_index& t);
			void reset_type_size(size_t sz) { type_size = sz; }
			void add_compatible_type(type_code_index type_code);
			virtual std::string to_string() { return  name; }
			virtual std::size_t get_type_size() { return type_size; }
			virtual ~Type() {}

		protected:
			std::set<type_code_index> convertible_types;
			type_code_index type_code;
			std::string name;
			size_t type_size;
		};
		struct ComplexType
		{
			type_code_index container_type;
			type_code_index element_type;
			bool operator<(const ComplexType& op)const
			{
				if (container_type == op.container_type)
					return element_type < op.element_type;
				return container_type < op.container_type;

			}
		};
		/*
			* operator_type for instance []
			* for instance string tmp;
			* the type of tmp[3] is char
		*/
		type_code_index get_type_code();
		type_code_index get_type_code(Token* tok);
		type_code_index& type_no();
		// to get a compound type's code like vector<map<int,real>>
		type_code_index get_ctype_code();
		bool is_basic_type(Tag t);
		int register_container(type_code_index container_type, type_code_index element_type);
		type_code_index merge(type_code_index l, type_code_index r);
		std::pair<type_code_index, type_code_index>
			demerge(type_code_index t);
		extern std::map<type_code_index, ComplexType> demerge_table;
		extern std::map<ComplexType, type_code_index> merge_table;
		extern std::map<type_code_index, std::map<std::string, type_code_index>> type_op_type_map;
		extern std::map<std::string, type_code_index> type_index;
		extern std::map<type_code_index, Type*> type_map;
		extern type_code_index type_counter;
		extern std::map<type_code_index, void(*)(type_code_index)> container_register;
		// get the operator function type
		type_code_index find_op_type(type_code_index ty, std::string op);
		bool exist_operator(type_code_index ty, std::string op);
		// to test if t1 can convert to t2
		bool type_convertible(type_code_index t1, type_code_index t2);

		size_t get_type_length(type_code_index t);
	}
}