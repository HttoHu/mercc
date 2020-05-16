/*function
* MIT License
* Copyright (c) 2019 Htto Hu
*/
#include "../include/parser.hpp"
#include "../include/expr.hpp"
#include "../include/word_record.hpp"
#include "../include/namespace.hpp"
#include "../include/function.hpp"
#include "../include/environment.hpp"
#include "../include/compound_box.hpp"
#include "../include/branch_and_loop.hpp"
#include "../include/namespace.hpp"
namespace Mer
{
	std::vector<UptrPNode> pre_stmt;
	namespace Parser
	{
		ParserNode* statement();
	}
	Mem::Object Program::execute()
	{
		mem.new_func(off);
		for (auto& a : pre_stmt)
		{
			a->execute();
		}
		for (*pc = 0; *pc < stmts.size(); ++ * pc)
			stmts[*pc]->execute();
		mem.end_func();
		return nullptr;
	}
	std::string Program::to_string()
	{
		std::string ret;
		for (auto& a : pre_stmt)
		{
			ret += a->to_string() + '\n';
		}
		ret += "\n\n\n";
		for (auto a : rem_functions)
		{
			ret += a->to_string() + "\n\n\n\n";
		}
		ret += "PROGRAM\n";
		for (auto& a : stmts)
		{
			ret += a->to_string() + '\n';
		}
		return ret;
	}
	bool& global_stmt()
	{
		static bool ret = true;
		return ret;
	}
	SizeOf::SizeOf()
	{

		token_stream.match(SIZEOF);
		token_stream.match(LPAREN);
		Mer::WordRecorder* result = nullptr;
		if (token_stream.this_tag() == ID)
			result = this_namespace->sl_table->find(Id::get_value(token_stream.this_token()));
		// sizeof(type)
		if (Mem::is_basic_type(token_stream.this_tag()) || result&& result->es == STYPE)
		{
			type_code_index type_code= Mem::get_type_code();
			if (token_stream.this_tag() == MUL)
				obj = std::make_shared<Mem::Int>(sizeof(size_t));
			else
				obj = std::make_shared<Mem::Int>(Mem::get_type_length(type_code));
			token_stream.match(RPAREN);
			return;
		}
		// sizeof(var)
		if (result == nullptr)
			throw Error("undefined id " + Id::get_value(token_stream.this_token()));
		size_t type_len = Mem::get_type_length(result->get_type());
		obj = std::make_shared<Mem::Int>(result->count*type_len);
		token_stream.next();
		token_stream.match(RPAREN);

	}
	MakeDefault::MakeDefault(type_code_index ty) :type(ty)
	{
		ret = Mem::create_var_t(ty);
	}
	Mem::Object MakeDefault::execute()
	{
		return ret->clone();
	}
	std::pair<std::vector<size_t>, std::vector<ParserNode*>> Parser::linearized_array(size_t element_type)
	{
		std::vector<size_t> ret_first;
		std::vector<ParserNode*> ret_second;
		auto arr_init_list = build_array_initlist_tree(element_type);

		// bfs to build ret_second
		std::deque<ArrayInitList*> q;
		q.push_back(arr_init_list);
		while (!q.empty()) {
			auto u = q.front();
			q.pop_front();
			if (u->leaves_parent()) {
				for (auto a : u->leaves)
					ret_second.push_back(a);
			}
			for (auto a : u->children) {
				q.push_back(a);
			}
		}
		//===================================
		// dfs to build ret_first the structure of the array
		while (true) {
			ret_first.push_back(arr_init_list->size());
			if (arr_init_list->leaves_parent())
				break;
			arr_init_list = arr_init_list->children.front();
		}

		return { ret_first,ret_second };
	}

