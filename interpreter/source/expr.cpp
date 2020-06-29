/*
* MIT License
* Copyright (c) 2019 Htto Hu
*/
#include "../include/expr.hpp"
#include "../include/value.hpp"
#include "../include/compound_box.hpp"
#include "../include/memory.hpp"
#include "../include/namespace.hpp"
#include "../include/function.hpp"
#include "../include/word_record.hpp"
#include "../include/optimizer.hpp"

namespace Mer
{
	extern size_t current_function_rety;


	Expr::Expr(type_code_index t) :is_bool(false), expr_type(t) {
		tree = assign();
		if (!tree)
			return;
		if (expr_type == 0 && tree->get_type() != 0)
		{
			expr_type = tree->get_type();
		}
		else if (expr_type != tree->get_type())
			tree = new Cast(tree, expr_type);
	}
	type_code_index Expr::get_type()
	{
		if (is_bool)
		{
			return Mem::BOOL;
		}
		if (tree == nullptr)
			return Mem::BVOID;
		return expr_type;
	}
	ParserNode* Expr::assign()
	{
		auto result = conditional_expr();
		std::set<Tag> assign_operator{
			ASSIGN,SADD,SSUB,SMUL,SDIV,SRS,SLS,SBAND,SBOR,SBXOR
		};
		while (assign_operator.count(token_stream.this_tag()))
		{
			auto tok = token_stream.this_token();
			token_stream.next();
			result = optimizer::optimize_bin_op(result, conditional_expr(), tok);
		}
		return result;
	}
	ParserNode * Expr::conditional_expr()
	{
		auto result = parse_or();
		if (token_stream.this_tag() == QUE)
		{
			token_stream.next();
			auto true_cond_expr = parse_or();
			token_stream.match(COLON);
			auto false_cond_expr = parse_or();
			return new ConditionalOperator(result, true_cond_expr, false_cond_expr);
		}
		return result;
	}

	Mer::ParserNode* Expr::parse_or()
	{
		auto result = parse_and();
		while (token_stream.this_tag() == OR)
		{
			is_bool = true;
			auto tok = token_stream.this_token();
			token_stream.next();
			result = new LogicalBinOp(result, tok, parse_and());
		}
		return result;
	}
	Mer::ParserNode* Expr::parse_and()
	{
		auto result = bit_or();
		while (token_stream.this_tag() == AND)
		{
			is_bool = true;
			auto tok = token_stream.this_token();
			token_stream.next();
			result = new LogicalBinOp(result, tok, bit_or());
		}
		return result;
	}
	ParserNode* Expr::bit_or()
	{
		auto result = parse_xor();
		while (token_stream.this_tag() == BOR)
		{
			auto tok = token_stream.this_token();
			token_stream.next();
			result = optimizer::optimize_bin_op(result,parse_xor(),tok);
		}
		return result;
	}
	ParserNode* Expr::parse_xor()
	{
		auto result = bit_and();
		while (token_stream.this_tag() == BXOR)
		{
			auto tok = token_stream.this_token();
			token_stream.next();
			result = optimizer::optimize_bin_op(result, bit_and(), tok);
		}
		return result;
	}
	ParserNode* Expr::bit_and()
	{
		auto result = equal();
		while (token_stream.this_tag() == BAND)
		{
			auto tok = token_stream.this_token();
			token_stream.next();
			result = optimizer::optimize_bin_op(result, equal(), tok);
		}
		return result;
	}
	ParserNode* Expr::equal()
	{
		auto result = nexpr();
		while (token_stream.this_tag() == EQ|| token_stream.this_tag()==NE)
		{
			is_bool = true;
			auto tok = token_stream.this_token();
			token_stream.next();
			result = optimizer::optimize_bin_op(result, nexpr(), tok);
		}
		return result;
	}
	ParserNode* Expr::nexpr()
	{
		auto result = l_r_shift();
		while (1)
		{
			auto tok = token_stream.this_token();
			switch (tok->get_tag())
			{
			case GE:
			case GT:
			case LE:
			case LT:
				is_bool = true;
				token_stream.next();
				break;
			default:
				return result;
			}
			result = optimizer::optimize_bin_op(result, l_r_shift(), tok);
		}
		return result;
	}

