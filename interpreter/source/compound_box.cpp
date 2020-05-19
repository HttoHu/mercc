/*
* MIT License
* Copyright (c) 2019 Htto Hu
*/
#include "../include/compound_box.hpp"
#include "../include/lexer.hpp"
#include "../include/memory.hpp"
#include "../include/word_record.hpp"
#include "../include/namespace.hpp"
#include "../include/optimizer.hpp"
using namespace Mer;
namespace Mer
{
	std::map<std::string, UStructure*> ustructure_map;
	std::map<type_code_index, std::string> type_name_mapping;
	std::map<type_code_index, std::map<std::string, FunctionBase*>> member_function_table;
	std::vector<Mem::Object> parents_vec;
	// the impl of the member variable of structure parser.
	void _structure_member_def(type_code_index var_type, std::vector<Mem::Object> & init, UStructure * structure_content) {
		std::vector<Mer::VarDeclUnit> member_units;

		while (true)
		{
			member_units.push_back(VarDeclUnit(var_type));
			if (token_stream.this_tag() == COMMA)
				token_stream.match(COMMA);
			else
				break;
		}
		for (auto &unit : member_units)
		{
			WordRecorder* recorder = nullptr;
			int len = unit.get_size();
			if (unit.arr())
			{
				len--;
				std::vector<ParserNode*> arr;
				auto exprs_info = unit.get_expr();
				if (typeid(*exprs_info) == typeid(InitList))
					arr = static_cast<InitList*>(unit.get_expr())->exprs();
				else
					arr = static_cast<EmptyList*>(unit.get_expr())->exprs();
				for (auto & it : arr)
					// it should be a const-literal, if you init the member with a var, the execute() will crash and print some error infomation which points to
					// the place of the error.
					init.push_back(it->execute());
				// two means it it an array.
				recorder = new GArrayRecorder(var_type, structure_content->be, unit.array_indexs);

			}
			else
			{
				init.push_back(unit.get_expr()->execute());
				recorder = new GVarIdRecorder(var_type, structure_content->be);
			}
			std::string mem_name = Id::get_value(unit.get_id());
			structure_content->push_new_children(var_type, mem_name, len);
			structure_content->structure_member_table.insert({ mem_name,recorder });
		}
	}

	//OK
	void build_ustructure()
	{
		token_stream.match(STRUCT);
		std::string name = Id::get_value(token_stream.this_token());
		if (ustructure_map.count(name))
			throw Error("struct " + name + " redefined");
		// you need to record the type info of the struct imediately, or you can't define the pointer of self-type;
		Mem::type_counter += 2;
		Mer::this_namespace->sl_table->push(name, new WordRecorder(ESymbol::SSTRUCTURE, Mem::type_counter));
		token_stream.match(ID);
		token_stream.match(BEGIN);
		UStructure* us = new UStructure();
		// register struct 
		ustructure_map.insert({ name,us });
		Mem::type_index.insert({ name,Mem::type_counter });
		type_name_mapping.insert({ Mem::type_counter,name });
		Mem::Type* struct_type_info = new Mem::Type(name, Mem::type_counter, { Mem::type_counter });
		Mem::type_map.insert({ Mem::type_counter,struct_type_info });
		tsymbol_table->new_block();
		//parsing
		while (token_stream.this_tag() != END)
		{
			size_t type = Mem::get_type_code();
			_structure_member_def(type, us->init_vec, us);
			token_stream.match(SEMI);
		}
		struct_type_info->reset_type_size(us->get_size());
		tsymbol_table->end_block();
		token_stream.match(END);
		token_stream.match(SEMI);
	}

	void build_enum()
	{
		token_stream.match(ENUM);
		std::string enum_type_name = Id::get_value(token_stream.this_token());
		tsymbol_table->push(enum_type_name, new WordRecorder(ESymbol::SENUM, Mem::INT));
		token_stream.next();
		token_stream.match(BEGIN);
		int cur_enumerate = 0;
		while (true)
		{
			std::string cur_enumerate_name = Id::get_value(token_stream.this_token());
			int this_enumerate = cur_enumerate;
			token_stream.next();
			if (token_stream.this_tag() == ASSIGN)
			{
				token_stream.next();
				auto tok = token_stream.this_token();
				if (tok->get_tag() == TTRUE)
					this_enumerate = 1;
				else if (tok->get_tag() == TFALSE)
					this_enumerate = 0;
				else if (tok->get_tag() == INTEGER)
					this_enumerate = Mer::Integer::get_value(tok);
				else
					throw Error("token " + tok->to_string() + " can not be an enumerator");
				token_stream.next();
			}

			cur_enumerate = this_enumerate + 1;
			auto this_symbol = new WordRecorder(ESymbol::SENUM_MEMBER, Mem::INT);
			this_symbol->count = this_enumerate;
			tsymbol_table->push(cur_enumerate_name, this_symbol);
			if (token_stream.this_tag() == COMMA)
				token_stream.next();
			else
				break;
		}
		token_stream.match(END); token_stream.match(SEMI);
	}

