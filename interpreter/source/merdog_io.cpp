/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
// file_stream
/*
 mem:struct
 - 0: filename
 - 1: std::vector<std::string> file_content;
 --------------------
 method
 => open(string), open a file by its name
 => string read(int), read line_no text;
 => void write(), write the content to the file.
*/
#include <sstream>
#include <algorithm>
#include "../include/basic_objects.hpp"
#include "../include/environment.hpp"
#include "../include/clib/merdog_io.hpp"
#include "../include/compound_box.hpp"
#include "../include/word_record.hpp"
#include <fstream>
#include <cstring>
#ifdef USING_CXX17
#include <filesystem>
#endif
extern std::string pre_input_content;
namespace Mer
{

	void _register_internal_function
	(std::string name,  type_code_index ret_type, const std::vector<type_code_index>& param_list, std::function<Mem::Object(const std::vector<Mem::Object>&)> mf,Namespace* _nsp=this_namespace) {
		SystemFunction* tmp = new SystemFunction(ret_type, mf);
		tmp->set_param_types(param_list);
		tmp->is_completed = true;
		_nsp->set_new_func(name, tmp);
	}
	namespace
	{
		template<typename T>
		void print_str(const T &str)
		{
#ifndef DISABLEIO
			std::cout << str;
#else
			output_buff += str;
#endif
		}
		template<typename T>
		void format_print(const char* m, const T & info)
		{
			static char buf[50];
			sprintf_s(buf, m, info);
			int len = strlen(buf);
#ifndef DISABLEIO
			printf("%s",buf);
#else
			for (int i = 0; i < len; i++)
				output_buff += buf[i];
#endif
		}
		std::shared_ptr<Mem::Int> _make_int_obj(int n)
		{
			return std::make_shared<Mem::Int>(n);
		}
		Mem::Object _init_str_n(const std::vector<Mem::Object>& args)
		{
			int count = Mem::get_raw<int>(args[0]);
			char c = std::static_pointer_cast<Mem::Char>(args[1])->get_value();
			return std::make_shared<Mem::String>(std::string(count, c));
		}
		Mem::Object _str_find_str(const std::vector<Mem::Object>& args) {
			auto tmp = std::static_pointer_cast<Mem::String>(parents_vec.back())->str;
			std::string str = std::static_pointer_cast<Mem::String> (args[0])->str;
			int spos = std::static_pointer_cast<Mem::Int>(args[1])->get_value();
			return std::make_shared<Mem::Int>(tmp.find(str,spos));
		}
		Mem::Object _substr(const std::vector<Mem::Object>& args)
		{
			auto tmp = std::static_pointer_cast<Mem::String>(parents_vec.back());
			auto off = std::static_pointer_cast<Mem::Int>(args[0]);
			auto size = std::static_pointer_cast<Mem::Int>(args[1]);
			return std::make_shared<Mem::String>(tmp->to_string().substr(off->get_value(), size->get_value()));
		}
		Mem::Object _str_size(const std::vector<Mem::Object>& args)
		{
			auto tmp = std::static_pointer_cast<Mem::String>(parents_vec.back());
			return std::make_shared<Mem::Int>(tmp->to_string().size());
		}
		Mem::Object _str_find(const std::vector<Mem::Object> &args) {
			auto tmp = std::static_pointer_cast<Mem::String>(parents_vec.back());
			std::string tmp_str = tmp->str;
			char ch = std::static_pointer_cast<Mem::Char>(args[0])->get_value();
			int startPos = std::static_pointer_cast<Mem::Int>(args[1])->get_value();
			return std::make_shared<Mem::Int>(tmp->str.find(ch, startPos));
		}
		Mem::Object _str_to_lower_case(const std::vector<Mem::Object>& args) {
			std::string str = std::static_pointer_cast<Mem::String>(parents_vec.back())->str;
			std::transform(str.begin(), str.end(), str.begin(), tolower);
			return std::make_shared<Mem::String>(str);
		}
		Mem::Object _str_to_upper_case(const std::vector<Mem::Object>& args) {
			std::string str = std::static_pointer_cast<Mem::String>(parents_vec.back())->str;
			std::transform(str.begin(), str.end(), str.begin(), toupper);
			return std::make_shared<Mem::String>(str);
		}
		Mem::Object _cout(const std::vector<Mem::Object>& args)
		{
			std::string content = args[0]->to_string();
			int cnt = 1;
			int i = 0;

			while (i < content.size())
			{
				std::string format_output_tag="%";
				switch (content[i])
				{
				case '%':
				{
					i++;
					if (i == content.size())
						throw Error("printf first arg error");
					if (content[i] == '.')
					{
						format_output_tag += content[i++];
						while (isdigit(content[i]))
						{
							format_output_tag += content[i++];
						}
					}
					if (content[i] == 'l')
						format_output_tag += content[i++];
					switch (content[i])
					{
					case '+':
					case '#':
					case 'd':
					case 'o':
					case 'O':
					case 'x':
					case 'X':
					{
						char info[4] = { '%',content[i],'\0' };
						if (content[i] == '#'||content[i]=='+')
						{
							i++;
							info[2] = content[i];
							info[3] = '\0';
						}
						i++;
						format_print(info, std::static_pointer_cast<Mem::Int>(args[cnt++]->Convert(Mem::INT))->get_value());
						continue;
					}
					case 'f':
					{
						i++;
						format_output_tag += 'f';
						format_print(format_output_tag.c_str(), 
							std::static_pointer_cast<Mem::Double>(args[cnt++]->Convert(Mem::DOUBLE))->get_value());
						continue;
					}
					case 'c':
					{
						i++;
						print_str(args[cnt++]->Convert(Mem::CHAR)->to_string());
						continue;
					}
					case 's':
					{
						i++;
						size_t pos = std::static_pointer_cast<Mem::Pointer>(args[cnt++])->get_value();
						while (1) {
							char * data = Mer::mem[pos]->get_raw_data();
							int len = Mer::mem[pos]->length();
							for (int i = 0; i < len; i++)
							{
								if (!data[i])
									goto EP;
								print_str(data[i]);
							}
							pos++;
						}
					EP:
						continue;
					}
					default:
						break;
					}
					break;
				}
				default:
					print_str(content[i++]);
					break;
				}
			}
			return nullptr;
		}