	ParserNode* Expr::l_r_shift()
	{
		auto result = expr();
		while (token_stream.this_tag() == LSHIFT || token_stream.this_tag() == RSHIFT)
		{
			is_bool = true;
			auto tok = token_stream.this_token();
			token_stream.next();
			result = optimizer::optimize_bin_op(result, l_r_shift(), tok);
		}
		return result;

	}


	ParserNode* Expr::expr()
	{
		auto result = term();

		while (token_stream.this_token()->get_tag() == PLUS || token_stream.this_token()->get_tag() == MINUS)
		{
			auto tok = token_stream.this_token();
			token_stream.next();
			result = optimizer::optimize_bin_op(result, term(), tok);
		}
		return result;
	}



	ParserNode* Expr::term()
	{
		auto result = member_visit();
		while (token_stream.this_tag() == MUL || token_stream.this_tag() == DIV || token_stream.this_tag()==MOD)
		{
			auto tok = token_stream.this_token();
			token_stream.next();
			result = optimizer::optimize_bin_op(result, member_visit(),tok);
		}
		return result;
	}

	ParserNode* Expr::member_visit()
	{
		auto result = subscript();
		while (token_stream.this_token()->get_tag() == DOT || token_stream.this_token()->get_tag() == PTRVISIT)
		{

			auto tok = token_stream.this_token();
			token_stream.next();
			type_code_index type_code = result->get_type();
			// get rid of the pointness.
			type_code_index raw_type = type_code + (type_code % 2 - 1);
			// pointer which may points a struct
			if (tok->get_tag() == PTRVISIT)
				result = new RmRef(result,type_code);
			// find struct info
			auto ustruct = find_ustructure_t(raw_type);
			// find member index and type;
			auto bias = count_bias(ustruct);
			// if the member is an array decay the customary method is invalid.
			if (typeid(*bias.second) == typeid(ArrayDecay))
			{
				if (result->global())
				{
					auto glo_arr_decay = new GloArrayDecay(bias.second->get_pos() + result->get_pos(),bias.first);
					delete bias.second;
					delete result;
					return glo_arr_decay;
				}
				static_cast<ArrayDecay*>(bias.second)->reset_pos(bias.second->get_pos() + result->get_pos());
				// Don't worry about exception safty, because when an exception throws the program will end.
				delete result;
				return bias.second;
			}
			return new MemberIndex(result, bias.second,bias.first);
		}
		return result;
	}
	ParserNode* Expr::subscript()
	{
		auto result = factor();
		while (token_stream.this_token()->get_tag() == LSB)
		{
			auto tok = token_stream.this_token();
			token_stream.match(LSB);
			auto exp = expr();
			token_stream.match(RSB);
			result = optimizer::optimize_array_subscript(result, exp);
		}
		if (token_stream.this_tag() == INC || token_stream.this_tag() == DEC)
		{
			auto real_tag = Tag(token_stream.this_tag() + 1);
			token_stream.next();
			return new UnaryOp(real_tag, result);
		}
		return result;
	}
	ParserNode* Expr::factor()
	{
		auto result = token_stream.this_token();
		// then it must be a type convertion.
		if (Mem::is_basic_type(result->get_tag()))
		{
			auto type_code = Mem::get_type_code(result);
			token_stream.next();
			token_stream.match(LPAREN);
			auto expr = assign();
			token_stream.match(RPAREN);
			return new Cast(expr, type_code);
		}
		switch (result->get_tag())
		{
		case MAKE:
			return Parser::make_var();
		case NEW:
			return new NewExpr();
		// deref.
		case MUL:
		{
			token_stream.match(MUL);
			auto ptr_part = factor();
			return new RmRef(ptr_part, ptr_part->get_type());
		}
		case BAND:
			return new GetAdd();
		case CAST:
			return new Cast();
		case TTRUE:
		{
			token_stream.match(TTRUE);
			return new LConV(result);
		}
		case TFALSE:
		{
			token_stream.match(TFALSE);
			return new LConV(result);
		}
		case LPAREN:
		{
			token_stream.match(LPAREN);
			// if it is a C-style type_convert such as (int)a;
			auto tok = token_stream.this_token();
			auto id_result = this_namespace->sl_table->find(tok->to_string());

			if (Mem::is_basic_type(tok->get_tag())||(id_result != nullptr && (id_result->es==STYPE)))
			{
				size_t convert_type_code = Mem::get_type_code();
				if (token_stream.this_tag() == MUL)
				{
					convert_type_code++;
					token_stream.next();
				}
				token_stream.match(RPAREN);
				return new Cast(factor(), convert_type_code);
			}
			ParserNode* v = assign();
			token_stream.match(RPAREN);
			return v;
		}
		case CHAR_LIT:
		case REAL:
		case STRING:
		case INTEGER:
			token_stream.next();
			return new LConV(result);
		case BNOT:
		case INC:
		case DEC:
		case PLUS:
		case NOT:
		case MINUS:
		{
			token_stream.next();
			ParserNode* n = optimizer::optimize_unary_op(member_visit(),result);
			return n;
		}
		case SIZEOF:
			return new SizeOf();
		case ID:
			return Parser::parse_id();
		case NULLPTR:
		{
			token_stream.match(NULLPTR);
			type_code_index to_convert_type = expr_type;
			if (to_convert_type)
				to_convert_type = Mem::BVOID + 1;
			return new LConV(std::make_shared<Mem::Pointer>(0), to_convert_type);
		}
		default:
			return new NonOp();
		}
	}

