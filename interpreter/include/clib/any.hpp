/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include <string>
#include <iostream>

//#define COMPILE_MERDOG_NEED_CXX17
#ifndef USING_CXX17
namespace mer
{
#define TYPE_CHECK
	//#undef TYPE_CHECK
	class placeholder
	{
	public:
		virtual ~placeholder() {}
		virtual placeholder* clone() = 0;
		virtual const std::type_info& type()noexcept = 0;
	};
	template<typename ValueType>
	class holder final :public placeholder
	{
	public:
		holder(const ValueType vt) :value(vt) {}
		placeholder* clone()override
		{
			return new holder<ValueType>(value);
		}
		virtual const std::type_info& type()noexcept override
		{
			return typeid(ValueType);
		}
		ValueType value;
	};

	class any
	{
	public:
		any() :content(nullptr) {}
		template<typename ValueType>
		any(const ValueType& value)
			: content(new holder<ValueType>(value)) {
		}
		any(const char* str) :any(std::string(str)) {}
		any(const any& other)
			: content(other.content ? other.content->clone() : 0) {}
		template <class ValueType>
		any& operator=(ValueType&& rhs)
		{
			any(static_cast<ValueType&&>(rhs)).swap(*this);
			return *this;
		}
		any& swap(any& rhs) noexcept
		{
			std::swap(content, rhs.content);
			return *this;
		}
		any& operator=(const any& rhs)
		{
			any(rhs).swap(*this);
			return *this;
		}

		// move assignment
		any& operator=(any&& rhs) noexcept
		{
			rhs.swap(*this);
			any().swap(rhs);
			return *this;
		}
		size_t get_type_length()const { return sizeof(placeholder); }
	private:
		placeholder* content;
		template<typename _Ty>
		friend _Ty any_cast(const any& a);
	};
	// when any_cast fail, it will throw the bad_cast
	class  bad_any_cast
	{
	public:
		const char* what() const
		{
			return "bad mer::any_cast";
		}
	};
	template<typename _Ty>
	_Ty any_cast(const any& a)
	{
#ifdef TYPE_CHECK
		if (typeid(_Ty) != a.content->type())
			throw mer::bad_any_cast();
#endif
		return static_cast<holder<_Ty>*>(a.content)->value;
	}
}
#else
#include <any>
namespace mer
{
	using any = std::any;
	template<typename _Ty>
	_Ty& any_cast(std::any& a)
	{
		return std::any_cast<_Ty>(a);
	}
	template<typename _Ty>
	_Ty& any_cast(std::any&& a)
	{
		return std::any_cast<_Ty>(a);
	}
}
#endif