		Mem::Object _scanf(const std::vector<Mem::Object>& args)
		{
			std::string content = args[0]->to_string();
			int cnt = 1;
			int i = 0;
			std::istream& is= pre_input_content.size()? my_stringstream: std::cin;
			try
			{
				while (i < content.size())
				{
					switch (content[i])
					{
					case '%':
					{
						i++;
						if (i == content.size())
							throw std::runtime_error("runtime_error");
						switch (content[i])
						{
						case 'd':
						{
							i++;
							int tmp; 
							is >> tmp;
							std::static_pointer_cast<Mem::Pointer>(args[cnt++])->rm_ref()->operator=(_make_int_obj(tmp));
							continue;
						}
						case 'f':
						{
							i++;
							double tmp; is >> tmp;
							std::static_pointer_cast<Mem::Pointer>(args[cnt++])->rm_ref()->operator=(std::make_shared<Mem::Double>(tmp));
							continue;
						}
						case 'c':
						{
							i++;
							char tmp; is >> tmp;
							std::static_pointer_cast<Mem::Pointer>(args[cnt++])->rm_ref()->operator=(std::make_shared<Mem::Char>(tmp));
							continue;
						}
						case 's':
						{
							i++;
							std::string tmp; is >> tmp;
							size_t pos = std::static_pointer_cast<Mem::Pointer>(args[cnt++])->get_value();
							for (auto a : tmp)
								mem[pos++] = std::make_shared<Mem::Char>(a);
							mem[pos] = std::make_shared<Mem::Char>('\0');
							continue;
						}
						default:
							throw std::runtime_error("scanf error");
						}
						break;
					}
					case ' ':
					case '\t':
						i++;
						break;
					default:
						if (getchar() != content[cnt])
							return _make_int_obj(cnt);
						break;
					}
				}
			}
			catch (...)
			{
				return _make_int_obj(-1);
			}
			return _make_int_obj(cnt);
		}

