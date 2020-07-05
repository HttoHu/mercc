/*
GNU GENERAL PUBLIC LICENSE
					   Version 3, 29 June 2007

 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.
*/
#include "../include/lexer.hpp"
using namespace Mer;
using TokenMap = std::map<std::string, Token*>;
using TagStrMap = std::map<Tag, std::string>;

std::string pre_input_content;
//#define
std::map<std::string, std::string> define_replace;
namespace Mer {
	extern std::map<std::string, void(*)()> repository;
	namespace num_process {
		// the number of every bit, convert to dec. 
		bool is_good_hex_bit(char ch) {
			if (ch >= '0' && ch <= '9')
				return true;
			ch = tolower(ch);
			if (ch > 'f' || ch < 'a')
				return false;
			return true;
		}
		int _hbit(char ch) {
			if (ch >= '0' && ch <= '9')
				return ch - '0';
			ch = tolower(ch);
			if (!is_good_hex_bit(ch))
				throw LexerError("invalid hex number");
			return 10 + ch - 'a';
		}
		// the number of every bit, convert to dec. 
		int _obit(char ch) {
			if (ch > '7' || ch < '0')
				throw LexerError("invalid oct number");
			return ch - '0';
		}
		Token* integer_to_token(unsigned long long num, const std::string& str, size_t& pos) {
			// suffix count u is unsigned such 112U. For example 1234LLU, l_count is 2, u_count is 1.
			int u_count = 0;
			int l_count = 0;

			while (tolower(str[pos]) == 'u' || tolower(str[pos]) == 'l') {
				u_count += (tolower(str[pos]) == 'u');
				l_count += tolower(str[pos]) == 'l';
				pos++;
			}
			// hcc don't distinguish long and long long, long -> 64 bit and long long is also 64 bit.
			if (u_count > 1 || l_count > 2) { throw LexerError("invalid integer"); }
			if (u_count) {
				return new Literal<unsigned int>(num, Tag::UINT);
			}
			return new Integer(num);
		}
		Token* hex_to_dec(const std::string& str, size_t& pos) {
			long long ret = 0;
			for (; pos < str.size(); pos++) {
				if (!is_good_hex_bit(str[pos]))
					break;
				ret *= 16;
				ret += _hbit(str[pos]);
			}
			// int32
			return new Integer(ret);
		}
		Token* oct_to_dec(const std::string& str, size_t& pos) {
			long long ret = 0;
			for (; pos < str.size(); pos++) {
				if (str[pos] < '0' || str[pos]>'7')
					break;
				ret *= 8;
				ret += _obit(str[pos]);
			}
			//int32
			if (ret == 0)
				return integer_to_token(ret, str, pos);
			return new Integer(ret);
		}
	}
	namespace Preprocessor
	{
		void preprocessor(const std::string& content, size_t& i);
		std::string retrive_word(const std::string& str, size_t& pos)
		{
			while (str[pos] == ' ')pos++;
			std::string ret;
			while (isalnum(str[pos]) || str[pos] == '_') ret += str[pos++];
			return ret;
		}
		bool match_str(const std::string& content, size_t& i, const std::string& str)
		{
			while (content[i] == ' ')
				i++;
			for (int j = 0; j < str.size(); j++)
				if (str[j] != content[i++])
					throw LexerError("lexer error");
		}

		void process_include(const std::string& content, size_t& i)
		{
			match_str(content, i, "<");

			std::string name = retrive_word(content, i);
			if (content[i] == '.')
			{
				name += "."; i++;
				name += retrive_word(content, i);
			}
			match_str(content, i, ">");
			auto result = repository.find(name);
			if (result == repository.end())
				throw LexerError("can not open the file " + name);
			result->second();
		}
		void process_undefine(const std::string& content, size_t& i)
		{
			std::string ref_str = retrive_word(content, i);
			if (define_replace.count(ref_str))
				define_replace.erase(ref_str);
		}
		void process_define(const std::string& content, size_t& i)
		{
			std::string ref_str = retrive_word(content, i);
			std::string replace_str;
			while (content[i] == ' ')i++;
			while (content[i] != '\n')
				replace_str += content[i++];
			define_replace.insert({ ref_str,replace_str });
		}
		// return 0 ->  endif , 1 -> else -1 -> error.
		int ignore_til_endif_else(const std::string& content, size_t& i)
		{
			while (i < content.size())
			{
				if (content[i] == '\n' || content[i] == '\r')
					token_stream.push_back(new Endl());
				else if (content[i] == '#')
				{
					size_t back_up_pos = i;
					i++;
					std::string ins_name = retrive_word(content, i);
					if (ins_name == "endif")
						return 0;
					else if (ins_name == "else")
						return 1;
					else
					{
						i = back_up_pos;
						preprocessor(content, i);
					}
				}
				i++;
			}
			return -1;
		}

