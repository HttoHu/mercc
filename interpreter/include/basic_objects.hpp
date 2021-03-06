/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
/*
*   note that : obj refers to the instance of Mem::Int or Mem::String, Mem::Bool etc.
*	The Object is a shared_ptr<Value> which is the parent of all objs.
*	special usage: When we need to get the raw of a Mem::xxx (e.g. Mem::Int) we can use get_raw<int>(obj) ),
*	I supposed that the obj memory structure is like that
*	| --- virtual_table_pointer(or some other value about parent : x64(8Byte), x86(4Byte) ---|
*	| int or string , bool, real according the obj type																|
*	so move 8/4 Byte could get the raw value of the obj.
*/
#pragma once
#include <string>
#include <set>
#include <map>
#include <memory>
#include <vector>
#include "error.hpp"
#include "type.hpp"
#include "clib/any.hpp"
using type_code_index = int;
const int BASICTYPE_MAX_CODE = 13;
namespace Mer
{
	class StructureBase;
	class Namespace;
	class Token;
	std::string type_to_string(size_t type_code);
	namespace Mem
	{

		class Type;
		class Value;

		extern Namespace* this_namespace;
		using Object = std::shared_ptr<Value>;
		void swap(Object rhs, Object lhs);
		class Value
		{
		public:
			virtual Object clone()const { return nullptr; }
			virtual std::string to_string()const { return ""; }
			virtual type_code_index get_type()const
			{
				return BasicType::NDEF;
			}
			virtual size_t length()const { return 0; }
			virtual char *get_raw_data() {
				return nullptr;
			}
			virtual Object operator=(const Object & v)
			{
				return nullptr;
			}
			virtual Object operator+=(const Object & v)
			{
				throw Error("+=: syntax error");
			}
			virtual Object operator-=(const Object & v)
			{
				throw Error("-=: syntax error");
			}
			virtual Object operator*=(const Object & v)
			{
				throw Error("*=: syntax error");
			}
			virtual Object operator/=(const Object & v)
			{
				throw Error("/=: syntax error");
			}
			virtual Object operator+ (const Object & v) { throw Error("+: syntax error"); }
			virtual Object operator- (const Object & v) { throw Error("-: syntax error"); }
			virtual Object operator* (const Object & v) { throw Error("* syntax error"); }
			virtual Object operator/ (const Object & v) { throw Error("/: syntax error"); }
			virtual Object Convert(type_code_index type) { throw Error("Convert Error"); }
			virtual Object get_negation()
			{
				throw Error("get_negation: runtime error");
			}
			virtual Object operator<< (const Object& v) { throw Error("<<: runtime error!"); }
			virtual Object operator>> (const Object& v) { throw Error(">>: runtime error!"); }
			virtual Object operator&(const Object& v) { throw Error("& :runtime error!"); }
			virtual Object operator|(const Object& v) { throw Error("|: runtime error!"); }
			virtual Object operator^(const Object& v) { throw Error("^: runtime error!"); }
			virtual Object operator<<= (const Object& v) { throw Error("<<=: runtime error!"); }
			virtual Object operator>>= (const Object& v) { throw Error(">>=: runtime error!"); }
			virtual Object operator&=(const Object& v) { throw Error("&=:runtime error!"); }
			virtual Object operator|=(const Object& v) { throw Error("|=: runtime error!"); }
			virtual Object operator^=(const Object& v) { throw Error("^=: runtime error!"); }
			virtual Object operator>(const Object & v) { throw Error(">: runtime error"); }
			virtual Object operator<(const Object & v) { throw Error("<: runtime error"); }
			virtual Object operator>=(const Object & v) { throw Error(">=: runtime error"); }
			virtual Object operator<=(const Object & v) { throw Error("<=: runtime error"); }
			virtual Object operator !=(const Object & v) { throw Error("!=: runtime error"); }
			virtual Object operator ==(const Object & v) { throw Error("==: runtime error"); }
			virtual Object operator[](const Object & v) { throw Error("[]: runtime error"); }
			virtual Object operator&& (const Object & v) { throw Error("&&: runtime error"); }
			virtual Object operator|| (const Object & v) { throw Error("OR: runtime error"); }
			virtual ~Value() { }
		private:
		};
		std::string type_to_string(BasicType bt);
		//get type