	ArrayInitList* Parser::build_array_initlist_tree(size_t ele_type_code)
	{
		
		std::vector<ArrayInitList*> children;
		std::vector<ParserNode*> leaves;
		// we regard "HEY" as {'H','E','Y','\0'};
		if (token_stream.this_tag() == Mer::STRING)
		{
			std::string value = String::get_value(token_stream.this_token());
			for (auto a : value) {
				leaves.push_back(new LConV(std::make_shared<Mem::Char>(a),Mem::CHAR));
			}
			leaves.push_back(new LConV(std::make_shared<Mem::Char>(0), Mem::CHAR));
			token_stream.next();
			return new ArrayInitList(leaves);
		}

		token_stream.match(BEGIN);

		while (token_stream.this_tag() == BEGIN) {
			children.push_back(build_array_initlist_tree(ele_type_code));
			if (token_stream.this_tag() == END)
				break;
			token_stream.match(COMMA);
		}
		while (token_stream.this_tag() != END) {
			leaves.push_back(Expr(ele_type_code).root());
			while (token_stream.this_tag() != END) {
				token_stream.match(COMMA);
				leaves.push_back(Expr(ele_type_code).root());
			}
		}
		token_stream.match(END);
		if (leaves.empty()) {
			if (children.empty())
				throw Error("empty array init list!");
			return new ArrayInitList(children);
		}
		else if (children.empty()) {
			if (leaves.empty())
				throw Error("empty array init list!");
			return new ArrayInitList(leaves);
		}
		else
			throw Error("invalid array init list");
	}
	void Parser::program()
	{
		global_stmt() = true;
		auto ret = std::make_unique<Program>(nullptr);
		while (1)
		{
			switch (token_stream.this_tag())
			{
			case ENUM:
				build_enum();
				break;
			case IMPORT:
			{
				token_stream.match(IMPORT);
				std::string name = Id::get_value(token_stream.this_token());
				auto result = repository.find(name);
				if (result == repository.end())
					throw Error("can not find lib " + name + " you may need to update merdog or check your spelling");
				result->second();
				token_stream.match(ID);
				token_stream.match(SEMI);
				break;
			}
			case STRUCT:
				build_ustructure();
				break;
			case ENDOF:
				mem.get_current() = mem.get_capacity() - 1;
				return;
			case ID:
			default:
				auto stmt = Parser::statement();
				if(stmt)
					pre_stmt.push_back(UptrPNode(stmt));
				break;
			}
		}
	}
	ParserNode* Parser::statement()
	{
		ParserNode* node = nullptr;
		switch (token_stream.this_token()->get_tag())
		{
		case VOID_DECL:
		case CHAR_DECL:
		case BOOL_DECL:
		case INTEGER_DECL:
		case REAL_DECL:
		case STRING_DECL:
			node = var_decl();
			if (node == nullptr)
			{
				return nullptr;
			}
			break;
		default:
			node = Expr().root();
			break;
		}

		token_stream.match(SEMI);
		return node;
	}
	ParserNode* Parser::var_decl()
	{
		type_code_index type = Mem::get_type_code();
		bool pointer_type = false;
		if (token_stream.this_tag() == MUL)
		{
			token_stream.match(MUL);
			type++;
			pointer_type = true;
		}
		// if it is a function decl
		if (global_stmt() && token_stream.this_tag() == ID)
		{
			auto id_name = Id::get_value(token_stream.this_token());
			token_stream.match(ID);
			// function decl or statement
			if (token_stream.this_tag() == LPAREN)
			{
				build_function(type, id_name);
				return nullptr;
			}
			else
			{
				token_stream.back();
			}
		}
		if (pointer_type)
		{
			token_stream.back();
			type--;
		}
		std::vector<VarDeclUnit*> units;
		units.push_back(new VarDeclUnit(type));
		while (token_stream.this_tag() != SEMI)
		{
			token_stream.match(COMMA);
			units.push_back(new VarDeclUnit(type));
		}
		if (global_stmt())
		{
			return new GloVarDecl(units, type);
		}
		return new LocalVarDecl(units, type);
	}
	type_code_index Parser::get_type()
	{
		switch (token_stream.this_tag())
		{
		case CHAR_DECL:
			token_stream.match(CHAR_DECL);
			return Mem::CHAR;
		case VOID_DECL:
			token_stream.match(VOID_DECL);
			return Mem::BVOID;
		case INTEGER_DECL:
			token_stream.match(INTEGER_DECL);
			return Mem::INT;
		case REAL_DECL:
			token_stream.match(REAL_DECL);
			return Mem::DOUBLE;
		case BOOL_DECL:
			token_stream.match(BOOL_DECL);
			return Mem::BOOL;
		case STRING_DECL:
			token_stream.match(STRING_DECL);
			return Mem::STRING;
		case ID:
		{
			auto info = Mer::this_namespace->sl_table->find(Id::get_value(token_stream.this_token()));
			if (info == nullptr)
				throw Error("id: " + Id::get_value(token_stream.this_token()) + "no found");
			if (info->es == ESymbol::SCONTAINER)
			{

			}
			token_stream.next();
			return info->get_type();
		}
		default:
			throw Error("not finished yet");
		}
	}
	Mer::ParserNode* Parser::make_var()
	{
		token_stream.match(MAKE);
		token_stream.match(LT);
		type_code_index type_c = Mem::get_type_code();
		token_stream.match(GT);
		if (token_stream.this_tag() == LPAREN)
		{
			return Parser::parse_initializer(type_c);
		}
		return new MakeDefault(type_c);
	}

