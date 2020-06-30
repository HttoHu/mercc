/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/#include "../include/optimizer.hpp"
#include "../include/expr.hpp"
#include "../include/value.hpp"
namespace Mer
{
	bool is_a_structure_type(type_code_index t);
	class StructCopyer :public ParserNode
	{
	public:
		StructCopyer(type_code_index ty, ParserNode* l, ParserNode* r);
		Mem::Object execute()override;
		~StructCopyer();
	private:
		int len;
		ParserNode* lhs;
		ParserNode* rhs;
	};
	namespace optimizer
	{
		namespace {
			Mem::Object add(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator+(rhs);
			}
			Mem::Object sub(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator-(rhs);
			}
			Mem::Object mul(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator*(rhs);
			}
			Mem::Object div(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator/(rhs);
			}
			Mem::Object sadd(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator+=(rhs);
			}
			Mem::Object ssub(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator-=(rhs);
			}
			Mem::Object smul(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator*=(rhs);
			}
			Mem::Object sdiv(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator/=(rhs);
			}
			Mem::Object assign(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator=(rhs);
			}
			Mem::Object equal(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator==(rhs);
			}

			Mem::Object lshift(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator<<(rhs);
			}
			Mem::Object rshift(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator>>(rhs);
			}
			Mem::Object band(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator&(rhs);
			}
			Mem::Object bor(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator|(rhs);
			}
			Mem::Object bxor(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator^(rhs);
			}
			Mem::Object slshift(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator<<=(rhs);
			}
			Mem::Object srshift(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator>>=(rhs);
			}
			Mem::Object sband(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator&=(rhs);
			}
			Mem::Object sbor(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator|=(rhs);
			}
			Mem::Object sbxor(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator^=(rhs);
			}
			Mem::Object not_equal(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator!=(rhs);
			}
			Mem::Object gt(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator>(rhs);
			}
			Mem::Object lt(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator<(rhs);
			}
			Mem::Object ge(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator>=(rhs);
			}
			Mem::Object le(const Mem::Object& lhs, const Mem::Object& rhs) {
				return lhs->operator<=(rhs);
			}
			Mem::Object int_mod(const Mem::Object& lhs, const Mem::Object& rhs)
			{
				return std::make_shared<Mem::Int>(*(int*)lhs->get_raw_data() % *(int*)(rhs->get_raw_data()));
			}
			Mem::Object sint_mod(const Mem::Object& lhs, const Mem::Object& rhs)
			{
				return std::make_shared<Mem::Int>(*(int*)lhs->get_raw_data() %= *(int*)(rhs->get_raw_data()));
			}