		Mem::Object create_var_t(type_code_index type);
		// every obj extends Value, and override some common operator
		// size_t get_type() to get the obj's type
		// clone , copy the obj and return it;
		// Convert(BasicType), to convert the Obj to another compatible type obj.
		class Int;
		class Bool :public Value
		{
		public:
			Bool(bool b) :value(b) {}
			Object operator=(const Object & v)override
			{
				auto tmp = v;
				value = std::static_pointer_cast<Bool>(v)->value;
				return tmp;
			}
			std::string to_string()const override
			{
				if (value)
					return "true";
				return "false";
			}
			type_code_index get_type()const override
			{
				return BasicType::BOOL;
			}
			size_t length()const override { return sizeof(bool); }
			char *get_raw_data() override {
				return (char*)(&value);
			}

			Object Convert(type_code_index type)override;
			Object get_negation()override
			{
				return std::make_shared<Mem::Bool>(!value);
			}
			Object operator==(const Object & v)override
			{
				return std::make_shared<Mem::Bool>(value == std::static_pointer_cast<Bool>(v)->value);
			}
			Object operator!=(const Object & v)override
			{
				return std::make_shared<Mem::Bool>(value != std::static_pointer_cast<Bool>(v)->value);
			}
			Object operator&& (const Object & v)override
			{
				return std::make_shared<Mem::Bool>(value && std::static_pointer_cast<Bool>(v)->value);
			}
			Object operator||(const Object & v)override
			{
				return std::make_shared<Mem::Bool>(value || std::static_pointer_cast<Bool>(v)->value);
			}
			Object clone()const override
			{
				return std::make_shared<Mem::Bool>(value);
			}
			bool _value() { return value; }
		private:
			bool value;
		};
		class Int :public Value
		{
		public:
			Int(int v) :value(v) {}
			std::string to_string()const override
			{
				return std::to_string(value);
			}
			type_code_index get_type()const override
			{
				return BasicType::INT;
			}
			size_t length()const override { return sizeof(int); }
			char *get_raw_data() override {
				return (char*)(&value);
			}

			Object operator=(const Object & v)override;
			Object operator+=(const Object & v)override
			{
				return std::make_shared<Int>(value += std::static_pointer_cast<Int>(v)->value);
			}
			Object operator-=(const Object & v)override
			{
				return std::make_shared<Int>(value -= std::static_pointer_cast<Int>(v)->value);
			}
			Object operator*=(const Object & v)override
			{
				return std::make_shared<Int>(value *= std::static_pointer_cast<Int>(v)->value);
			}
			Object operator/=(const Object & v)override
			{
				return std::make_shared<Int>(value /= std::static_pointer_cast<Int>(v)->value);
			}
			Object operator+ (const Object & v)override
			{
				return std::make_shared<Int>(value + std::static_pointer_cast<Int>(v)->value);
			}
			Object operator- (const Object & v)override
			{
				return std::make_shared<Int>(value -
					std::static_pointer_cast<Int>(v)->value);
			}
			Object operator* (const Object & v)override
			{
				return std::make_shared<Int>(value *
					std::static_pointer_cast<Int>(v)->value);
			}
			Object operator/ (const Object & v)override
			{
				return std::make_shared<Int>(value /
					std::static_pointer_cast<Int>(v)->value);
			}
			Object operator> (const Object & v)override
			{
				return std::make_shared < Bool >(value > std::static_pointer_cast<Int>(v)->value);
			}
			Object operator>= (const Object & v)override
			{
				return std::make_shared<Bool>(value >= std::static_pointer_cast<Int>(v)->value);
			}
			Object operator< (const Object & v)override
			{
				return std::make_shared<Bool>(value < std::static_pointer_cast<Int>(v)->value);
			}
			Object operator<= (const Object & v)override
			{
				return std::make_shared<Bool>(value <= std::static_pointer_cast<Int>(v)->value);
			}
			Object operator== (const Object & v)override
			{
				return std::make_shared<Bool>(value == std::static_pointer_cast<Int>(v)->value);
			}
			Object operator!= (const Object & v)override
			{
				return std::make_shared<Bool>(value != std::static_pointer_cast<Int>(v)->value);
			}
			Object operator<< (const Object& v)override {
				return std::make_shared<Mem::Int>(value << std::static_pointer_cast<Int>(v)->value);
			}
			Object operator>> (const Object& v)override {
				return std::make_shared<Mem::Int>(value >> std::static_pointer_cast<Int>(v)->value);
			}
			Object operator& (const Object& v)override {
				return std::make_shared<Mem::Int>(value & std::static_pointer_cast<Int>(v)->value);
			}
			Object operator| (const Object& v)override {
				return std::make_shared<Mem::Int>(value | std::static_pointer_cast<Int>(v)->value);
			}
			Object operator^ (const Object& v)override {
				return std::make_shared<Mem::Int>(value ^ std::static_pointer_cast<Int>(v)->value);
			}
			Object operator<<= (const Object& v)override {
				return std::make_shared<Mem::Int>(value <<= std::static_pointer_cast<Int>(v)->value);
			}
			Object operator>>= (const Object& v)override {
				return std::make_shared<Mem::Int>(value >>= std::static_pointer_cast<Int>(v)->value);
			}
			Object operator&= (const Object& v)override {
				return std::make_shared<Mem::Int>(value &= std::static_pointer_cast<Int>(v)->value);
			}
			Object operator|= (const Object& v)override {
				return std::make_shared<Mem::Int>(value |= std::static_pointer_cast<Int>(v)->value);
			}
			Object operator^= (const Object& v)override {
				return std::make_shared<Mem::Int>(value ^= std::static_pointer_cast<Int>(v)->value);
			}
			Object clone()const override
			{
				return std::make_shared<Int>(value);
			}
			Object get_negation()override
			{
				return std::make_shared<Int>(-value);
			}
			int &get_value()
			{
				return value;
			}
			static int get_val(Mem::Object obj) {
				return std::static_pointer_cast<Mem::Int>(obj)->get_value();
			}
			Object Convert(type_code_index type) override;
			Object operator[](const Object & v)override { throw Error("int doesn't have a member <operator[](int)>"); }
		private:
			int value;
		};
		class Double :public Value
		{
		public:
			Double(double v) :value(v) {}
			std::string to_string()const override
			{
				return std::to_string(value);
			}
			type_code_index get_type()const override
			{
				return BasicType::DOUBLE;
			}
			size_t length()const override { return sizeof(double); }
			char *get_raw_data() override {
				return (char*)(&value);
			}