		int eat_tok_till_endif_else(const std::string& content, size_t& i)
		{
			while (i < content.size())
			{
				Token* tok = parse_single_token(content, i);
				if (tok == nullptr)
					return -1;
				token_stream.push_back(tok);
				i++;
				if (content[i] == '#')
				{
					size_t back_up_pos = i;
					i++;
					std::string ins_name = retrive_word(content, i);
					if (ins_name == "endif")
						return 0;
					else if (ins_name == "else")
						return 1;
					else
					{
						i = back_up_pos;
						preprocessor(content, i);
					}
				}
			}
			return -1;
		}

		void process_preinput(const std::string& content, size_t& i)
		{
			while (content[i] == ' ')i++;
			if (content[i] == '\n' || content[i] == '\r') i++;
			std::string pre_ins;
			while (i < content.size())
			{
				if (content[i] == '#')
				{
					i++;
					pre_ins = retrive_word(content, i);
					if (pre_ins == "end_pre_input")
						break;
					else
					{
						pre_input_content += "#" + pre_ins;
						pre_ins.clear();
					}
				}
				else
					pre_input_content += content[i++];
			}
			if(pre_ins!="end_pre_input")
				throw LexerError("expect #end_pre_input");
			Mer::my_stringstream.str(pre_input_content);
		}
		void process_ifdef(const std::string& content, size_t& i)
		{
			std::string ref_str = retrive_word(content, i);
			if (!define_replace.count(ref_str))
			{
				int state = ignore_til_endif_else(content, i);
				if (state == 0)
					return;
				else if (state == 1)
				{
					int else_state = eat_tok_till_endif_else(content, i);
					if (else_state != 0)
						throw LexerError("preprocesser error, maybe you defined too many #else");
					return;
				}
				throw LexerError("you many forget to write #endif");
			}
			else
			{
				int state = eat_tok_till_endif_else(content, i);
				if (state == 0)
					return;
				else if (state == 1)
				{
					int else_state = ignore_til_endif_else(content, i);
					if (else_state != 0)
						throw LexerError("preprocesser error, maybe you defined too many #else");
					return;
				}
				throw LexerError("you many forget to write #endif");
			}
		}
		void process_ifndef(const std::string& content, size_t& i)
		{
			std::string ref_str = retrive_word(content, i);
			if (define_replace.count(ref_str))
			{
				int state = ignore_til_endif_else(content, i);
				if (state == 0)
					return;
				else if (state == 1)
				{
					int else_state = eat_tok_till_endif_else(content, i);
					if (else_state != 0)
						throw LexerError("preprocesser error, maybe you defined too many #else");
					return;
				}
				throw LexerError("you many forget to write #endif");
			}
			else
			{
				int state = eat_tok_till_endif_else(content, i);
				if (state == 0)
					return;
				else if (state == 1)
				{
					int else_state = ignore_til_endif_else(content, i);
					if (else_state != 0)
						throw LexerError("preprocesser error, maybe you defined too many #else");
					return;
				}
				throw LexerError("you many forget to write #endif");
			}
		}
		void process_endif(const std::string& content, size_t& i) {}
		void preprocessor(const std::string& content, size_t& i)
		{
			// skip #
			i++;
			std::string ins = retrive_word(content, i);
			std::map<std::string, void(*)(const std::string&, size_t&)> sub_processor_dic{
				{ "include",process_include },{"define",process_define},{"ifdef",process_ifdef},
				{"endif",process_endif},{"ifndef",process_ifndef},{"undef",process_undefine},{"pre_input",process_preinput}
			};
			auto result = sub_processor_dic.find(ins);
			if (result == sub_processor_dic.end())
				throw LexerError("invalid preprocess instruction " + ins);
			result->second(content, i);
		}

