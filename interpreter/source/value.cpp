/*
*	MIT License
*	Copyright (c) 2019 Htto Hu
*/
#include "../include/memory.hpp"
#include "../include/word_record.hpp"
#include "../include/namespace.hpp"
#include "../include/environment.hpp"
#include "../include/compound_box.hpp"
namespace Mer {
	namespace optimizer
	{
		ParserNode* optimize_bin_op(ParserNode* left, ParserNode* right, Token* tok);
		ParserNode* optimize_unary_op(ParserNode* left, Token* tok);
		ParserNode* optimize_array_subscript(ParserNode* arr, ParserNode* subscript,type_code_index default_type=0);
	}

	Variable::Variable(WordRecorder* wr)
	{
		pos = wr->get_pos();
		type = wr->get_type();
		if (wr->es == SARRAY)
			is_arr = true;
	}

	type_code_index Variable::get_type()
	{
		return type;
	}

	size_t Variable::get_pos() {
		return pos + mem.get_current();
	}

	Mem::Object Variable::execute()
	{
		return mem[mem.get_current() + pos];
	}

	ParserNode* Variable::clone()
	{
		auto ret = new Variable;
		ret->is_arr = is_arr;
		ret->type = type;
		ret->pos = pos;
		return ret;
	}

	FunctionCall::FunctionCall(FunctionBase* _func, std::vector<ParserNode*>&& exprs) : func(_func)
	{
		if (!_func->is_check_type())
			;
		else for (int i = 0; i < argument.size(); i++)
		{
			type_code_index arg_type = exprs[i]->get_type(), param_type = func->param_types[i];
			if (arg_type != param_type)
			{
				// common ptr can convert to void ptr
				if (arg_type == Mem::BVOID + 1 && param_type % 2 == 0)
					continue;
				else
					// type convert
					exprs[i] = new Cast(exprs[i], param_type);
			}
		}
		for (auto it : exprs)
			argument.push_back(ObjGener::from(it));
	}

	type_code_index FunctionCall::get_type()
	{
		return func->get_type();
	}

	Mem::Object FunctionCall::execute()
	{
		std::vector<Mem::Object> tmp;
		for (auto it : argument)
		{
			auto tmp_vec = it->gen_obj();
			tmp.insert(tmp.end(), tmp_vec.begin(), tmp_vec.end());
		}
		return func->run(tmp);
	}

	std::string FunctionCall::to_string()
	{
		std::string str = "function()";
		return str;
	}

	ParserNode* FunctionCall::clone()
	{
		throw Error("Can not clone");
		//return ret;
	}

	FunctionCall::~FunctionCall()
	{
		for (auto a : argument)
			delete a;
	}


	ParserNode* Parser::parse_id()
	{
		auto id = token_stream.this_token();
		auto result = this_namespace->sl_table->find(Id::get_value(id));
		if (result == nullptr)
		{
			auto target_namespace = kill_namespaces();
			if (target_namespace == nullptr)
				throw Error("Merdog is broken");
			return _parse_id_wn(target_namespace);
		}
		switch (result->es)
		{
			//type convert;
			//type(...) form, (type)epxr has processed within Expr::factor();
		case ESymbol::STYPE:
		{
			//skip type name
			token_stream.next();
			token_stream.match(LPAREN);
			auto expr = Expr().root();
			token_stream.match(RPAREN);
			return new Cast(expr, Mem::get_type_code(id));
		}
		case ESymbol::SENUM_MEMBER:
			token_stream.next();
			return new LConV(std::make_shared<Mem::Int>(result->count), Mem::INT);
		case ESymbol::SGVAR:
			return parse_glo(result);
		case ESymbol::SENUM:
		case ESymbol::SSTRUCTURE:
			if (token_stream.next_token()->get_tag() == BEGIN)
			{
				token_stream.next();
				return parse_struct_init_list(result->get_type());
			}
		case ESymbol::SCONTAINER:
			return var_decl();
		case ESymbol::SFUN:
			return parse_function_call(this_namespace);
		case ESymbol::SGARR:
			return parse_array<GArrayRecorder>(result);
		case ESymbol::SARRAY:
			return parse_array<ArrayRecorder>(result);
		case ESymbol::SVAR:
			return parse_var(result);
		default:
		{
			auto target_namespace = kill_namespaces();
			if (target_namespace == nullptr)
				throw Error("unsupported id type");
			return _parse_id_wn(target_namespace);
		}
		}
	}