			Object operator=(const Object & v)override;
			Object operator+=(const Object & v)override
			{
				return std::make_shared<Double>(value +=
					std::static_pointer_cast<Double>(v)->value);
			}
			Object operator-=(const Object & v)override
			{
				return std::make_shared<Double>(value -=
					std::static_pointer_cast<Double>(v)->value);
			}
			Object operator*=(const Object & v)override
			{
				return std::make_shared<Double>(value *=
					std::static_pointer_cast<Double>(v)->value);
			}
			Object operator/=(const Object & v)override
			{
				return std::make_shared<Double>(value /=
					std::static_pointer_cast<Double>(v)->value);
			}
			Object operator+(const Object & v)override
			{
				return std::make_shared<Double>(value +
					std::static_pointer_cast<Double>(v)->value);
			}
			Object operator-(const Object & v)override
			{
				return std::make_shared<Double>(value -
					std::static_pointer_cast<Double>(v)->value);
			}
			Object operator*(const Object & v)override
			{
				return std::make_shared<Double>(value *
					std::static_pointer_cast<Double>(v)->value);
			}
			Object operator/(const Object & v)override
			{
				return std::make_shared<Double>(value /
					std::static_pointer_cast<Double>(v)->value);
			}
			Object operator<(const Object & v)override
			{
				return std::make_shared<Bool>(value < std::static_pointer_cast<Double>(v)->value);
			}
			Object operator>(const Object & v)override
			{
				return std::make_shared<Bool>(value > std::static_pointer_cast<Double>(v)->value);
			}
			Object operator<=(const Object & v)override
			{
				return std::make_shared<Bool>(value <= std::static_pointer_cast<Double>(v)->value);
			}
			Object operator>=(const Object & v)override
			{
				return std::make_shared<Bool>(value >= std::static_pointer_cast<Double>(v)->value);
			}
			Object operator==(const Object & v)override
			{
				return std::make_shared<Bool>(value == std::static_pointer_cast<Double>(v)->value);
			}
			Object operator!=(const Object & v)override
			{
				return std::make_shared<Bool>(value != std::static_pointer_cast<Double>(v)->value);
			}
			Object clone()const override
			{
				return std::make_shared<Double>(value);
			}
			Object get_negation()override
			{
				return std::make_shared<Double>(-value);
			}
			Object operator[](const Object & v)override { throw Error("double doesn't have a member <operator[](int)>"); }
			double get_value()
			{
				return value;
			}
			Object Convert(type_code_index type)override;
		private:
			double value;
		};
		class String :public Value
		{
		public:
			String(const std::string& v) :str(v) {  }
			String(char ch) :str(std::string(1, ch)) {}
			Object operator+(const Object & v)override
			{
				return std::make_shared<String>(str +
					std::static_pointer_cast<String>(v)->str);
			}
			Object operator=(const Object & v)override
			{
				str = std::static_pointer_cast<String>(v)->str;
				return std::make_shared<String>(str);
			}
			type_code_index get_type()const override
			{
				return BasicType::STRING;
			}

