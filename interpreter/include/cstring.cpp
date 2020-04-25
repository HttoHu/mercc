#include "clib/cstring.hpp"
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
	}
	
	// the first args type's len should equal to the second one's len.
	// or some terrible things would happen.
	// I reckon no one would call it to copy some strange blocks.
	Mem::Object mstrcpy(const std::vector<Mem::Object>& args)
	{
		
		size_t dest_pos = std::static_pointer_cast<Mem::Pointer>(args[0])->get_value();
		size_t src_pos= std::static_pointer_cast<Mem::Pointer>(args[1])->get_value();
		while (1) {
			char *dest_data=Mer::mem[dest_pos]->get_raw_data();
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
	// memset( void *dest, int v, int size)  
	// I knew the type of size should be size_t ,however I didn't want to create a unsigned type.
	Mem::Object mmemset(const std::vector<Mem::Object>& args)
	{
		size_t dest_pos = std::static_pointer_cast<Mem::Pointer>(args[0])->get_value();
		int set_num = *(int*)(args[1]->get_raw_data());
		int size = *(int*)(args[2]->get_raw_data());
		int i = 0;
		while (true)
		{
			char *dest_data = Mer::mem[dest_pos]->get_raw_data();
			int len = Mer::mem[dest_pos]->length();
			for (int j = 0; j < len; j++)
				dest_data[i] = set_num;
			i += len;

			if (i >= size)
				break;
		}
		return args[0]->clone();
	}

	void set_cstring()
	{
		_register_internal_function("strlen", Mem::INT, { Mem::CHAR + 1 }, mstrlen);
		_register_internal_function("strcpy", Mem::CHAR+1, { Mem::CHAR + 1,Mem::CHAR+1 }, mstrcpy);
		_register_internal_function("memset", Mem::BVOID + 1, { Mem::BVOID + 1,Mem::INT ,Mem::INT }, mmemset);
	}
}