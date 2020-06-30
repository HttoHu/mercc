/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
using type_code_index = int;
#include<iostream>
#include<vector>
#include <deque>
#include "../basic_objects.hpp"
namespace Mer
{
	namespace Container
	{
		class Vector :public Mem::Value
		{
		public:
			static int vector_type_code;
			Vector() {}
			Vector(int count) :content(count) {}
			Vector(int count, Mem::Object obj);
			Vector(const std::vector<Mem::Object>& objs) :content(objs) {}
			Vector(std::vector<Mem::Object>&& objs) :content(objs) {}
			Mem::Object operator[](const Mem::Object& index)override;
			Mem::Object clone()const override;
			std::vector<Mem::Object> content;
		};
		class Deque :public Mem::Value {
		public:
			static int deque_type_code;
			Deque() {}
			Deque(int count) :content(count) {}
			Deque(int count, Mem::Object obj);
			Deque(const std::deque<Mem::Object>& objs) :content(objs) {}
			Deque(std::deque<Mem::Object>&& objs) :content(objs) {}
			Mem::Object operator[](const Mem::Object &index)override;
			Mem::Object clone()const override;
			std::deque<Mem::Object> content;

		};
		void using_vector();
		void using_deque();
		void register_new_vector_type(type_code_index element_type);
		void register_new_deque_type(type_code_index element_type);
	}
}