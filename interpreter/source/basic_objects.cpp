/*
GNU GENERAL PUBLIC LICENSE
					   Version 3, 29 June 2007

 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.
*/
#include "../include/basic_objects.hpp"
#include "../include/memory.hpp"
#include "../include/lexer.hpp"
#include "../include/namespace.hpp"
#include "../include/function.hpp"
#include "../include/word_record.hpp"
#include "../include/compound_box.hpp"

namespace Mer
{
	void Mem::swap(Object rhs, Object lhs)
	{
		auto tmp = rhs->clone();
		rhs->operator=(lhs);
		lhs->operator=(tmp);
	}
	std::string Mem::type_to_string(BasicType bt)
	{
		std::string ret;
		bool is_p = bt % 2 || bt == 0;
		is_p = !is_p;
		if (is_p)
			bt = BasicType(bt - 1);
		switch (bt)
		{
		case Mem::BVOID:
			ret += "void";
			break;
		case Mem::BOOL:
			ret += "bool";
			break;
		case Mem::INT:
			ret += "int";
			break;
		case Mem::DOUBLE:
			ret += "double";
			break;
		case Mem::STRING:
			ret += "string";
			break;
		case Mem::CHAR:
			ret += "char";
			break;
		default:
			auto seeker = type_name_mapping.find(bt);
			if (seeker == type_name_mapping.end())
			{
				ret += ("!unkown type :" + std::to_string(bt));
			}
			else
				ret = seeker->second;
			break;
		}
		if (is_p)
			ret += "* ";
		return ret;
	}


	Mem::Object Mem::create_var_t(type_code_index type)
	{
		// pointer
		if (type % 2 == 0)
		{
			return std::make_shared<Pointer>(0);
		}
		switch (type)
		{
		case CHAR:
			return std::make_shared<Char>(0);
		case INT:
			return std::make_shared<Int>(0);
		case DOUBLE:
			return std::make_shared<Double>(0.0);
		case BOOL:
			return std::make_shared<Bool>(true);
		case STRING:
			return std::make_shared<String>("");
		default:
		{
			if (is_a_structure_type(type))
			{
				auto struct_info = find_ustructure_t(type);
				return std::make_shared<Mem::ObjList>(struct_info->init(), type);
			}
			throw Error("Type error!");
		}
		}
	}

	Mem::Object Mem::Int::operator=(const Object & v)
	{
		value = std::static_pointer_cast<Mem::Int>(v)->get_value();
		return v;
	}

	Mem::Object  Mem::Int::Convert(type_code_index type)
	{
		if (! (type % 2))
			return std::make_shared<Mem::Pointer>(value);
		switch (type)
		{
		case INT:
			return std::make_shared<Int>(value);
		case DOUBLE:
			return std::make_shared<Double>(value);
		case BOOL:
			return std::make_shared<Bool>(value);
		case CHAR:
			return std::make_shared<Char>(value);
		default:
			throw Error("int_value:" + std::to_string(value) + " int cannot convert to " + type_to_string((BasicType)type));
			break;
		}
	}

	Mem::Object Mem::Double::operator=(const Object & v) {

		value = std::static_pointer_cast<Double>(v)->value;
		return Convert(Mem::DOUBLE);
	}

	Mem::Object Mem::Double::Convert(type_code_index type)
	{
		switch (type)
		{
		case INT:
			return std::make_shared<Int>(value);
		case DOUBLE:
			return std::make_shared<Double>(value);
		default:
			throw Error("double cannot convert to " + type_to_string((BasicType)type));
		}
	}

	Mem::Object Mem::Bool::Convert(type_code_index type)
	{
		switch (type)
		{
		case BOOL:
			return std::make_shared<Bool>(value);
		case INT:
			return std::make_shared<Int>(value);
		default:
			throw Error("type-convert error");
		}
	}

	bool Mem::Type::convertible(const type_code_index& t)
	{
		return convertible_types.find(t) != convertible_types.end();
	}

	void Mem::Type::add_compatible_type(type_code_index type_code)
	{
		convertible_types.insert(type_code);
	}


	Mem::Object Mem::Pointer::operator=(const Object & v)
	{
		add = std::static_pointer_cast<Pointer> (v)->add;
		return std::make_shared<Pointer>(add);
	}

	Mem::Object Mem::Pointer::operator==(const Object & v)
	{
		return std::make_shared<Mem::Bool>(std::static_pointer_cast<Pointer>(v)->add == add);
	}

	Mem::Object Mem::Pointer::operator!=(const Object & v)
	{
		return std::make_shared<Mem::Bool>(std::static_pointer_cast<Pointer>(v)->add != add);
	}

	Mem::Object Mem::Pointer::operator+(const Object & v)
	{
		return std::make_shared<Pointer>(add + *(int*)v->get_raw_data());
	}

	Mem::Object Mem::Pointer::operator-(const Object & v)
	{
		return std::make_shared<Pointer>(add - *(int*)v->get_raw_data());
	}

	Mem::Object Mem::Pointer::operator+=(const Object&v)
	{
		return std::make_shared<Pointer>(add += *(int*)v->get_raw_data());
	}

	Mem::Object Mem::Pointer::operator-=(const Object & v)
	{
		return std::make_shared<Pointer>(add -= *(int*)v->get_raw_data());
	}

	Mem::Object Mem::Pointer::clone() const
	{
		return std::make_shared<Mem::Pointer>(add);
	}

	Mem::Object Mem::Pointer::rm_ref()
	{
		return Mer::mem[add];
	}

	Mem::Object Mem::Pointer::Convert(type_code_index type)
	{
		switch (type) {
		case Mem::INT:
			return std::make_shared<Int>(add);
		default:
			return clone();
		}
	}

	Mem::Object Mem::Pointer::operator[](const Object & v)
	{
		return Mer::mem[std::static_pointer_cast<Int>(v)->get_value() + add];
	}

	Mem::Object Mem::String::operator[](const Object & v)
	{
		return std::make_shared<Char>(&str[std::static_pointer_cast<Int>(v)->get_value()]);
	}

	std::string type_to_string(size_t type_code)
	{
		return Mem::type_to_string(Mem::BasicType(type_code));
	}

	Mem::Object Mer::Mem::Char::operator=(const Object & v)
	{
		auto tmp = v;
		*value = *std::static_pointer_cast<Char>(v)->value;
		return tmp;
	}

	Mem::Object Mer::Mem::Char::Convert(type_code_index type)
	{
		switch (type)
		{
		case STRING:
			return std::make_shared<String>(*value);
		case BOOL:
			return std::make_shared<Bool>(*value);
		case INT:
			return std::make_shared<Int>(*value);
		case CHAR:
			return std::make_shared<Char>(*value);
		default:
			throw Error("type-convert error");
		}
	}

	Mem::Object Mer::Mem::AnyObj::clone() const
	{
		return std::make_shared<AnyObj>(obj);
	}

	Mem::Object Mem::ObjList::clone() const
	{
		std::vector<Object> vec;
		for (auto& a : elems)
		{
			vec.push_back(a->clone());
		}
		return std::make_shared<ObjList>(std::move(vec), type_code);
	}
}