	BinOp::BinOp(ParserNode* l, Token* o, ParserNode* r) :left(l), right(r),op_tok(o)
	{
		if ((o->get_tag() == MOD || o->get_tag() == SMOD) && l->get_type() != Mem::INT)
			throw Error("type " + type_to_string(l->get_type()) + " don't support % operation, please convert it to int");
		auto result = optimizer::op_table.find(o->get_tag());
		if (result == optimizer::op_table.end())
			throw Error(TagStr[o->get_tag()] + " invalid operation");
		op = result->second;
		if (o->get_tag() != LSB && l->get_type() != r->get_type())
		{
			if (!(l->get_type() % 2 && r->get_type() % 2))
				return;
			right = new Cast(r, l->get_type());
		}
	}

	Mem::Object BinOp::execute()
	{
		return op(left->execute(), right->execute());
	}

	type_code_index BinOp::get_type()
	{
		switch (op_tok->get_tag())
		{
		case AND:
		case OR:
		case EQ:
		case NE:
		case GE:
		case GT:
		case LE:
		case LT:
			return Mem::BOOL;
		default:
			break;
		}
		return left->get_type();
	}

	std::string BinOp::to_string()
	{
		return left->to_string() + op_tok->to_string() + right->to_string();
	}

	UnaryOp::UnaryOp(Tag t, ParserNode * e):expr(e)
	{
		auto result = optimizer::unary_op_table.find({ t,e->get_type() });
		if (result == optimizer::unary_op_table.end())
			throw Error(TagStr[t] + " invalid operation");
		if (t == NOT && e->get_type() != Mem::BOOL)
			if (Mem::type_convertible(e->get_type(), Mem::BOOL))
				expr = new Cast(expr, Mem::BOOL);
			else
				throw Error("type " + Mem::type_to_string(Mem::BasicType(e->get_type())) + " can't convert to bool");
		op = result->second;
	}

	Mer::Mem::Object UnaryOp::execute()
	{
		return op(expr->execute());
	}


	InitList* InitList::make_list_from_tmp(size_t pos, type_code_index ty,UStructure* us)
	{
		std::vector<ParserNode*> vec;
		for (int i = 0u; i < us->get_size(); i++)
		{
			vec.push_back(new GVar(us->get_type_structure()[i], pos+i));
		}
		return new InitList(vec, ty);
	}

	InitList::InitList(const std::vector<ParserNode*>& _exprs, type_code_index _ty):init_v(_exprs),type(_ty)
	{
		size = init_v.size();
	}

