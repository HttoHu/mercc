/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include "value.hpp"
#include <functional>
namespace Mer
{
	bool compare_param_feature(const std::vector<type_code_index>& p1, const std::vector<type_code_index>& p2);

	enum ESymbol
	{
		SFUN, SVAR, SNAME,
		SGVAR, SGARR, SENUM, SENUM_MEMBER,
		SSTRUCTURE, SARRAY,
		STYPE, USVAR, MVAR,
		SCONTAINER,
	};
	struct WordRecorder
	{
	public:
		WordRecorder(ESymbol e, type_code_index tc = 0) :es(e), type_code(tc) {}
		ESymbol es;
		int count = 1;
		virtual size_t get_pos() { return 0; }
		virtual std::string to_string() { return""; }
		type_code_index get_type() { return type_code; }
		virtual ~WordRecorder() {}
	protected:
		type_code_index type_code;
	};
	struct VarIdRecorder :public Mer::WordRecorder
	{
	public:
		VarIdRecorder(type_code_index type, size_t p, WordRecorder wr = SVAR)
			:WordRecorder(wr), pos(p)
		{
			type_code = type;
		}
		size_t pos;
		size_t get_pos()override {
			return pos + mem.get_current();
		}
		std::string to_string()override
		{
			return "pos:" + std::to_string(pos);
		}
	private:
	};
	struct ArrayRecorder :public Mer::WordRecorder {
	public:
		ArrayRecorder(type_code_index type, size_t p, const std::vector<size_t>& arr_indxs) :
			WordRecorder(SARRAY), pos(p), array_indexs(arr_indxs) {
			type_code = type;
		}
		size_t get_pos()override {
			return pos + mem.get_current();
		}
		std::string to_string()override
		{
			return "pos:" + std::to_string(pos);
		}
		size_t pos;
		std::vector<size_t> array_indexs;
	};
	struct MVarRecorder :public WordRecorder
	{
		// if count is 2 means it is an array.
		MVarRecorder(type_code_index type, size_t _pos) :pos(_pos), WordRecorder(MVAR, type) {}
		size_t get_pos()override { return pos; }
		std::string to_string()override { return "MVar :" + std::to_string(pos); }
		~MVarRecorder() {  }
		size_t pos;
	};
	struct StructRecorder :public WordRecorder
	{
	public:
		StructRecorder() :WordRecorder(ESymbol::SSTRUCTURE)
		{
			Mem::type_counter += 2;
			type_code = Mem::type_counter;
		}

	};
	class Namespace;
	struct GArrayRecorder :public Mer::WordRecorder {
	public:
		GArrayRecorder(type_code_index type, size_t p, const std::vector<size_t>& arr_indxs) :
			WordRecorder(SGARR), pos(p), array_indexs(arr_indxs) {
			type_code = type;
		}
		size_t get_pos()override {
			return pos;
		}
		std::string to_string()override
		{
			return "gpos:" + std::to_string(pos);
		}
		size_t pos;
		std::vector<size_t> array_indexs;
	};
	struct GVarIdRecorder :public WordRecorder
	{
	public:
		GVarIdRecorder(type_code_index t, size_t _pos, ESymbol e = SGVAR) :WordRecorder(e, t), pos(_pos) {
		}
		bool& is_arr() { return arr; }
		size_t get_pos()override
		{
			return pos;
		}
		Mem::Object& get_value()
		{
			return mem[pos];
		}
		size_t pos;
		bool arr = false;
	};
	struct FuncIdRecorder :public WordRecorder
	{
	public:
		FuncIdRecorder(type_code_index type) :WordRecorder(ESymbol::SFUN), functions(compare_param_feature) {
			type_code = type;
		}
		FunctionBase* find(const std::vector<type_code_index>& pf);
		bool dnt_check = false;
		FuncIdRecorder(FunctionBase* fb);
		std::map<std::vector<type_code_index>, FunctionBase*, decltype(compare_param_feature)*> functions;

		~FuncIdRecorder();
	private:

	};
	struct ContainerTypeRecorder :public WordRecorder
	{
	public:
		ContainerTypeRecorder(const std::string& str) :WordRecorder(SCONTAINER)
		{
			type_code = Mem::type_counter += 2;
		}
		std::string to_string() override {
			return "type:" + type_name;
		}
	private:
		std::string type_name;
	};
	class SymbolTable
	{
	public:
		SymbolTable()
		{
			data.push_front(std::map<std::string, WordRecorder*>());
		}
		void new_block()
		{
			data.push_front(std::map<std::string, WordRecorder*>());
		}
		void end_block();
		void type_check(Token* id, ESymbol e)
		{
			auto result = find(Id::get_value(id));
			if (result == nullptr)
				throw Error("id " + id->to_string() + " not found.");
			if (result->es != e)
				throw Error("id type not matched");
		}
		WordRecorder* find_front(std::string id)
		{
			auto result = data.front().find(id);
			if (result == data.front().end())
				return nullptr;
			return result->second;
		}
		WordRecorder* find(std::string id);
		void push_glo(std::string id, WordRecorder* wr);
		void push(std::string id, WordRecorder* wr);
		void print();
		~SymbolTable();
	private:
		std::vector<WordRecorder*> _rem_vec;
		std::deque<std::map<std::string, WordRecorder*>> data;
	};
}