			Object Convert(type_code_index type)override
			{
				return std::make_shared<String>(str);
			}
			Object operator+=(const Object & v)override
			{
				return std::make_shared<String>(str += std::static_pointer_cast<String>(v)->str);
			}
			Object operator>(const Object & v)override
			{
				return std::make_shared<Bool>(str > std::static_pointer_cast<String>(v)->str);
			}
			Object operator<(const Object & v)override
			{
				return std::make_shared<Bool>(str < std::static_pointer_cast<String>(v)->str);
			}
			Object operator>=(const Object & v)override
			{
				return std::make_shared<Bool>(str >= std::static_pointer_cast<String>(v)->str);
			}
			Object operator<=(const Object & v)override
			{
				return std::make_shared<Bool>(str <= std::static_pointer_cast<String>(v)->str);
			}
			Object operator!=(const Object & v)override
			{
				return std::make_shared<Bool>(str != std::static_pointer_cast<String>(v)->str);
			}
			Object operator==(const Object & v)override
			{
				return std::make_shared<Bool>(str == std::static_pointer_cast<String>(v)->str);
			}
			Object operator[](const Object & v)override;
			Object clone()const override
			{
				return std::make_shared<String>(str);
			}
			std::string to_string()const override
			{
				return str;
			}
			std::string str;
		};

		class ObjList :public Value
		{
		public:
			ObjList(std::vector<Object>&& lst, type_code_index type_c) :elems(std::move(lst)), type_code(type_c) {}
			ObjList(size_t sz, type_code_index type_c) : type_code(type_c), elems(sz) {}
			virtual Object operator[](const Object & v)override
			{
				return elems[std::static_pointer_cast<Int>(v)->get_value()];
			}
			virtual type_code_index get_type()const override
			{
				return BasicType::ARRAY;
			}
			Object clone()const override;
			type_code_index get_ele_type()const
			{
				return type_code;
			}
			type_code_index type_code;
			std::vector<Object> elems;
		};
		class Pointer :public Value
		{
		public:
			Pointer(size_t s) :add(s) {}
			Pointer(Object _obj)=delete;
			char *get_raw_data()override { return (char*)&add; }
			size_t length()const override { return sizeof(size_t); }
			Mem::Object operator=(const Object & v)override;
			Mem::Object operator==(const Object & v)override;
			Mem::Object operator!=(const Object & v)override;
			Mem::Object operator+(const Object & v)override;
			Mem::Object operator-(const Object & v)override;
			Mem::Object operator+=(const Object & v)override;
			Mem::Object operator-=(const Object & v)override;
			Mem::Object clone()const override;
			Mem::Object rm_ref();
			Object Convert(type_code_index type) override;
			Object operator[](const Object & v)override;
			std::string to_string()const override
			{
				return "pointer " + std::to_string(add);
			}
			size_t get_value()const { return add; }
		private:
			size_t add;
		};
		class Char :public Value
		{
		public:
			Char(char v) :value(new char(v)) {}
			Char(char* v) :value(v), del(false) {}
			std::string to_string()const override
			{
				return std::string(1, *value);
			}
			type_code_index get_type()const override
			{
				return BasicType::CHAR;
			}
			size_t length()const override { return 1; }
			char *get_raw_data() override {
				return value;
			}


