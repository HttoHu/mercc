/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include <functional>
#include <iostream>
#include "../basic_objects.hpp"
namespace Mer
{


	using _compare_operator = std::function<bool(Mem::Object, Mem::Object)>; 
	extern std::map<type_code_index, _compare_operator> compare_map;

	_compare_operator find_compare_operator(type_code_index s);
	namespace Container
	{
		using SetContent = std::set<Mem::Object, _compare_operator>;
		using MapContent = std::map<Mem::Object, Mem::Object, _compare_operator>;
		class Set :public Mem::Value
		{
		public:
			Set(SetContent&& sc) :data(sc) {}
			Set(const SetContent& sc) :data(sc) {}
			Set(type_code_index element_type,const std::vector<Mem::Object>& vec);
			Set(type_code_index element_type);
			static type_code_index type_code;
			Mem::Object operator[](const Mem::Object&)override;
			Mem::Object clone()const override;
			SetContent data;
		};
		class Map :public Mem::Value
		{
		public:
			Map(const MapContent& sc) :data(sc) {}
			Map(type_code_index key);
			static type_code_index type_code;
			Mem::Object operator[](const Mem::Object&)override;
			Mem::Object clone()const override;
			MapContent data;
		};
	}
	void using_set();
	void using_map();
	void register_set(type_code_index element_type);
}