		extern std::map<std::string, void(*)()> repository;
	}
	size_t Endl::current_line = 0;
	TagStrMap	TagStr{
		{ IMPORT,"IMPORT" },{ NAMESPACE,"NAMESPACE" },{ STRUCT,"struct" },{ NEW,"new" },{ PTRVISIT,"PTRVISIT" },
		{INC,"++"},{_BINC,"++(back)"},{DEC,"--"},{_BDEC,"--(back)"}, {QUE,"QUE"},
		{ SHARP,"SHARP" },{ INCLUDE,"INCLUDE" },
		{ REF,"REF" },{ PROGRAM,"PROGRAME" },{ COMMA,"COMMA" },{ COLON,"COLON" },
		{ ID,"ID" },{ INTEGER,"INTEGER" },{ REAL,"REAL" } ,{ FUNCTION,"FUNCTION" },{ RETURN,"RETURN" },
		{ IF,"IF" },{ ELSE_IF,"ELSE_IF" },{ ELSE,"ELSE" },{ WHILE,"WHILE" },{ DO,"DO" } ,{ FOR,"FOR" },{ BREAK,"BREAK" },{ CONTINUE,"CONTINUE" },{ SWITCH,"SWITCH" },
		{ INTEGER_DECL,"INTEGER_DECL" },{ REAL_DECL,"REAL_DECL" },{ STRING_DECL,"STRING_DECL" },{ BOOL_DECL,"BOOL_DECL" },{ VOID_DECL,"VOID_DECL" },{ CHAR_DECL,"CHAR_DECL" },
		{ PLUS,"PLUS" },{ MINUS,"MINUS" },{ MUL,"MUL" },{ DIV,"DIV" },{MOD,"MOD"},
		{ MAKE,"make" },
		{ GE,"GE" },{ GT,"GT" },{ LE,"LE" },{ LT,"LT" },{ EQ,"EQ" },{ NE,"NE" },
		{ AND,"AND" },{ OR,"OR" },{ NOT,"NOT" },{ GET_ADD,"GET_ADD" },{BNOT,"BNOT"},
		{ LSHIFT,"LSHIFT"},{RSHIFT,"RSHIFT"},{BAND,"BAND"},{BOR,"BOR"},{BXOR,"BXOR"},
		{ SLS,"SLS"},{SRS,"SRS"},{SBAND,"SBAND"},{SBOR,"SBOR"},{SBXOR,"SBXOR"},
		{ LPAREN,"LPAREN" },{ RPAREN,"RPAREN" },{ LSB,"LSB" },{ RSB,"RSB" },
		{ DOT,"DOT" },{ BEGIN,"BEGIN" },{ END,"END" },
		{ SEMI,"SEMI" },{ ASSIGN,"ASSIGN" },{ SADD,"SADD" },{ CASE,"CASE" },
		{ ENDL,"ENDL" },{ PRINT,"PRINT" },{ CAST,"CAST" },
		{ TTRUE,"TTRUE" },{ TFALSE,"TFALSE" },{ NULLPTR,"NULLPTR" },{ SIZEOF,"SIZEOF" },
	};
	TokenMap	BasicToken{
		{ "#",new Token(SHARP) },
		{"++",new Token(INC)},{"--",new Token(DEC)},
		{ "+",new Token(PLUS) },{ "-",new Token(MINUS) },{ "*",new Token(MUL) },{ "/",new Token(DIV) },{"%",new Token(MOD)},
		{ "=",new Token(ASSIGN) },
		{ "+=",new Token(SADD) },{ "-=",new Token(SSUB) },{ "*=",new Token(SMUL) },{ "/=",new Token(SDIV) },
		{ "<",new Token(LT) },{ "<=",new Token(LE) },{ ">",new Token(GT) },{ ">=",new Token(GE) },{ "==",new Token(EQ) },
		{"<<",new Token(LSHIFT)},{">>",new Token(RSHIFT)},{"&",new Token(BAND)},{"|",new Token(BOR)},{"^",new Token(BXOR)},
		{"<<=",new Token(SLS)},{">>=",new Token(SRS)},{"&=",new Token(SBAND)},{"|=",new Token(SBOR)},{"^=",new Token(SBXOR)},
		{ "!=",new Token(NE) },{ "!",new Token(NOT) },{ "&&",new Token(AND) },{ "||",new Token(OR) },{"~",new Token(BNOT)}, { ":",new Token(COLON) },
		{ ",",new Token(COMMA) },{ ";",new Token(SEMI) },{ ".",new Token(DOT) },{ "->",new Token(PTRVISIT) },
		{ "[",new Token(LSB) },{ "]",new Token(RSB) },{ "(",new Token(LPAREN) },{ ")",new Token(RPAREN) },
		{ "{",new Token(BEGIN) },{ "}",new Token(END) },{"?",new Token(QUE)},
		{ "include",new Token(INCLUDE) },{ "namespace",new Token(NAMESPACE) },{ "struct",new Token(STRUCT) },{"enum",new Token(ENUM)},
		{ "if",new Token(IF) },{ "elif",new Token(ELSE_IF) },{ "else",new Token(ELSE) },{ "sizeof",new Token(SIZEOF) },
		{ "while",new Token(WHILE) },{ "break",new Token(BREAK) },{ "for",new Token(FOR) },{ "do",new Token(DO) },{ "switch",new Token(SWITCH) },{ "case",new Token(CASE) },
		{ "continue",new Token(CONTINUE) },{ "default",new Token(DEFAULT) },
		{ "function",new Token(FUNCTION) },{ "return",new Token(RETURN) },
		{ "new",new Token(NEW) },{ "make",new Token(MAKE) },
		{ "cast",new Token(CAST) },{ "true",new Token(TTRUE) },
		{ "false",new Token(TFALSE) },
		{ "string",new Token(STRING_DECL) },{ "bool",new Token(BOOL_DECL) },
		{ "ref",new Token(REF) },{ "double",new Token(REAL_DECL) },{ "void",new Token(VOID_DECL) },{ "char",new Token(CHAR_DECL) },
		{ "int",new Token(INTEGER_DECL) },{ "program",new Token(PROGRAM) },{ "nullptr",new Token(NULLPTR) }
	};
}
//==========================================