	ParserNode* Parser::parse_var(WordRecorder* var_info)
	{
		Token* var_id = token_stream.this_token();
		token_stream.match(ID);
		auto ret = new Variable(var_info);
		return ret;
	}


	ParserNode* _make_l_conv(int n) {
		return new LConV(std::make_shared< Mem::Int>(n), Mem::BasicType::INT);
	}
	template<typename ARR_TYPE>
	ParserNode* get_array_bias(WordRecorder* var_info)
	{
		int type_len = 1;
		type_code_index ele_type = var_info->get_type();
		if (is_a_structure_type(ele_type))
			type_len = find_ustructure_t(ele_type)->get_size();;
		auto array_indexs = static_cast<ARR_TYPE*>(var_info)->array_indexs;
		std::vector<ParserNode*> indexs;
		// if the following of the array is not [, then the array will decay to a pointer which points to the first elements
		if (token_stream.this_tag() != Tag::LSB)
			if (typeid(ARR_TYPE) == typeid(ArrayRecorder))
				return new ArrayDecay(var_info->get_pos(), ele_type + 1);
			else
				return new GloArrayDecay(var_info->get_pos(), ele_type + 1);
		while (token_stream.this_tag() == Tag::LSB) {
			token_stream.match(LSB);
			indexs.push_back(Expr().root());
			token_stream.match(RSB);
		}
		// obtain the a index from the indexs
		ParserNode* ret = _make_l_conv(0);
		for (int i = 0; i < indexs.size() - 1; i++) {
			int p = type_len;
			for (int j = 0; j <= i; j++)
			{
				p *= array_indexs[indexs.size() - 1 - i];
			}
			ParserNode* tmp = optimizer::optimize_bin_op(indexs[i], _make_l_conv(p), BasicToken["*"]);
			ret = optimizer::optimize_bin_op(ret, tmp, BasicToken["+"]);
		}
		if (type_len != 1)
			indexs.back() = optimizer::optimize_bin_op(_make_l_conv(type_len), indexs.back(),BasicToken["*"]);
		ret = optimizer::optimize_bin_op(ret, indexs.back(), BasicToken["+"]);
		
		return ret;
	}
	template<typename ARR_TYPE>
	ParserNode* Parser::parse_array(WordRecorder* var_info)
	{
		token_stream.match(ID);
		auto ret = get_array_bias<ARR_TYPE>(var_info);
		if (typeid(*ret) == typeid(ArrayDecay)||typeid(*ret)== typeid(GloArrayDecay))
			return ret;
		if (typeid(ARR_TYPE) == typeid(ArrayRecorder))
			return optimizer::optimize_array_subscript(new Variable(var_info), ret);
		else
			return optimizer::optimize_array_subscript(new GVar(var_info), ret);
	}



	ParserNode* Parser::_parse_id_wn(Namespace* names)
	{
		auto id = token_stream.this_token();
		auto result = names->sl_table->find(Id::get_value(id));
		if (result == nullptr)
			throw Error(id->to_string() + " no found");
		switch (result->es)
		{
		case ESymbol::SFUN:

			return parse_function_call(names);
		case ESymbol::SGVAR:
		{
			return parse_glo(result);
		}
		default:
			throw Error("Unsupported type " + id->to_string());
		}
	}

