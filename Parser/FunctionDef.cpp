#include "FunctionDef.h"
#include "Types.h"
#include "AStree.h"

Parser *FunctionDef::parser;

struct FunctionDef::Data
{
	Types *type;
	AStreeRef block;
};

FunctionDef::FunctionDef()
{
	data = new Data;
}

FunctionDef::~FunctionDef()
{
	delete data;
}

void FunctionDef::setBlock(AStreeRef & ast)
{
	data->block = std::move(ast);
}

void FunctionDef::setType(Types * tp)
{
	data->type = tp;
}

Types * FunctionDef::getType()
{
	return data->type;
}
