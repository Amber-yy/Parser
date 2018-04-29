#include "FunctionDef.h"
#include "Types.h"
#include "AStree.h"

Parser *FunctionDef::parser;

struct FunctionDef::Data
{
	AStreeRef block;	
	Types *type;
	FunctionDef *next;
	void *stack;
	void *local;
	void *returnValue;
	int currentOffset;
	int size;
	bool isReturn;
	bool isRunning;
};

FunctionDef::FunctionDef()
{
	data = new Data;
	data->isReturn = false;
	data->isRunning = false;
	data->next = nullptr;
	data->size = data->type->toFunction()->returnType->getSize();
	if (data->size)
	{
		data->returnValue = new char[data->size];
	}
}

FunctionDef::~FunctionDef()
{
	delete data->next;
	if (data->size)
	{
		delete data->returnValue;
	}
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

bool FunctionDef::isRunning()
{
	return data->isRunning;
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

void FunctionDef::setStack(void * s)
{
	data->stack = s;
	data->local = s;
}

void FunctionDef::addArgValue(void * s, int size)
{
	memcpy(data->local, s, size);
	data->local = (char *)data->local + size;
}

void FunctionDef::setReturnValue(void * s)
{
	memcpy(data->returnValue, s, data->size);
}

void * FunctionDef::getReturnValue()
{
	return data->returnValue;
}

void FunctionDef::run()
{
	data->isRunning = true;
	data->block->eval();
	data->isRunning = false;
}

int FunctionDef::getOffset()
{
	return data->currentOffset;
}

Types * FunctionDef::getType()
{
	return data->type;
}

FunctionDef * FunctionDef::copy()
{
	if (data->next)
	{
		return data->next;
	}

	data->next = new FunctionDef;
	data->next->data->block = data->block->copy(data->next, nullptr);

	return data->next;
}