	InitList* parse_struct_init_list(type_code_index _ty)
	{
		UStructure* result = find_ustructure_t(_ty);
		const std::vector<size_t>& type_vec = result->get_type_structure();
		std::vector<size_t>::size_type idx = 0;
		std::vector<ParserNode*> expr_lists;
		// get member structure
		token_stream.match(BEGIN);
		while (true)
		{
			expr_lists.push_back(Expr(type_vec[idx++]).root());
			if (token_stream.this_tag() == COMMA)
				token_stream.match(COMMA);
			else
				break;
		}
		token_stream.match(END);
		return new InitList(expr_lists, _ty);
	}

	bool is_a_structure_type(type_code_index t)
	{
		return type_name_mapping.find(t) != type_name_mapping.end();
	}

	std::pair<type_code_index,ParserNode *> count_bias(UStructure * us)
	{
		ParserNode* ret=nullptr;
		std::string mem_name = Id::get_value(token_stream.this_token());
		token_stream.next();
		auto result = us->find_id_info(mem_name);
		type_code_index ret_type=result->get_type();
		switch (result->es)
		{
		case ESymbol::SGVAR:
			ret = _make_l_conv(result->get_pos());
			break;
		case ESymbol::SGARR:
			ret = optimizer::optimize_bin_op(
				get_array_bias<ArrayRecorder>(result),
				_make_l_conv(result->get_pos()),
				BasicToken["+"]);
			break;
		default:
			throw Error("unsupported grammar, @count_bias");
		}
		if (token_stream.this_tag() == DOT)
		{
			token_stream.next();
			auto ustruct = find_ustructure_t(result->get_type());
			auto children = count_bias(ustruct);
			ret=optimizer::optimize_bin_op(children.second, ret, BasicToken["+"]);
			ret_type = children.first;
		}
		return{ ret_type, ret };
	}


	UStructure* find_ustructure_t(type_code_index type)
	{
		auto result = type_name_mapping.find(type);
		if (result == type_name_mapping.end())
			throw Error("struct type " + std::to_string(type) + " undefined");
		auto result2 = ustructure_map.find(result->second);
		if (result2 == ustructure_map.end())
			throw Error("Id " + result2->first + " undefined");
		return result2->second;
	}
	void UStructure::push_new_children(size_t t, std::string id_name, size_t count)
	{
		type_structure.push_back(t);
		mapping.insert({ id_name,be += count });
		STMapping.insert({ id_name,t });
	}
	std::pair<type_code_index, size_t> UStructure::get_member_info(std::string member_name)
	{
		auto result = mapping.find(member_name);
		if (result == mapping.end())
			throw Error("member " + member_name + " no found!");
		auto type_result = STMapping.find(member_name);
		return { type_result->second,result->second };
	}
	std::vector<Mem::Object> UStructure::init()
	{
		std::vector<Mem::Object> ret(init_vec.size());
		for (size_t i = 0; i < ret.size(); i++)
		{
			ret[i] = init_vec[i]->clone();
		}
		return ret;
	}

	void UStructure::print()
	{
		std::cout << "------Name-Type-Map------\n";
		for (auto a : STMapping)
		{
			std::cout << "name:" << a.first << "\t\t\t" << "type: " << a.second << std::endl;
		}
		std::cout << "------Name-Position_Map------\n";
		for (auto a : mapping)
		{
			std::cout << "name:" << a.first << "\t\t\t" << "position: " << a.second << std::endl;
		}
		std::cout << "-----------------------------------\n";
	}


	WordRecorder* UStructure::find_id_info(const std::string& id)
	{
		auto result = structure_member_table.find(id);
		if (result == structure_member_table.end())
			throw Error("member " + id + " no found!");
		return result->second;
	}
	size_t MemberIndex::get_pos()
	{
		// the struct is regarded as a array, and the first element is the array info so we need +1 to get the real pos
		return parent->get_pos() + (size_t)*(int*)bias->execute()->get_raw_data();
	}
	Mem::Object MemberIndex::execute()
	{
		return Mer::mem[get_pos()];
	}
}