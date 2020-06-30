/*
		MIT License
/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include <vector>
#include "memory.hpp"
#include "parser_node.hpp"
#include "lexer.hpp"
#include "../include/parser.hpp"
namespace Mer
{
	class Namespace;
	class Expr;
	class FunctionBase;
	class StructureBase;
	// the sets of the ObjGener is to generate objects when function calls;
	// for instance f(a) -> a is a structure but it looks like a single variable and just can generate one object.
	// so we need to introduce ObjGener, which can generate many objects from a ParserNode.
	class ObjGener
	{
	public:
		// convert parser_node to obj gener.
		static ObjGener* from(ParserNode* node);
	public:
		virtual std::vector<Mem::Object> gen_obj()=0;
		virtual ~ObjGener() {}
	};

	class SingleObjGener final:public ObjGener
	{
	public:
		SingleObjGener(ParserNode* n) :node(n) {}
		std::vector<Mem::Object> gen_obj()override;
		~SingleObjGener() { delete node; }
	private:
		ParserNode* node;
	};
	class MultiObjGener final :public ObjGener
	{
	public:
		MultiObjGener(std::vector<ParserNode*> &&_nodes) :nodes(std::move(_nodes)) {}
		std::vector<Mem::Object> gen_obj()override;
		~MultiObjGener();
	private:
		std::vector<ParserNode*> nodes;
	};

	class StructObjGener final :public ObjGener
	{
	public:
		StructObjGener(type_code_index ty_c, ParserNode* _node);
		std::vector<Mem::Object> gen_obj()override;
		~StructObjGener() { delete node; }
	private:
		int len;
		ParserNode* node;
	};
	//literal-const

	class LConV :public ParserNode
	{
	public:
		LConV(Token *t);
		LConV(Mem::Object _obj, type_code_index t) :obj(_obj), type(t) {}
		type_code_index get_type()override{return type;}
		Mem::Object execute()override{return obj;}
		std::string to_string()override{return obj->to_string();}
		ParserNode* clone()override;
	private:
		LConV() {}
		type_code_index type=0;
		Mem::Object obj;
	};

	class Namespace;

	class GVar :public ParserNode
	{
	public:
		GVar(type_code_index _type, size_t off_pos) :type(_type), pos(off_pos) {}
		GVar(WordRecorder *result);
		GVar(WordRecorder* result, size_t offset);
		type_code_index get_type()override{return type;}
		size_t get_pos()override{return pos;}
		Mem::Object execute()override{return mem[pos];}
		bool global()const override { return true; }
		ParserNode* clone()override { return new GVar(type, pos); }
	private:
		type_code_index type;
		size_t pos;
	};
	class Variable final:public ParserNode
	{
	public:
		Variable(WordRecorder* wr);
		Variable(type_code_index _type, size_t _pos) :type(_type), pos(_pos) {}
		type_code_index get_type()override;
		size_t get_pos()override;
		std::string to_string()override{return "(" + type_to_string(type) + ")" + std::to_string(pos);}
		Mem::Object execute()override;
		bool& arr() { return is_arr; }
		ParserNode* clone()override;
	private:
		Variable() {}
		bool is_arr=false;
		type_code_index type=0;
		size_t pos;
	};
	// to translate structure, when a structure need to be returned.
	class TmpVar :public ParserNode
	{
	public:
		// the third argument should be cloned or only used in this scope.
		TmpVar(type_code_index ty, size_t _pos, ParserNode* _expr) :type_code(ty), pos(_pos), trans_node(_expr) {}
		size_t get_pos()override { return pos; }
		type_code_index get_type()override { return type_code; }
		ParserNode* clone() override;
		Mem::Object execute()override;
		~TmpVar() { delete trans_node; }
	private:
		ParserNode* trans_node;
		size_t pos;
		type_code_index type_code;
	};
	// To evaluate/run many nodes.
	class EvalMultiNode final:public ParserNode {
	public:
		EvalMultiNode(std::vector<ParserNode*>&& vec) :exprs(std::move(vec)) {}
		Mem::Object execute()override;
		ParserNode* clone()override { throw Error("(interpreter-error):EvalMultiNode can not be cloned."); }
		~EvalMultiNode();
	private:
		std::vector<ParserNode* > exprs;
	};
	class FunctionCall :public ParserNode
	{
	public:
		FunctionCall(FunctionBase * fun, std::vector<ParserNode*>&& exprs);
		type_code_index get_type()override;
		Mem::Object execute()override;
		std::string to_string()override;
		size_t get_pos()override { return 0; }
		ParserNode* clone()override;
		~FunctionCall();
	private:
		FunctionCall() {}
		FunctionBase * func=nullptr;
		std::vector<ObjGener*> argument;
	};
	class MemberFunctionCall :public ParserNode
	{
	public:
		MemberFunctionCall(FunctionBase* fun, std::vector<ParserNode*>& exprs,ParserNode* _p);
		type_code_index get_type()override;
		Mem::Object execute()override;
		std::string to_string()override;
		ParserNode* clone()override;
		~MemberFunctionCall();
	private:
		MemberFunctionCall() {}
		FunctionBase* func=nullptr;
		std::vector<ParserNode*> argument;
		UptrPNode parent;
		std::vector<Mem::Object> obj_vec;
	};
	/*
		the initializer of a class
	*/
	struct WordRecorder;
	template<typename ARR_TYPE>
	ParserNode* get_array_bias(Mer::WordRecorder* var_info);
	ParserNode* _make_l_conv(int n);
	namespace Parser
	{
		ParserNode* parse_glo(WordRecorder* var_info);
		ParserNode *parse_id();
		ParserNode *parse_var(WordRecorder* var_info);
		// global array and local array
		template<typename ARR_TYPE>
		ParserNode* parse_array(WordRecorder* var_info);
		// parse id with namespace.
		ParserNode *_parse_id_wn(Namespace *names);
		FunctionCall* parse_initializer(type_code_index type);
		std::vector<ParserNode*> parse_arguments();
		ParserNode *parse_function_call(Namespace *names);
		MemberFunctionCall* parse_call_by_function(FunctionBase*);
		Namespace *kill_namespaces();
	}
}