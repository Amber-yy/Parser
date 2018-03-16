#pragma once

#include "Types.h"

#include <vector>
#include <string>

struct StructDef
{
	StructDef();
	std::vector<std::string> members;
	std::vector<int> offSets;
	std::vector<Types *> types;
	int size;
};

struct UnionDef
{
	UnionDef();
	std::vector<std::string> members;
	std::vector<Types *> types;
	int size;
};

struct EnumDef
{

};