		Mem::Object _input_char(const std::vector<Mem::Object>& args)
		{
			char obj;
			if (pre_input_content.empty())
				std::cin >> std::noskipws>>obj;
			else
				my_stringstream >>std::noskipws>> obj;
			return std::make_shared<Mem::Char>(obj);
		}
		Mem::Object _put_char(const std::vector<Mem::Object>& args)
		{
			char obj=*(args[0]->get_raw_data());
			print_str(obj);
			return nullptr;
		}

		Mem::Object mgets(const std::vector<Mem::Object>& args)
		{
			size_t dest_pos = std::static_pointer_cast<Mem::Pointer>(args[0])->get_value();
			std::string str;
			if (!pre_input_content.size())
				std::getline(std::cin, str);
			else
				std::getline(my_stringstream, str);
			for (auto a : str)
			{
				*(Mer::mem[dest_pos++]->get_raw_data()) = a;
			}
			return args[0]->clone();
		}

		Mem::Object mputs(const std::vector<Mem::Object>& args)
		{
			size_t dest_pos = std::static_pointer_cast<Mem::Pointer>(args[0])->get_value();
			while (true)
			{
				char cur = *mem[dest_pos]->get_raw_data();
				if (cur == '\0')
				{
					print_str('\n');
					break;
				}
				print_str(cur);
				dest_pos++;
			}
			return std::make_shared<Mem::Int>(1);
		}
	}

	Namespace* mstd = new Namespace(nullptr);

	void set_io()
	{
		Mer::SystemFunction* substr = new SystemFunction(Mem::BasicType::STRING, _substr);
		Mer::SystemFunction* str_size = new SystemFunction(Mem::BasicType::INT, _str_size);
		Mer::SystemFunction* find_ch = new SystemFunction(Mem::BasicType::INT,_str_find);
		Mer::SystemFunction* find_str = new SystemFunction(Mem::BasicType::INT, _str_find_str);
		Mer::SystemFunction* to_lower = new SystemFunction(Mem::BasicType::STRING, _str_to_lower_case);
		Mer::SystemFunction* to_upper = new SystemFunction(Mem::BasicType::STRING, _str_to_upper_case);
		Mer::SystemFunction* _mer_scanf = new SystemFunction(Mem::BasicType::INT, _scanf);
		Mer::SystemFunction* cout = new SystemFunction(Mem::BasicType::BVOID, _cout);
		_mer_scanf->dnt_check_param();
		cout->dnt_check_param();
		// set string===========================================
		substr->set_param_types({ Mer::Mem::BasicType::INT, Mer::Mem::BasicType::INT });
		str_size->set_param_types({ });
		find_ch->set_param_types({ Mem::BasicType::CHAR,Mem::BasicType::INT });
		find_str->set_param_types({ Mem::BasicType::STRING,Mem::BasicType::INT});
		member_function_table[Mem::STRING]["substr"] = substr;
		member_function_table[Mem::STRING]["size"] = str_size;
		member_function_table[Mem::STRING]["find"] = find_ch;
		member_function_table[Mem::STRING]["find_str"] = find_str;
		member_function_table[Mem::STRING]["to_lower"] = to_lower;
		member_function_table[Mem::STRING]["to_upper"] = to_upper;
		// string init==================================================================
		auto str_init = new SystemFunction(Mem::STRING, _init_str_n);
		str_init->set_param_types({ Mem::INT,Mem::CHAR });
		type_init_function_map[InitKey(Mem::STRING, std::vector<type_code_index>{ Mem::INT, Mem::CHAR })] = str_init;
		//======================================================
		root_namespace->set_new_func("printf", cout);
		root_namespace->set_new_func("scanf", _mer_scanf);
		_register_internal_function("getchar", Mem::CHAR, {}, _input_char, root_namespace);
		_register_internal_function("putchar", Mem::BVOID, { Mem::CHAR }, _put_char, root_namespace);
		_register_internal_function("gets", Mem::CHAR+1, {Mem::CHAR+1}, mgets, root_namespace);
		_register_internal_function("puts", Mem::INT, { Mem::CHAR + 1 }, mputs, root_namespace);


	}
}