			Mem::Object get_neg(const Mem::Object &v) {
				return v->get_negation();
			}
			Mem::Object trans(const Mem::Object &v)
			{
				return v;
			}
			Mem::Object int_front_inc(const Mem::Object &v)
			{
				std::static_pointer_cast<Mem::Int>(v)->get_value()++;
				return v;
			}
			Mem::Object int_back_inc(const Mem::Object &v)
			{
				auto ret = v->clone();
				std::static_pointer_cast<Mem::Int>(v)->get_value()++;
				return ret;
			}
			Mem::Object int_front_dec(const Mem::Object &v)
			{
				std::static_pointer_cast<Mem::Int>(v)->get_value()--;
				return v;
			}
			Mem::Object int_back_dec(const Mem::Object &v)
			{
				auto ret = v->clone();
				std::static_pointer_cast<Mem::Int>(v)->get_value()--;
				return ret;
			}
			Mem::Object int_bnot(const Mem::Object& lhs)
			{
				return std::make_shared<Mem::Int>(~*(int*)lhs->get_raw_data());
			}
			Mem::Object char_front_inc(const Mem::Object& v)
			{
				std::static_pointer_cast<Mem::Char>(v)->get_value()++;
				return v;
			}
			Mem::Object char_back_inc(const Mem::Object& v)
			{
				auto ret = v->clone();
				std::static_pointer_cast<Mem::Char>(v)->get_value()++;
				return ret;
			}
			Mem::Object char_front_dec(const Mem::Object& v)
			{
				std::static_pointer_cast<Mem::Char>(v)->get_value()--;
				return v;
			}
			Mem::Object char_back_dec(const Mem::Object& v)
			{
				auto ret = v->clone();
				std::static_pointer_cast<Mem::Char>(v)->get_value()--;
				return ret;
			}
			Mem::Object char_bnot(const Mem::Object& lhs)
			{
				return std::make_shared<Mem::Char>(~*(char*)lhs->get_raw_data());
			}
		}
		std::map<Mer::Tag, Mem::Object(*) (const Mem::Object&, const Mem::Object&)> op_table{
			{Mer::PLUS,add},{MINUS,sub},{MUL,mul},{DIV,div},{SMUL,smul},{SDIV,sdiv},{SADD,sadd},
			{SSUB,ssub},{ASSIGN,assign},{EQ,equal},{NE,not_equal},{GT,gt},{GE,ge},{LT,lt},{LE,le},
			{MOD,int_mod},{LSHIFT,lshift},{RSHIFT,rshift},{BOR,bor},{BAND,band},
			{SLS,slshift},{SRS,srshift},{SBXOR,sbxor},{SBAND,sband},{SBOR,sbor}
		};
		std::map<std::pair<Mer::Tag, type_code_index>, Mem::Object(*)(const Mem::Object&)> unary_op_table
		{ 
			{{Mer::MINUS,Mem::INT},get_neg},{{PLUS,Mem::INT},trans},
			{{NOT,Mem::BOOL},get_neg},{{INC,Mem::INT},int_front_inc},{{DEC,Mem::INT},int_front_dec},
			{{_BINC,Mem::INT},int_back_inc},{{_BDEC,Mem::INT},int_back_dec},{{BNOT,Mem::INT},int_bnot},
			{{INC,Mem::CHAR},char_front_inc},{{DEC,Mem::CHAR},char_front_dec},{{Mer::MINUS,Mem::CHAR},get_neg},
			{{_BINC,Mem::CHAR},char_back_inc},{{_BDEC,Mem::CHAR},char_back_dec},{{BNOT,Mem::CHAR},char_bnot},
		} ;

		
		ParserNode* optimize_bin_op(ParserNode* left, ParserNode* right, Token* tok)
		{			
			type_code_index ty = left->get_type();
			if (is_a_structure_type(ty))
			{
				if (tok->get_tag() != ASSIGN)
					throw Error("structs don't support the operation " + tok->to_string());
				return new StructCopyer(ty, left, right);
			}
			if (typeid(*left) == typeid(LConV) && typeid(*right) == typeid(LConV)) {
				Mem::Object left_v = left->execute();
				Mem::Object right_v;
				if (right->get_type() != ty)
					right_v = right->execute()->Convert(ty);
				else
					right_v = right->execute();
				Mem::Object ret;
				switch (tok->get_tag())
				{
				case MOD:
					ret = std::make_shared<Mem::Int>(*(int*)left_v->get_raw_data() % *(int*)right_v->get_raw_data());
					break;
				case RSHIFT:
					ret = left_v->operator>>(right_v);
					break;
				case LSHIFT:
					ret = left_v->operator<<(right_v);
					break;
				case BOR:
					ret = left_v->operator|(right_v);
					break;
				case BAND:
					ret = left_v->operator&(right_v);
					break;
				case BXOR:
					ret = left_v->operator^(right_v);
					break;
				case SADD:
					ret = left_v->operator+=(right_v);
					break;
				case SSUB:
					ret = left_v->operator-=(right_v);
					break;
				case SDIV:
					ret = left_v->operator/=(right_v);
					break;
				case SMUL:
					ret = left_v->operator*=(right_v);
					break;
				case SBXOR:
					ret = left_v->operator^=(right_v);
					break;
				case SBAND:
					ret = left_v->operator&=(right_v);
					break;
				case SBOR:
					ret = left_v->operator|=(right_v);
					break;
				case SLS:
					ret = left_v->operator<<=(right_v);
					break;
				case SRS:
					ret = left_v->operator>>=(right_v);
					break;
				case ASSIGN:
					ret = left_v->operator=(right_v);
					break;
				case PLUS:
					ret = left_v->operator+(right_v);
					break;
				case MINUS:
					ret = left_v->operator-(right_v);
					break;
				case MUL:
					ret = left_v->operator*(right_v);
					break;
				case DIV:
					ret = left_v->operator/(right_v);
					break;
				case EQ:
					ret = left_v->operator==(right_v);
					break;
				case NE:
					ret = left_v->operator!=(right_v);
					break;
				case GT:
					ret = left_v->operator>(right_v);
					break;
				case GE:
					ret = left_v->operator>=(right_v);
					break;
				case LT:
					ret = left_v->operator<(right_v);
					break;
				case LE:
					ret = left_v->operator<=(right_v);
					break;
				default:
					throw Error("Undefined operator");
				}
				delete left; delete right;
				return new LConV(ret, ty);
			}
			return new BinOp(left, tok, right);
		}
		ParserNode* optimize_unary_op(ParserNode* left, Token* tok)
		{
			Mem::Object ret;
			if (typeid(*left) == typeid(LConV)) {
				switch (tok->get_tag())
				{
				case NOT:
					if (left->get_type() != Mem::BOOL)
						if (Mem::type_convertible(left->get_type(), Mem::BOOL))
						{
							new LConV(std::static_pointer_cast<Mem::Bool>(left->execute()->Convert(Mem::BOOL))->get_negation(),Mem::BOOL);
						}
						else
							throw Error("type " + Mem::type_to_string(Mem::BasicType(left->get_type())) + " can't convert to bool");
					break;
				case MINUS:
				{
					auto tmp = left->execute();
					ret = tmp->get_negation();
					break;
				}
				case PLUS:
					ret = left->execute();
				default:
					throw Error("no matched operator");
				}
				type_code_index ty = left->get_type();
				delete left;
				return new LConV(ret, ty);
			}
			return new UnaryOp(tok->get_tag(), left);
		}
		ParserNode* optimize_array_subscript(ParserNode* arr, ParserNode* subscript,type_code_index default_type)
		{
			ParserNode* ret;
			type_code_index type=default_type?default_type:arr->get_type();
			if ((typeid(*arr)==typeid(GVar)||typeid(*arr) == typeid(Variable)) && static_cast<Variable*>(arr)->arr())
			{
				int index = 0;
				if (typeid(*subscript) == typeid(LConV))
				{
					auto v = static_cast<LConV*>(subscript);
					// skip subscript 
					index = Mem::get_raw<int>(v->execute());
					if (typeid(*arr) == typeid(GVar))
						ret = new GVar(type, arr->get_pos() + index);
					else
						ret = new Variable(type, arr->get_pos() + index);
					delete arr;
					delete subscript;
					return ret;
				}
				else
				{
					if (typeid(*arr) == typeid(GVar))
						ret = new ContainerGloIndex(arr->get_pos(),type, subscript);
					else
						ret = new ContainerIndex(type, arr->get_pos(), subscript);
					delete arr;
					return ret;
				}
			}
			return new SubScript(arr, subscript,type);
		}
	}
}