	WordRecorder* Parser::get_current_info()
	{
		return this_namespace->sl_table->find(Mer::Id::get_value(token_stream.this_token()));
	}

	NamePart::NamePart()
	{
		if (token_stream.this_tag() == MUL)
		{
			pointer = true;
			token_stream.match(MUL);
		}
		id = token_stream.this_token();
		token_stream.match(ID);
		while (token_stream.this_tag() == LSB)
		{
			arr = true;
			token_stream.match(LSB);
			if (token_stream.this_tag() == RSB)
			{
				token_stream.match(RSB);
				auto_array = true;
				return;
			}
			auto c = token_stream.this_token();
			// get element count
			int ele_count = (size_t)Integer::get_value(c);
			count += ele_count;
			array_indexs.push_back(ele_count);
			token_stream.match(INTEGER);
			token_stream.match(RSB);
		}
		// if it is multidimensional array, count the size it needs to reserve
		if (array_indexs.size() > 1) {
			int f = array_indexs[0];
			for (int i = 1; i < array_indexs.size(); i++) {
				f *= array_indexs[i];
			}
			count = f + 1;
		}

	}


	VarDeclUnit::VarDeclUnit(type_code_index t) :type_code(t)
	{
		NamePart name_part;
		id = name_part.get_id();
		is_arr = name_part.is_array();
		if (name_part.is_pointer())
		{
			is_p = true;
			type_code++;
		}
		// init a var by it's initializer
		if (token_stream.this_tag() == LPAREN)
		{
			expr = Parser::parse_initializer(type_code);
			return;
		}
		// manage to process array , the front of an array is an info obj which records the array's elemens's type and length and pos;
		if (name_part.is_array())
		{
			array_indexs = name_part.array_indexs;
			// if the array's size is deduced by init_list.
			if (name_part.is_auto_array())
			{
				token_stream.match(ASSIGN);
				auto right_value = Parser::linearized_array(t);
				array_indexs = right_value.first;
				size = right_value.second.size() + 1;
				// set initlist 
				InitList* tmp = new InitList;
				tmp->exprs() = right_value.second;
				tmp->type = type_code;
				tmp->size = right_value.second.size();
				//=====
				expr = tmp;
			}
			else
			{
				size = name_part.get_count();
				if (token_stream.this_tag() == ASSIGN)
				{
					token_stream.match(ASSIGN);
					auto right_value = Parser::linearized_array(t);
					/*
						One-dimensional array can be init by one element for example:
					*/
					if (array_indexs.size() == 1 && right_value.first.size() == 1 && right_value.first.front() == 1) {
						for (int j = 0; j < array_indexs.front() - 1; j++) {
							right_value.second.push_back(new LConV(Mem::create_var_t(type_code), type_code));
						}
					}
					// common condition check the dimension and size
					else for (int i = 0; i < right_value.first.size(); i++) {

						if (i >= array_indexs.size()|| right_value.first[i] > array_indexs[i])
							throw Error("init list overflow!");
						if (right_value.first[i] < array_indexs[i])
						{
							int len = array_indexs[i] - right_value.first[i];
							for (int j = 0; j < len;j++)
								right_value.second.push_back(new LConV(Mem::create_var_t(type_code),type_code));
						}
					}
					// set initlist 
					InitList* tmp = new InitList;
					tmp->exprs() = right_value.second;
					tmp->type = type_code;
					tmp->size = right_value.second.size();
					//=====
					expr = tmp;

				}
				// array default init
				else {
					expr = new EmptyList(type_code, size - 1);
				}
			}
			return;
		}
		// try to find if the type is a structure type
		if (type_name_mapping.find(type_code) != type_name_mapping.end())
		{
			Mer::UStructure* result = find_ustructure_t(type_code);
			if (token_stream.this_tag() == BEGIN)
				expr = new StructureInitList(result, type_code);
			else if (token_stream.this_tag() == ASSIGN)
			{
				token_stream.match(ASSIGN);
				expr = Expr(type_code).root();
				if (expr->get_type() != type_code)
					throw Error("::VarDeclUnit::VarDeclUnit(size_t t): type not matched, from " + std::to_string(type_code) + " to " + std::to_string(expr->get_type()));
			}
			else
			{
				// if the size is 1, it will be regarded as a single variable.
				this->size = 1+result->get_size();
				auto init_lists = new InitList();
				for (auto a : result->init_vec)
					init_lists->exprs().push_back(new LConV(a->clone(), a->get_type()));
				expr = init_lists;
			}
			return;
		}
		auto type_info = Mem::type_map.find(t);
		if (type_info == Mem::type_map.end())
			throw Error("unknown type " + std::to_string(t));
		if (token_stream.this_tag() == ASSIGN)
		{
			token_stream.match(ASSIGN);
			// container init list
			if (token_stream.this_tag() == BEGIN && type_info->second->type_kind == Mem::Type::kind::container)
			{
				type_code_index element_type = Mem::demerge(type_code).second;
				auto ilist = new InitList(element_type);

				auto result = type_init_function_map.find(InitKey(t, { Mem::INIT_LIST }));
				if (result == type_init_function_map.end())
				{
					delete ilist;
					throw Error("type " + type_to_string(t) + " can not initialized by list");
				}
				expr = new FunctionCall(result->second, { ilist });
				return;
			}
			// common condition 
			expr = Expr(type_code).root();
			if (expr->get_type() != type_code)
				expr = new Cast(expr, type_code);
			return;
		}
		// container var decl
		if (type_info->second->type_kind == Mem::Type::kind::container)
		{
			expr = new LConV(Mem::create_var_t(type_code), type_code);
			return;
		}
		expr = new LConV(Mem::create_var_t(type_code), type_code);
	}

