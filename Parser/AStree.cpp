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

bool AStree::isWhileState()
{
	return false;
}

bool AStree::isDoWhileState()
{
	return false;
}

bool AStree::isBreakState()
{
	return false;
}

bool AStree::isSwitchState()
{
	return false;
}

bool AStree::parseCondition(Result res)
{
	char *data = (char *)res.value + res.offset;

	for (int i = 0; i < res.type->getSize(); ++i)
	{
		if (data[i] != 0)
		{
			return true;
		}
	}

	return false;
}

SwitchState * AStree::toSwitchState()
{
	return dynamic_cast<SwitchState *>(this);
}

BreakState * AStree::toBreakState()
{
	return dynamic_cast<BreakState *>(this);
}

DoWhileState * AStree::toDoWhileState()
{
	return dynamic_cast<DoWhileState *>(this);
}

WhileState * AStree::toWhileState()
{
	return dynamic_cast<WhileState *>(this);
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

Block::Block()
{
	type = StateBlock;
}

Types * Block::getType()
{
	return nullptr;
}

Result Block::eval()
{
	Block *loop=nullptr;
	AStree *temp = block;

	while (temp)
	{
		Block *q = temp->toBlock();
		if (q->type == LoopBlock|| q->type == SwitchBlock)
		{
			loop = q;
			break;
		}

		temp = temp->block;
	}

	for (int i = 0; i < statements.size(); ++i)
	{
		statements[i]->eval();
		if (function->isReturned() || (loop&&loop->isBreak))
		{
			break;
		}
	}

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
	if (expr.get() != nullptr)
	{
		expr->eval();
	}

	function->setReturned();

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
	bool ok = parseCondition(res);

	if (ok)
	{
		conTrue->eval();
	}
	else if (conFalse.get() != nullptr)
	{
		conFalse->eval();
	}

	return Result();
}

bool IfState::isLeftValue()
{
	return false;
}

bool IfState::isIfState()
{
	return true;
}

Types * WhileState::getType()
{
	return nullptr;
}

Result WhileState::eval()
{
	isBreak = false;

	while (parseCondition(condition->eval())&&!isBreak&&!function->isReturned())
	{
		state->eval();
	}

	return Result();
}

bool WhileState::isLeftValue()
{
	return false;
}

bool WhileState::isWhileState()
{
	return true;
}

Types * DoWhileState::getType()
{
	return nullptr;
}

Result DoWhileState::eval()
{
	isBreak = false;

	do
	{
		state->eval();
	}while (parseCondition(condition->eval())&&!isBreak&&!function->isReturned());

	return Result();
}

bool DoWhileState::isLeftValue()
{
	return false;
}

bool DoWhileState::isDoWhileState()
{
	return true;
}

Types * BreakState::getType()
{
	return nullptr;
}

Result BreakState::eval()
{
	Block *loop = nullptr;
	AStree *temp = block;

	while (temp)
	{
		Block *q = temp->toBlock();
		if (q->type == LoopBlock|| q->type == SwitchBlock)
		{
			loop = q;
			break;
		}

		temp = temp->block;
	}

	loop->isBreak = true;
}

bool BreakState::isLeftValue()
{
	return false;
}

bool BreakState::isBreakState()
{
	return true;
}