	FunctionCall* Parser::parse_initializer(type_code_index type)
	{
		std::vector<ParserNode*> exprs = parse_arguments();
		// get the args' type to find init function
		// cos the init function can be overloaded by the different param
		std::vector<type_code_index> args_type;
		for (auto a : exprs)
		{
			args_type.push_back(a->get_type());
		}
		auto result = type_init_function_map.find(InitKey(type, args_type));

		std::string err_msg = "(";
		// get the information of args
		for (size_t i = 0; i < args_type.size(); i++)
		{
			err_msg += type_to_string(args_type[i]);
			if (i != args_type.size() - 1)
				err_msg += ",";
		}
		err_msg += ")";

		if (result == type_init_function_map.end())
			throw Error("type:" + type_to_string(type) + " don't support initializer " + err_msg);
		auto func = result->second;

		return new FunctionCall(func, std::move(exprs));
	}

	std::vector<ParserNode*> Parser::parse_arguments()
	{
		std::vector<ParserNode*> exprs;
		token_stream.match(LPAREN);
		if (token_stream.this_tag() == RPAREN)
		{
			token_stream.match(RPAREN);
			return exprs;
		}
		auto param_unit = Expr().root();
		exprs.push_back(param_unit);
		while (token_stream.this_tag() == COMMA)
		{
			token_stream.match(COMMA);
			auto param_unit2 = Expr().root();
			exprs.push_back(param_unit2);
		}
		token_stream.match(RPAREN);
		return exprs;
	}

	extern std::vector<UptrPNode>* current_ins_table ;
	ParserNode* Parser::parse_function_call(Namespace* names)
	{
		
		std::string func_name = Id::get_value(token_stream.this_token());
		// to check the param's type.

		token_stream.match(ID);
		std::vector<ParserNode*> exprs = parse_arguments();
		ParamFeature pf;
		for (auto a : exprs)
			pf.push_back(a->get_type());
		auto result = names->sl_table->find(func_name);
		auto recorder = static_cast<FuncIdRecorder*>(result);
		auto func = recorder->find(pf);
		if (func == nullptr)
		{
			throw Error("function " + func_name + param_feature_to_string(pf) + " not found its defination");
		}
		current_ins_table->push_back(std::make_unique <FunctionCall>(func, std::move(exprs)));
		return new GVar(func->get_type(), 0);
	}

	MemberFunctionCall* Parser::parse_call_by_function(FunctionBase* f)
	{
		std::vector<ParserNode*> exprs;
		// to check the param's type.
		//get ref of parent
		auto tmp = parse_arguments();
		exprs.insert(exprs.end(), tmp.begin(), tmp.end());
		return new MemberFunctionCall(f, exprs, structure_parent_stack.back());
	}

	Namespace* Parser::kill_namespaces()
	{
		Namespace* current = this_namespace;
		auto result = _find_namespace_driver(current, Id::get_value(token_stream.this_token()));
		while (result != nullptr)
		{
			current = result;
			token_stream.match(ID);
			token_stream.match(DOT);
			result = _find_namespace_driver(current, Id::get_value(token_stream.this_token()));
		}
		return current;
	}

	LConV::LConV(Token* t)
	{
		switch (t->get_tag())
		{
		case TTRUE:
			type = Mem::BOOL;
			obj = std::make_shared<Mem::Bool>(true);
			break;
		case TFALSE:
			type = Mem::BOOL;
			obj = std::make_shared<Mem::Bool>(false);
			break;
		case INTEGER:
			type = Mem::INT;
			obj = std::make_shared<Mem::Int>(Integer::get_value(t));
			break;
		case REAL:
			type = Mem::DOUBLE;
			obj = std::make_shared <Mem::Double >(Real::get_value(t));
			break;
		case STRING:
			type = Mem::STRING;
			obj = std::make_shared<Mem::String>(String::get_value(t));
			break;
		case CHAR_LIT:
			type = Mem::CHAR;
			obj = std::make_shared<Mem::Char>(CharToken::get_value(t));
			break;
		default:
			throw Error("syntax error");
		}
	}

	ParserNode* LConV::clone()
	{
		auto ret = new LConV;
		ret->type = type;
		ret->obj = obj->clone();
		return ret;

	}