	InitList::InitList(type_code_index t):type(t)
	{
		token_stream.match(BEGIN);
		while (token_stream.this_tag() != Tag::END)
		{
			auto insertion = Expr().root();
			if (insertion->get_type() != t)
			{ 
				size_t insertion_type = insertion->get_type();
				delete insertion;
				throw Error("init_list type not matched from " + type_to_string(t) + " to " + type_to_string(insertion_type));
			}
			init_v.push_back(insertion);
			if (token_stream.this_tag() == Tag::COMMA)
				token_stream.match(COMMA);
		}
		token_stream.match(END);
	}

	InitList::InitList(type_code_index t, int sz) :type(t), size(sz)
	{
		token_stream.match(BEGIN);
		while (token_stream.this_tag() != Tag::END)
		{
			init_v.push_back(Expr(t).root());
			if (token_stream.this_tag() == Tag::COMMA)
				token_stream.match(COMMA);
		}
		token_stream.match(END);
		if (init_v.size() == 1 && sz > 1)
		{
			for (int i = 1; i < sz; i++)
			{
				init_v.push_back(init_v[0]->clone());
			}
			return;
		}
		if (sz == -1)
			size = init_v.size();
		else if (init_v.size() != sz)
			throw Error("Error, array overflow expect " + std::to_string(sz) + " but received " + std::to_string(init_v.size()));

		for (size_t i = 1; i < init_v.size(); i++)
		{
			if (type != init_v[i - 1]->get_type())
			{
				throw Error("there is a type-distinction in an init list. from "+ type_to_string(type)+" to "+type_to_string(init_v[i-1]->get_type()));
			}
		}

	}

	Mem::Object InitList::execute()
	{
		std::vector<Mem::Object> v(init_v.size());
		auto sz = init_v.size();
		for (size_t i = 0; i < sz; i++)
		{
			 v[i]=init_v[i]->execute()->clone();
		}
		auto ret= std::make_shared<Mem::ObjList>(std::move(v), type);
		return ret;
	}

	std::vector<Mem::Object> InitList::get_array()
	{
		std::vector<Mem::Object> v(init_v.size());
		if (v.size() == 1 && size > 1)
		{
			auto tmp = init_v[0]->execute();
			v = std::vector<Mem::Object>(size, tmp);
			return v;
		}
		if (v.size() != size)
			throw Error("list size is not matched with array");
		for (size_t i = 0; i < v.size(); i++)
		{
			v[i] = init_v[i]->execute();
		}
		return v;
	}

	ParserNode* InitList::clone()
	{
		InitList* ret = new InitList;
		ret->type = type;
		ret->size = size;
		for (auto& a : init_v)
			ret->init_v.push_back(a->clone());
		return ret;
	}


	Mem::Object ContainerIndex::execute()
	{
		return mem[get_pos()];
	}

	type_code_index ContainerIndex::get_type()
	{
		return type;
	}

	size_t ContainerIndex::get_pos()
	{
		return mem.get_current() + pos + std::static_pointer_cast<Mem::Int>(expr->execute())->get_value();
	}

	ContainerIndex::~ContainerIndex()
	{
		 delete expr; 
	}

	EmptyList::EmptyList(type_code_index t, size_t &sz) :type_code(t), size(sz)
	{
		for (size_t i = 0; i < sz; i++)
		{
			Mem::Object cur_obj = Mem::create_var_t(t);
			if (typeid(*cur_obj) == typeid(Mem::ObjList))
			{
				auto& objs = std::static_pointer_cast<Mem::ObjList>(cur_obj)->elems;
				size += objs.size() - 1;
				for (auto a : objs)
					init_v.push_back(new LConV(a->clone(), a->get_type()));
			}
			else
				init_v.push_back(new LConV(Mem::create_var_t(t), t));
		}
		sz = size;
	}

	Mem::Object EmptyList::execute()
	{
		return nullptr;
	}

	size_t ContainerGloIndex::get_pos()
	{
		return Mem::get_raw<int>(expr->execute()) + pos;
	}

	Mem::Object ContainerGloIndex::execute()
	{
		return mem[get_pos()];
	}