bool is_function_args = false;

std::map<char, char> escape_character_table = {
	{'r','\r'},{'n','\n'},{'b','\b'},{'t','\t'},{'a','\a'},{'0','\0'},{'\'','\''},{'\"','\"'},{'\\','\\'}
};
char _convert_escape(char c)
{
	auto result = escape_character_table.find(c);
	if (result == escape_character_table.end())
	{
		throw LexerError(" illegal escape char");
	}
	return result->second;
}

//==========================================
Token* Mer::END_TOKEN = new Token(ENDOF);
TokenStream Mer::token_stream;
//get a word from content


Token* Mer::parse_single_token(const std::string& content, size_t& i)
{
	for (;i < content.size(); i++)
	{
		char cur_char = content[i];
		switch (content[i])
		{
		case '\'':
			return parse_char(content, i);
		case '\"':
			return parse_string(content, i);
		case '\r':
		case '\n':
			return new Endl();
		case '{':
			if (!is_function_args)
				Id::id_table().push_front(std::map<std::string, Id*>());
			else
				is_function_args = false;
			return BasicToken[std::string(1, content[i])];
		case '}':
			for (auto& a : Id::id_table().front())
			{
				token_stream._rem_tok_vec.push_back(a.second);
			}
			Id::id_table().pop_front();

			return BasicToken["}"];
		case '|':
		case '-':
		case '+':
		case '&':
			if (i + 1 < content.size() && content[i + 1] == cur_char)
			{
				auto ret = BasicToken[std::string(2, cur_char)];
				i++;
				return ret;
			}
			else if (i + 1 < content.size() && content[i + 1] == '=')
			{
				i++;
				return BasicToken[std::string{ cur_char,'=' }];
			}
			//->
			else if (cur_char == '-' && i + 1 < content.size() && content[i + 1] == '>')
			{
				i++; 
				return BasicToken[std::string{ cur_char,'>' }];
			}
			else
				return BasicToken[std::string(1, cur_char)];
		case '~':
		case '>':
		case '<':
		case '=':
		case '!':
		case '*':
		{
			std::string str = std::string(1, content[i]);
			if (i + 1 < content.size() && content[i + 1] == '=')
			{
				str += content[++i];
			}
			else if (i + 1 < content.size() && (cur_char == '<' || cur_char == '>') && cur_char == content[i + 1])
			{
				str += content[++i];
				if (i + 1 < content.size() && content[i + 1] == '=')
					str += content[++i];
			}
			return BasicToken[str];
		}
		case ',':
		case '%':
		case '?':
		case ';':
		case ':':
		case '.':
		case '[':
		case ']':
		case '^':
		case ')':
			return BasicToken[std::string(1, content[i])];
		case '\t':
		case ' ':
			break;
		case '(':
			if (is_function_args)
				Id::id_table().push_back(std::map<std::string, Id*>());
			return BasicToken["("];
		case '/':
			if (i + 1 < content.size() && content[i + 1] == '/')
			{
				while (i + 1 < content.size() && content[i++] != '\n')
					continue;
				// in case that the uncorrect of line no
				i -= 2;
				break;
			}
			else if (i + 1 < content.size() && content[i + 1] == '*')
			{
				i += 2;
				while (i < content.size())
				{
					if (content[i] == '\n' || content[i] == '\r')
					{
						token_stream.push_back(new Endl());
					}
					i++;
					if (content[i] == '*')
					{
						i++;
						if (i < content.size() && content[i] == '/')
							break;
					}
				}
				break;
			}
			else if (i + 1 < content.size() && content[i + 1] == '=')
			{
				token_stream.push_back(BasicToken["/="]);
				i++;
				break;
			}
			return BasicToken["/"];
		case '0':case '1':case '2':case '3':case '4':case '5':case '6':
		case '7':case '8':case '9':
		{
			auto ret= parse_number(content, i);
			i--;
			return ret;
		}
		default:
			return parse_word(content, i);
		}
	}
	return nullptr;
}

