#include "FunctionDef.h"
#include "Types.h"
#include "AStree.h"

Parser *FunctionDef::parser;

struct FunctionDef::Data
{
	Types *type;
	AStreeRef block;
	bool isReturn;
};

FunctionDef::FunctionDef()
{
	data = new Data;
	data->isReturn = false;
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

bool FunctionDef::isReturned()
{
	return data->isReturn;
}

void FunctionDef::setReturned()
{
	data->isReturn = true;
}

Types * FunctionDef::getType()
{
	return data->type;
}
