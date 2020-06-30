#pragma once
/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
/*
	function_model: 
	mem-> |begin: local variable, program:| local vars...|....
	|function_call2:[current] args, local vars|function_call1 :args ,local vars|
	function call all pushed back, glo var are all in the front.

*/
#include <vector>
#include <stack>
#include <map>
#include <set>
#include "basic_objects.hpp"

namespace Mer
{
	const int mem_max_size = 10000;
	class Memory
	{
	public:
		Memory()
		{
			stack_mem = new Mem::Object[capacity];
			block_flag.push_back(0);
		}
		size_t new_block();
		void new_func(size_t off);
		void end_func();
		size_t push(int size);
		size_t push();
		size_t end_block();
		size_t& get_current()
		{
			return current;
		}
		Mem::Object& operator[]  (size_t in);
		~Memory()
		{
			delete[] stack_mem;
		}
		size_t& get_index() {
			return index;
		}
		void reset_func_block_size() {
			function_block_size = 0;
		}
		int function_block_size=0;
		size_t get_capacity() { return capacity; }

		size_t reserve_glo_pos(size_t size);
		void reset();
		void check()
		{
			while (index + current > capacity / 2)
			{
				alloc();
				capacity *= 2;
			}
		}
	private:
		// reserve to ret value. If you need to return a structure, the return value will put into [0]-[99].
		size_t glo_var_index = 200;
		//alloc for memory
		void alloc();
		// how many var's the funciton contains;

		size_t index = 0;
		size_t current = 0;
		size_t capacity = mem_max_size;
		std::stack<size_t> call_stack;
		std::vector<size_t> block_flag;
		Mem::Object *stack_mem;
	};
	class RawMem
	{
	public:
		char *mem;
		size_t capacity;

	};
	extern Memory mem;
}