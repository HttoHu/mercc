/*
	 GNU GENERAL PUBLIC LICENSE
						   Version 3, 29 June 2007

	 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
	 Everyone is permitted to copy and distribute verbatim copies
	 of this license document, but changing it is not allowed.
*/
#pragma once
#include "../type.hpp"
#include "../function.hpp"
#include "../namespace.hpp"
// offer the most basic io method and string operation
namespace Mer
{
	class Namespace;
	extern Namespace *mstd;
	void set_io();
}