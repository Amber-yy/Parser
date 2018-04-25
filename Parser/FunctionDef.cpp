#include "FunctionDef.h"
#include "Types.h"
#include "AStree.h"

Parser *FunctionDef::parser;

struct FunctionDef::Data
{
	AStreeRef block;	
	Types *type;
	void *stack;
	void *local;
	int currentOffset;
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

void * FunctionDef::getLocal()
{
	return data->local;
}

void * FunctionDef::getStack()
{
	return data->stack;
}

void FunctionDef::setOffset(int off)
{
	data->currentOffset = off;
}

int FunctionDef::getOffset()
{
	return data->currentOffset;
}

Types * FunctionDef::getType()
{
	return data->type;
}