Token* Mer::parse_number(const std::string& str, size_t& pos)
{
	unsigned long long ret = 0;
	// hex or oct number.
	if (pos < str.size() && str[pos] == '0')
	{
		pos++;
		if (pos < str.size() && tolower(str[pos]) == 'x')
			return num_process::hex_to_dec(str, ++pos);
		if (str[pos] != '.')
			return num_process::oct_to_dec(str, pos);
	}
	// integer part
	for (; pos < str.size(); pos++)
	{
		if (isdigit(str[pos]))
		{
			ret = ret * 10 + (str[pos] - 48);
		}
		else
		{
			break;
		}
	}
	long double tmp = 0.0;
	long double tmp2 = 1;
	if (str[pos] == '.')
	{
		pos++;
		for (; pos < str.size(); pos++)
		{
			if (isdigit(str[pos]))
			{
				tmp2 /= 10;
				tmp = tmp + tmp2 * (str[pos] - (size_t)48);
			}
			else if (tolower(str[pos]) == 'l')
			{
				pos++;
				return new Literal<long double>((long double)ret + tmp, Tag::LREAL);
			}
			else if (tolower(str[pos]) == 'f')
			{
				pos++;
				return new Literal<double>((double)ret + tmp, Tag::REAL);
			}
			else
				return new Literal<double>((double)ret + tmp, Tag::REAL);
		}
		return new Literal<double>((double)ret + tmp, Tag::REAL);
	}
	return num_process::integer_to_token(ret, str, pos);
}

Token* Mer::parse_word(const std::string& str, size_t& pos)
{
	std::string ret;
	bool first_char = true;
	for (; pos < str.size(); pos++)
	{
		if (first_char)
		{
			first_char = false;
			if (isalpha(str[pos]) || str[pos] == '_')
				ret += str[pos];
			continue;
		}
		else
		{
			if (isalnum(str[pos]) || str[pos] == '_')
				ret += str[pos];
			else
			{
				pos--;
				break;
			}
		}
	}
	// #define replacement.
	if (define_replace.count(ret))
	{
		auto result = define_replace.find(ret);
		size_t i = 0;
		while (i < result->second.size())
		{
			token_stream.push_back(parse_single_token(result->second, i));
			i++;
		}
		// to return a token ,if we return nullptr, the lexer will terminated, so we must return the back of the token_stream
		auto ret_tok = token_stream.content.back();
		token_stream.pop_back();
		return ret_tok;
	}
	auto result = Mer::BasicToken.find(ret);
	if (result != Mer::BasicToken.end())
		return result->second;


	auto id_result = Id::find(ret);
	if (id_result != nullptr)
		return id_result;
	else
		return new Id(ret);
}