	inline void _record_id(VarDeclUnit* var_unit, type_code_index type, size_t pos)
	{
		if (var_unit->arr())
		{
			auto array_id_recorder = new ArrayRecorder(type, pos, var_unit->array_indexs);
			array_id_recorder->count = var_unit->get_size() - 1;
			this_namespace->sl_table->push(Id::get_value(var_unit->get_id()), array_id_recorder);
		}
		else if (var_unit->pointer())
			this_namespace->sl_table->push(Id::get_value(var_unit->get_id()), new VarIdRecorder(type + 1, pos));
		else
			this_namespace->sl_table->push(Id::get_value(var_unit->get_id()), new VarIdRecorder(type, pos));
	}

	inline void _record_glo_id(VarDeclUnit* var_unit, type_code_index type, size_t pos)
	{
		if (var_unit->arr())
		{
			auto glo_arr_id_recorder = new GArrayRecorder(type, pos, var_unit->array_indexs);
			glo_arr_id_recorder->count = var_unit->get_size() - 1u;
			this_namespace->sl_table->push_glo(Id::get_value(var_unit->get_id()), glo_arr_id_recorder);
		}
		else if (var_unit->pointer())
			this_namespace->sl_table->push_glo(Id::get_value(var_unit->get_id()), new GVarIdRecorder(type + 1, pos));
		else
			this_namespace->sl_table->push_glo(Id::get_value(var_unit->get_id()), new GVarIdRecorder(type, pos));
	}

	LocalVarDecl::LocalVarDecl(std::vector<VarDeclUnit*>& vec, type_code_index t) :type(t)
	{
		for (const auto& a : vec)
			sum += a->get_size();
		pos = mem.push(sum) - sum;
		size_t tmp_pos = pos;
		// the var may be array ,pointer or a common var.
		_record_id(vec[0], type, pos);
		process_unit(vec[0], tmp_pos);
		for (size_t i = 1; i < vec.size(); i++)
		{
			_record_id(vec[i], type, tmp_pos += vec[i - 1]->get_size());
			process_unit(vec[i], tmp_pos+mem.get_current());
		}
		// clear unit 
		for (auto it : vec)
			delete it;
	}

