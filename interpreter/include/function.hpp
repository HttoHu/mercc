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
#include "expr.hpp"
namespace Mer
{
	// arguments types list
	using _compare_operator =std::function<bool(Mem::Object,Mem::Object)>;
	using ParamFeature = std::vector<type_code_index>;
	using _intern_func = std::function<Mem::Object (const std::vector<Mem::Object>&)>;
	bool compare_param_feature(const std::vector<type_code_index>& p1, const std::vector<type_code_index>& p2);
	std::string param_feature_to_string(const ParamFeature& pf);
	class Param
	{
	public:
		bool type_check(const std::vector<type_code_index> &types);
		void push_new_param(type_code_index type, size_t pos)
		{
			arg_pos.push_back({ type,pos });
		}
		std::vector<std::pair<type_code_index, size_t>> &get_param_table() { return arg_pos; }
		ParamFeature get_param_feature();
	private:
		size_t param_size;
		// type + pos;
		std::vector<std::pair<type_code_index,size_t>> arg_pos;
	};
	class FunctionBase
	{
	public:
		FunctionBase();

		virtual void check_param(const std::vector<type_code_index>& types);
		void set_param_types(const std::vector<type_code_index>& types)
		{
			param_types = types;
		}
		virtual type_code_index get_type() { return 0; }
		// covert args' type in order to comply with params' type.
		virtual void run(const std::vector<Mem::Object>& objs) { }
		std::vector<type_code_index> param_types;
		bool is_check_type() { return check_param_type; }
		std::string to_string(std::string name="no_name_func")const ;
		virtual ~FunctionBase() {}

		bool is_completed = false;
	protected:
		bool check_param_type = true;

	};
	class Function : public FunctionBase
	{
	public:
		Function(type_code_index t, Param *p);
		Function(type_code_index t);
		Param *param=nullptr;
		void run(const std::vector<Mem::Object> &objs)override;
		type_code_index get_type()override { return type; }
		std::vector<UptrPNode> stmts;
		size_t* pc=new size_t(0);
		size_t off=0;
		~Function();
	private:

		size_t param_size=0;
		type_code_index type;
	};
	class SystemFunction :public FunctionBase 
	{
	public:
		SystemFunction(type_code_index t, _intern_func fun) :type(t), func(fun) {}
		void run(const std::vector<Mem::Object>& objs)override;
		type_code_index get_type()override{return type;}
		void check_param(const std::vector<type_code_index>& types)override;
		void dnt_check_param()
		{
			check_param_type = false;
		}
		~SystemFunction();
	private:
		type_code_index type;

		_intern_func func;
	};
	class InitKey
	{
	public:
		InitKey(type_code_index type_c, const std::vector<type_code_index>& param) :type_code(type_c), params(param) {}
		type_code_index type_code;
		std::vector<type_code_index> params;
		bool operator<(const InitKey& init_key)const;
	};
	namespace Parser
	{
		// build param type_code vec;
		ParamFeature build_param_feature();
		// build param which contains nameinfo and type_code;
		Param* build_param();
		void build_function(size_t rtype, std::string name);
		std::pair<std::string,Function*> _build_function();
	}
	// the set of functions which will be deleted when the program exit;
	extern std::set<FunctionBase*> rem_functions;
	extern std::map<InitKey, FunctionBase*> type_init_function_map;
	extern std::map<type_code_index, Mem::Object> type_init_map;
	extern std::map<type_code_index, _compare_operator> compare_map;
}