Token* Mer::parse_string(const std::string& str, size_t& pos)
{
	std::string value;
	// jump over '
	pos++;
	for (; pos < str.size(); pos++)
	{
		if (str[pos] == '\"')
			break;
		if (str[pos] == '\\')
		{
			pos++;
			if (pos >= str.size())
				throw LexerError("out of range");
			char tmp = _convert_escape(str[pos]);
			value += tmp;
		}
		else
			value += str[pos];
	}
	return new String(value);
}
void Mer::build_token_stream(const std::string& content) {
	std::string tmp_str;
	token_stream.push_back(new Endl());
	for (size_t i = 0; i < content.size(); i++)
	{
		char cur_char = content[i];
		switch (content[i])
		{
		case '#':
			Preprocessor::preprocessor(content, i);
			// in case it eat \n
			i--;
			break;
		case '\'':
			token_stream.push_back(parse_char(content, i));
			break;
		case '\"':
			token_stream.push_back(parse_string(content, i));
			break;
		case '\r':
		case '\n':
			token_stream.push_back(new Endl());
			break;
		case '{':
			if (!is_function_args)
				Id::id_table().push_front(std::map<std::string, Id*>());
			else
				is_function_args = false;
			token_stream.push_back(BasicToken[std::string(1, content[i])]);
			break;
		case '}':
			for (auto& a : Id::id_table().front())
			{
				token_stream._rem_tok_vec.push_back(a.second);
			}
			Id::id_table().pop_front();

			token_stream.push_back(BasicToken["}"]);
			break;
		case ',':
			token_stream.push_back(BasicToken[","]);
			break;
		case '|':
		case '-':
		case '+':
		case '&':
			if (i + 1 < content.size() && content[i + 1] == cur_char)
			{
				token_stream.push_back(BasicToken[std::string(2, cur_char)]);
				i++;
				break;
			}
			else if (i + 1 < content.size() && content[i + 1] == '=')
			{
				i++;
				token_stream.push_back(BasicToken[std::string{ cur_char,'=' }]);
			}
			//->
			else if (cur_char == '-' && i + 1 < content.size() && content[i + 1] == '>')
			{
				i++; token_stream.push_back(BasicToken[std::string{ cur_char,'>' }]);
			}
			else
				token_stream.push_back(BasicToken[std::string(1, cur_char)]);
			break;
		case '~':
		case '>':
		case '<':
		case '=':
		case '!':
		case '*':
		{
			std::string str = std::string(1, content[i]);
			if (i + 1 < content.size() && content[i + 1] == '=')
			{
				str += content[++i];
			}
			else if (i + 1 < content.size() && (cur_char == '<' || cur_char == '>') && cur_char == content[i + 1])
			{
				str += content[++i];
				if (i+1 < content.size() && content[i+1] == '=')
					str += content[++i];
			}
			token_stream.push_back(BasicToken[str]);
			break;
		}
		case '%':
		case '?':
		case ';':
		case ':':
		case '.':
		case '[':
		case ']':
		case '^':
		case ')':
			token_stream.push_back(BasicToken[std::string(1, content[i])]);
			break;
		case '\t':
		case ' ':
			break;
		case '(':
			if (is_function_args)
				Id::id_table().push_back(std::map<std::string, Id*>());
			token_stream.push_back(BasicToken["("]);
			break;
		case '/':
			if (i + 1 < content.size() && content[i + 1] == '/')
			{
				while (i + 1 < content.size() && content[i++] != '\n')
					continue;
				// in case that the uncorrect of line no
				i -= 2;
				break;
			}
			else if (i + 1 < content.size() && content[i + 1] == '*')
			{
				i += 2;
				while (i < content.size())
				{
					if (content[i] == '\n' || content[i] == '\r')
					{
						token_stream.push_back(new Endl());
					}
					i++;
					if (content[i] == '*')
					{
						i++;
						if (i < content.size() && content[i] == '/')
							break;
					}
				}
				break;
			}
			else if (i + 1 < content.size() && content[i + 1] == '=')
			{
				token_stream.push_back(BasicToken["/="]);
				i++;
				break;
			}
			token_stream.push_back(BasicToken["/"]);
			break;

		case '0':case '1':case '2':case '3':case '4':case '5':case '6':
		case '7':case '8':case '9':
			token_stream.push_back(parse_number(content, i));
			i--;
			break;
		default:
			token_stream.push_back(parse_word(content, i));
			break;
		}
	}
	token_stream.push_back(END_TOKEN);
}