	GVar::GVar(WordRecorder* result)
	{
		pos = static_cast<GVarIdRecorder*>(result)->pos;
		type = result->get_type();
	}

	GVar::GVar(WordRecorder* result, size_t offset)
	{
		pos = static_cast<GVarIdRecorder*>(result)->pos + offset;
		type = result->get_type();
	}

	// the same function of parse_member but used to parse glo var
	ParserNode* Parser::parse_glo(WordRecorder* var_info)
	{
		// Similar to parse_array;
		auto id_name = token_stream.this_token();
		token_stream.match(ID);
		return new GVar(var_info);
	}

	MemberFunctionCall::MemberFunctionCall(FunctionBase* _func, std::vector<ParserNode*>& exprs, ParserNode* _p) : parent(_p), func(_func), argument(exprs), obj_vec(exprs.size() + 1)
	{
		std::vector<type_code_index> type_vec;
		for (auto& a : exprs)
		{
			type_vec.push_back(a->get_type());
		}
		func->check_param(type_vec);
	}

	type_code_index MemberFunctionCall::get_type()
	{
		return func->get_type();
	}

	Mem::Object MemberFunctionCall::execute()
	{
		parents_vec.push_back(parent->execute());
		auto sz = argument.size();
		for (size_t i = 0; i < sz; i++)
		{
			obj_vec[i] = argument[i]->execute()->clone();
		}
		auto ret = func->run(obj_vec);
		parents_vec.pop_back();
		return ret;
	}

	std::string MemberFunctionCall::to_string()
	{
		std::string str = "function:";

		str += "(";
		for (auto& a : argument)
			str += a->to_string();
		str += ")";
		return str;
	}

	ParserNode* MemberFunctionCall::clone()
	{
		auto ret = new MemberFunctionCall;
		for (auto a : argument)
			ret->argument.push_back(a->clone());
		ret->func = func;
		ret->parent = UptrPNode(parent->clone());
		return ret;
	}

	MemberFunctionCall::~MemberFunctionCall()
	{
		for (auto a : argument)
			delete a;
	}
	ParserNode* TmpVar::clone()
	{
		return new TmpVar(type_code, pos, trans_node->clone());
	}
	Mem::Object TmpVar::execute()
	{
		return Mer::mem[pos] = trans_node->execute();
	}
	Mem::Object EvalMultiNode::execute()
	{
		for (auto a : exprs)
			a->execute();
		return nullptr;
	}
	EvalMultiNode::~EvalMultiNode()
	{
		for (auto a : exprs)
			delete a;
	}
	std::vector<Mem::Object> MultiObjGener::gen_obj()
	{
		std::vector<Mem::Object> ret;
		for (auto it : nodes)
			ret.push_back(it->execute());
		return ret;
	}
	MultiObjGener::~MultiObjGener()
	{
		for (auto it : nodes)
			delete it;
	}
	StructObjGener::StructObjGener(type_code_index ty_c, ParserNode* _node):node(_node)
	{
		len = find_ustructure_t(ty_c)->get_size();
	}
	std::vector<Mem::Object> StructObjGener::gen_obj()
	{
		std::vector<Mem::Object> ret(len);
		int sp = node->get_pos();
		for (int i = 0; i < len; i++)
		{
			ret[i] = mem[sp + i]->clone();
		}
		return ret;
	}
	ObjGener* ObjGener::from(ParserNode* node)
	{
		// too many typeid to ascertain the type of node...
		// the work is finished by parsing phase... so don't worry about the performance.
		if (typeid(*node) == typeid(InitList))
			return new MultiObjGener(std::move(static_cast<InitList*>(node)->exprs()));
		else if (typeid(*node) == typeid(EmptyList))
			return new MultiObjGener(std::move(static_cast<EmptyList*>(node)->exprs()));
		else if (is_a_structure_type(node->get_type()))
			return new StructObjGener(node->get_type(), node);
		else
			return new SingleObjGener(node);
	}
}