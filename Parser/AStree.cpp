#include "AStree.h"
#include "Parser.h"
#include "FunctionDef.h"
#include "Types.h"

Parser * AStree::parser;

AStree::AStree()
{
	block = nullptr;
}

AStree::~AStree()
{

}

bool AStree::isBlock()
{
	return false;
}

bool AStree::isReturnState()
{
	return false;
}

bool AStree::isIfState()
{
	return false;
}

IfState * AStree::toIfState()
{
	return dynamic_cast<IfState *>(this);
}

ReturnState * AStree::toReturnState()
{
	return dynamic_cast<ReturnState *>(this);
}

Block * AStree::toBlock()
{
	return dynamic_cast<Block *>(this);
}

Types * Block::getType()
{
	return nullptr;
}

Result Block::eval()
{
	return Result();
}

bool Block::isLeftValue()
{
	return false;
}

bool Block::isBlock()
{
	return true;
}

Types * ReturnState::getType()
{
	return function->getType()->toFunction()->returnType;
}

Result ReturnState::eval()
{
	return Result();
}

bool ReturnState::isLeftValue()
{
	return false;
}

bool ReturnState::isReturnState()
{
	return true;
}

Types * IfState::getType()
{
	return nullptr;
}

Result IfState::eval()
{
	Result res=condition->eval();
	bool ok = false;

	for (int i = 0; i < res.type->getSize(); ++i)
	{
		if (*(char *)res.value != 0)
		{
			ok = true;
			break;
		}
	}

	if (ok)
	{
		return conTrue->eval();
	}
	
	return conFalse->eval();
}

bool IfState::isLeftValue()
{
	return false;
}

bool IfState::isIfState()
{
	return true;
}