	Mem::Object LocalVarDecl::execute()
	{
		for (int i = 0; i < sum; i++) {
			mem[mem.get_current() + pos + i] = exprs[i]->execute()->clone();
		}
		return Mem::Object(nullptr);
	}

	void LocalVarDecl::process_unit(VarDeclUnit* a, size_t c_pos)
	{
		if (a->get_size()!=1)
		{
			std::vector<ParserNode*> arr;
			auto exprs_info = a->get_expr();
			if (typeid(*exprs_info) == typeid(InitList))
				arr = static_cast<InitList*>(a->get_expr())->exprs();
			else
				arr = static_cast<EmptyList*>(a->get_expr())->exprs();
			// the info of the array.
			auto array_info = new LConV(std::make_shared<Mem::Array>(type, c_pos, arr.size()), type);
			exprs.push_back(std::unique_ptr<LConV>(array_info));
			for (auto it : arr)
			{
				exprs.push_back(UptrPNode(it));
			}
			delete a->get_expr();
		}
		else {
			exprs.push_back(UptrPNode(a->get_expr()));
		}
	}

	GloVarDecl::GloVarDecl(std::vector<VarDeclUnit*>& vec, type_code_index t) :type(t)
	{
		for (const auto& a : vec)
		{
			sum += a->get_size();
		}
		pos = mem.reserve_glo_pos(sum) - sum;

		size_t tmp_pos = pos;
		_record_glo_id(vec[0], type, pos);
		process_unit(vec[0], pos);
		// parse every var_unit;
		for (size_t i = 1; i < vec.size(); i++)
		{
			_record_glo_id(vec[i], type, tmp_pos += vec[i - 1]->get_size());
			process_unit(vec[i], tmp_pos);
		}
		for (auto a : vec)
		{
			delete a;
		}
	}


	Mem::Object GloVarDecl::execute()
	{
		for (int i = 0; i < sum; i++) {
			mem[pos + (size_t)i] = exprs[i]->execute()->clone();
		}
		return nullptr;
	}

	void GloVarDecl::process_unit(VarDeclUnit* a, size_t c_pos)
	{
		if (a->arr())
		{
			std::vector<ParserNode*> arr;
			auto exprs_info = a->get_expr();
			if (typeid(*exprs_info) == typeid(InitList))
				arr = static_cast<InitList*>(a->get_expr())->exprs();
			else
				arr = static_cast<EmptyList*>(a->get_expr())->exprs();
			// the info of the array.
			auto array_info = new LConV(std::make_shared<Mem::GArray>(type, c_pos, arr.size()), type);
			exprs.push_back(std::unique_ptr<LConV>(array_info));
			for (auto it : arr)
				exprs.push_back(UptrPNode(it));
			delete a->get_expr();
		}
		else {
			exprs.push_back(UptrPNode(a->get_expr()));
		}
	}


	Cast::Cast()
	{
		token_stream.match(CAST);
		token_stream.match(LT);
		to_type = Mem::get_type_code(token_stream.this_token());
		token_stream.next();
		token_stream.match(GT);
		token_stream.match(LPAREN);
		expr = Expr().root();
		token_stream.match(RPAREN);
	}


	Mem::Object Cast::execute()
	{
		return expr->execute()->Convert(to_type);
	}
	ArrayInitList::ArrayInitList(const std::vector<ParserNode*> _leaves) :leaves(_leaves)
	{
		type_code_index tyc = _leaves.front()->get_type();
		for (auto a : _leaves) {
			if (a->get_type() != tyc)
				throw Error("array initlist type not matched from " + type_to_string(tyc) + " to " + type_to_string(a->get_type()));
		}
	}
	ArrayInitList::ArrayInitList(const std::vector<ArrayInitList*> _children) :children(_children) {
		size_t cnt = children.front()->size();
		for (auto a : children) {
			if (a->size() != cnt)
				throw Error("invalid array init list");
		}
	}
	Mem::Object ConditionalOperator::execute()
	{
		bool cond = std::static_pointer_cast<Mem::Bool>(condition->execute()->Convert(Mem::BOOL))->_value();
		if (cond)
			return true_expr->execute();
		return false_expr->execute();
	}
}