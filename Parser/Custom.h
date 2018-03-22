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
	std::string name;
	int size;
};

struct UnionDef
{
	UnionDef();
	std::vector<std::string> members;
	std::vector<Types *> types;
	std::string name;
	int size;
};

struct EnumDef
{
	std::string name;
};
