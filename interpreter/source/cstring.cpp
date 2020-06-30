/*
GNU GENERAL PUBLIC LICENSE
					   Version 3, 29 June 2007

 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.
*/
#include "../include/clib/cstring.hpp"
#include "../include/type.hpp"
#include "../include/function.hpp"
#include "../include/namespace.hpp"
#include "../include/memory.hpp"
namespace Mer {
	void _register_internal_function(std::string name, type_code_index ret_type, const std::vector<type_code_index>& param_list, std::function<Mem::Object(const std::vector<Mem::Object>&)> mf, Namespace* _nsp = this_namespace);

	Mem::Object mstrlen(const std::vector<Mem::Object>& args)
	{
		int ret = 0;
		size_t pos = std::static_pointer_cast<Mem::Pointer>(args[0])->get_value();
		while (1) {
			char * data = Mer::mem[pos]->get_raw_data();
			int len = Mer::mem[pos]->length();
			for (int i = 0; i < len; i++)
			{
				if (!data[i])
					return std::make_shared<Mem::Int>(ret);
				ret++;
			}
			pos++;
		}
		return std::make_shared<Mem::Int>(ret);
	}

	// the first args type's len should equal to the second one's len.
	// or some terrible things would happen.
	// I reckon no one would call it to copy some strange blocks.
	Mem::Object mstrcpy(const std::vector<Mem::Object>& args)
	{

		size_t dest_pos = std::static_pointer_cast<Mem::Pointer>(args[0])->get_value();
		size_t src_pos = std::static_pointer_cast<Mem::Pointer>(args[1])->get_value();
		while (1) {
			char *dest_data = Mer::mem[dest_pos]->get_raw_data();
			char * src_data = Mer::mem[src_pos]->get_raw_data();
			int len = Mer::mem[src_pos]->length();
			for (int i = 0; i < len; i++)
			{
				if (!src_data[i])
					goto end_pos;
				dest_data[i] = src_data[i];
			}
			dest_pos++; src_pos++;
		}
	end_pos:
		return args[0]->clone();
	}
	// strncpy(char *dest,char* src,int count);
	Mem::Object mstrncpy(const std::vector<Mem::Object>& args)
	{

		size_t dest_pos = std::static_pointer_cast<Mem::Pointer>(args[0])->get_value();
		size_t src_pos = std::static_pointer_cast<Mem::Pointer>(args[1])->get_value();
		int count = std::static_pointer_cast<Mem::Int>(args[2])->get_value();
		// to count how many chars we've copied. 
		int cpy_cnt = 0;
		bool append_zero_mod = false;
		while (cpy_cnt < count) {
			char *dest_data = Mer::mem[dest_pos++]->get_raw_data();
			char * src_data = Mer::mem[src_pos++]->get_raw_data();
			int len = Mer::mem[dest_pos]->length();
			for (int i = 0; i < len; i++)
			{
				cpy_cnt++;
				if (append_zero_mod) 
					dest_data[i] = 0;
				else
				{
					dest_data[i] = src_data[i];
					if (!src_data[i])
						append_zero_mod = true;
				}
			}
		}
		return args[0]->clone();
	}
	// memset( void *dest, int v, int size)  
	// I knew the type of size should be size_t ,however I didn't want to create a unsigned type.
	Mem::Object mmemset(const std::vector<Mem::Object>& args)
	{
		size_t dest_pos = std::static_pointer_cast<Mem::Pointer>(args[0])->get_value();
		int set_num = *(int*)(args[1]->get_raw_data());
		int size = *(int*)(args[2]->get_raw_data());
		int i = 0;
		while (i < size)
		{
			char *dest_data = Mer::mem[dest_pos]->get_raw_data();
			int len = Mer::mem[dest_pos++]->length();
			for (int j = 0; j < len; j++)
				dest_data[j] = set_num;
			i += len;
		}
		return args[0]->clone();
	}

	void set_cstring()
	{
		_register_internal_function("strlen", Mem::INT, { Mem::CHAR + 1 }, mstrlen);
		_register_internal_function("strcpy", Mem::CHAR + 1, { Mem::CHAR + 1,Mem::CHAR + 1 }, mstrcpy);
		_register_internal_function("strncpy", Mem::CHAR + 1, { Mem::CHAR + 1,Mem::CHAR + 1 ,Mem::INT }, mstrncpy);
		_register_internal_function("memset", Mem::BVOID + 1, { Mem::BVOID + 1,Mem::INT ,Mem::INT }, mmemset);
	}
}