			Object operator=(const Object & v)override;
			Object operator+=(const Object & v)override
			{
				return std::make_shared<Char>(*value += *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator-=(const Object & v)override
			{
				return std::make_shared<Char>(*value -= *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator*=(const Object & v)override
			{
				return std::make_shared<Char>(*value *= *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator/=(const Object & v)override
			{
				return std::make_shared<Char>(*value /= *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator+ (const Object & v)override
			{
				return std::make_shared<Char>(*value + *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator- (const Object & v)override
			{
				return std::make_shared<Char>(*value -
					*std::static_pointer_cast<Char>(v)->value);
			}
			Object operator* (const Object & v)override
			{
				return std::make_shared<Char>(*value *
					*std::static_pointer_cast<Char>(v)->value);
			}
			Object operator/ (const Object & v)override
			{
				return std::make_shared<Char>(*value /
					*std::static_pointer_cast<Char>(v)->value);
			}
			Object operator> (const Object & v)override
			{
				return std::make_shared < Bool >(*value > * std::static_pointer_cast<Char>(v)->value);
			}
			Object operator>= (const Object & v)override
			{
				return std::make_shared<Bool>(*value >= *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator< (const Object & v)override
			{
				return std::make_shared<Bool>(*value < *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator<= (const Object & v)override
			{
				return std::make_shared<Bool>(*value <= *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator== (const Object & v)override
			{
				return std::make_shared<Bool>(*value == *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator!= (const Object & v)override
			{
				return std::make_shared<Bool>(*value != *std::static_pointer_cast<Char>(v)->value);
			}
			Object operator<< (const Object& v)override {
				return std::make_shared<Mem::Char>(*value << std::static_pointer_cast<Char>(v)->get_value());
			}
			Object operator>> (const Object& v)override {
				return std::make_shared<Mem::Char>(*value >> std::static_pointer_cast<Char>(v)->get_value());
			}
			Object operator& (const Object& v)override {
				return std::make_shared<Mem::Char>(*value & std::static_pointer_cast<Char>(v)->get_value());
			}
			Object operator| (const Object& v)override {
				return std::make_shared<Mem::Char>(*value | std::static_pointer_cast<Char>(v)->get_value());
			}
			Object operator^ (const Object& v)override {
				return std::make_shared<Mem::Char>(*value ^ std::static_pointer_cast<Char>(v)->get_value());
			}
			Object operator<<= (const Object& v)override {
				return std::make_shared<Mem::Char>(*value <<= std::static_pointer_cast<Char>(v)->get_value());
			}
			Object operator>>= (const Object& v)override {
				return std::make_shared<Mem::Char>(*value >>= std::static_pointer_cast<Char>(v)->get_value());
			}
			Object operator&= (const Object& v)override {
				return std::make_shared<Mem::Char>(*value &= std::static_pointer_cast<Char>(v)->get_value());
			}
			Object operator|= (const Object& v)override {
				return std::make_shared<Mem::Char>(*value |= std::static_pointer_cast<Char>(v)->get_value());
			}
			Object operator^= (const Object& v)override {
				return std::make_shared<Mem::Char>(*value ^= std::static_pointer_cast<Char>(v)->get_value());
			}
			Object clone()const override
			{
				return std::make_shared<Char>(*value);
			}
			Object get_negation()override
			{
				return std::make_shared<Char>(-*value);
			}
			char &get_value()
			{
				return *value;
			}
			Object Convert(type_code_index type) override;
			Object operator[](const Object & v)override { throw Error("char doesn't have a member <operator[](int)>"); }
			virtual ~Char() { if (del)delete value; }
		private:
			bool del = true;
			char* value;
		};
		class AnyObj :public Mem::Value {
		public:
			template<typename T>
			AnyObj(const T& t) :obj(t) {}
			template<typename T>
			AnyObj& operator=(const T& t)
			{
				obj = t;
				return *this;
			}
			template<typename T>
			AnyObj& operator==(T&& t)noexcept {
				obj = t;
				return *this;
			}
			template<typename T>
			T& cast() {
				return mer::any_cast<T&>(obj);
			}
			Mem::Object clone()const override;
		private:
			mer::any obj;
		};
		template<typename T>
		T get_raw(Object obj)
		{
			// An UB operation to improve performance, but just be careful, I've tested in Microsoft CL, Clang ,GCC.
#ifndef SAFE_MOD
			return *(T*)((char*)obj.get() + sizeof(void*));
#else
			throw Error("unsafe calling");
#endif
		}

	}
}