void Mer::new_build_token_stream(const std::string& content)
{
	token_stream.push_back(new Endl());
	size_t i = 0;
	while (i<content.size()) 
	{
		if (content[i] == '#')
		{
			Preprocessor::preprocessor(content, i);
			i--;
		}
		else
		{
			Token* tok = parse_single_token(content, i);
			if (tok == nullptr)
				break;
			token_stream.push_back(tok);
		}
		i++;
	}
	token_stream.push_back(END_TOKEN);
}

std::string Mer::get_this_id_string_value()
{
	auto ret = Id::get_value(token_stream.this_token());
	token_stream.match(ID);
	return ret;
}
TokenStream& Mer::TokenStream::operator=(const TokenStream& tok_stream)
{
	content.assign(tok_stream.content.begin(), tok_stream.content.end());
	pos = 0;
	return *this;
}
Token* Mer::TokenStream::next_token()
{
	if (pos + 1 < content.size())
		return content[pos + 1];
	else
		throw Error("token_stream out of range");
}
void Mer::TokenStream::add(Token* tok)
{
	content.insert(content.begin() + pos + 1, tok);
}
void Mer::TokenStream::advance()
{
	pos++;
	if (pos >= content.size())
	{
		content.push_back(END_TOKEN);
		throw Error("to the end of token_stream");
	}
}
void Mer::TokenStream::match(Tag t)
{
	// to check the token whether it is right, and if matched call advance, or throw an error.
	// example: match(PLUS); 
	if (this_token()->get_tag() == Mer::Tag::ENDL)
	{
		advance();
		match(t);
	}
	else if (this_token()->get_tag() == t)
		advance();
	else
		throw Error("expect " + TagStr[t] + " but get " + this_token()->to_string());
}
void TokenStream::remove_tokens()
{
	int index = -1;
	// delete literal_const and endl value
	for (const auto& a : content)
	{
		index++;
		switch (a->get_tag())
		{
		case ENDL:
		case CHAR_LIT:
		case INTEGER:
		case REAL:
		case STRING:
		{
			auto tmp = a;
			delete tmp;
			break;
		}
		default:
			break;
		}
	}
	content.clear();
	Endl::current_line = 0;
	// delete Ids
	for (auto& a : Id::id_table())
	{
		for (auto& b : a)
		{
			delete b.second;
		}
	}
	Id::id_table().clear();
	Id::id_table().push_back(std::map<std::string, Id*>());

	// delete container
	content.clear();
}
void TokenStream::clear()
{
	for (auto& a : _rem_tok_vec)
		delete a;
	_rem_tok_vec.clear();
	remove_tokens();

	pos = 0;
}

Token* Mer::parse_char(const std::string& str, size_t& pos)
{
	// ignore \'
	pos++;
	if (str[pos] == '\\')
	{
		char result = _convert_escape(str[++pos]);
		if (str[++pos] != '\'')
			throw LexerError("illegal escape character!");
		return new CharToken(result);
	}
	char result = str[pos++];
	if (str[pos] != '\'')
		throw LexerError("illegal character!");
	return new CharToken(result);
}

std::string Endl::to_string()const
{
	return "\n";
}
size_t Endl::get_value(Token* tok)
{
	if (tok->get_tag() == ENDL)
	{
		return static_cast<Endl*>(tok)->line_no;
	}
	else
		throw LexerError("convert failed");
}