	NewExpr::NewExpr(bool init_nothing)
	{
		if (init_nothing)
			return;
		token_stream.match(NEW);
		type_code_index type_code = Mem::get_type_code();
		if (token_stream.this_tag() == LPAREN)
		{
			expr = Expr(type_code).root();
		}
		else if (token_stream.this_tag() == SEMI)
		{
			auto insertion = Mem::create_var_t(type_code);
			expr = new LConV(insertion, type_code);
		}
		if (expr->get_type() != type_code)
		{
			throw Error("new-type not matched from " + type_to_string(expr->get_type()) + " to " + type_to_string(type_code));
		}
	}

	ParserNode* NewExpr::clone()
	{
		NewExpr* ret = new NewExpr(true);
		ret->expr = expr->clone();
		return ret;
	}

	Mem::Object NewExpr::execute()
	{
		// discarded
		return nullptr;
	}

	GetAdd::GetAdd()
	{
		token_stream.match(BAND);
		id = Expr().root();
		type = id->get_type();
	}

	type_code_index Mer::GetAdd::get_type()
	{
		return type + 1;
	}

	Mem::Object Mer::GetAdd::execute()
	{
		return std::make_shared<Mem::Pointer>(id->get_pos());
	}

	RmRef::RmRef(bool init_nothing)
	{
		if (init_nothing)
			return;
		token_stream.match(MUL);
		id = Expr().root();
		type = id->get_type();
	}

	type_code_index Mer::RmRef::get_type()
	{
		return type - 1;
	}
	Mem::Object Mer::RmRef::execute()
	{
		return std::static_pointer_cast<Mem::Pointer>(id->execute())->rm_ref();
	}
	ParserNode* RmRef::clone()
	{
		RmRef* ret = new RmRef(true);
		ret->id = id->clone();
		ret->type = type;
		return ret;
	}
	size_t RmRef::get_pos()
	{
		return *(size_t*)id->execute()->get_raw_data();
	}
	Mer::Index::Index(ParserNode* l, size_t _index, type_code_index _type) :left(l), index(_index), type(_type)
	{
		if (_type == -1)
			type = left->get_type();
	}
	Mem::Object Index::execute()
	{
		return left->execute()->operator[](std::make_shared<Mem::Int>(index));
	}

	type_code_index Mer::Index::get_type()
	{
		return type;
	}
	SubScript::SubScript(ParserNode* l, ParserNode* s) :left(l), subscr(s)
	{
		if (Mem::exist_operator(l->get_type(), "[]"))
		{ 
			type = Mem::find_op_type(l->get_type(), "[]");
		}
		else
			type = l->get_type();
	}
	SubScript::SubScript(ParserNode * l, ParserNode * s, type_code_index _ty) :left(l), subscr(s),type(_ty)
	{
	}
	type_code_index SubScript::get_type()
	{
		auto ret = left->get_type();
		// if pointer subscript, the element type is pointer_type -1 
		if (ret % 2 == 0)
			ret--;
		return ret;
	}
	Mem::Object SubScript::execute()
	{
		return left->execute()->operator[](subscr->execute());
	}

	size_t SubScript::get_pos()
	{
		return left->get_pos() + (size_t)*(int*) subscr->execute()->get_raw_data()+1;
	}

	LogicalBinOp::LogicalBinOp(ParserNode* l, Token* tok, ParserNode* r) :left(l),right(r)
	{
		if (tok->get_tag() == AND)
			is_and_op = true;
		else if (tok->get_tag() == OR)
			is_and_op = false;
		else
			throw Error("intern error");
	}

	Mem::Object LogicalBinOp::execute()
	{
		if (is_and_op ^ Mem::get_raw<bool>(left->execute()))
			return std::make_shared<Mem::Bool>(!is_and_op);
		return std::make_shared<Mem::Bool>(Mem::get_raw<bool>(right->execute()));
	}

	Mem::Object ArrayDecay::execute() {
		return std::make_shared<Mem::Pointer>(pos + mem.get_current());
	}

	Mem::Object GloArrayDecay::execute()
	{
		return std::make_shared<Mem::Pointer>(pos);
	}

}