#include "AStree.h"
#include "Parser.h"
#include "FunctionDef.h"
#include "Types.h"

Parser * AStree::parser;

AStree::AStree()
{
	block = nullptr;
	type = StateBlock;
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

bool AStree::isForState()
{
	return false;
}

bool AStree::isVariableDefState()
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

VariableDefState * AStree::toVariableDefState()
{
	return dynamic_cast<VariableDefState *>(this);
}

ForState * AStree::toForState()
{
	return dynamic_cast<ForState *>(this);
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
	AStree *loop=nullptr;
	AStree *temp = block;

	while (temp)
	{
		if (temp->type == LoopBlock|| temp->type == SwitchBlock)
		{
			loop = temp;
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
	if (state->isBreakState())
	{
		return Result();
	}
	isBreak = false;

	while (!isBreak && !function->isReturned()&&parseCondition(condition->eval()))
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
	if (state->isBreakState())
	{
		return Result();
	}

	isBreak = false;

	do
	{
		state->eval();
	}while (!isBreak && !function->isReturned()&&parseCondition(condition->eval()));

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
	AStree *loop = nullptr;
	AStree *temp = block;

	while (temp)
	{
		if (temp->type == LoopBlock|| temp->type == SwitchBlock)
		{
			loop = temp;
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

Types * SwitchState::getType()
{
	return nullptr;
}

Result SwitchState::eval()
{
	Result v=target->eval();
	char *buffer = new char[v.type->getSize()];
	char *test = new char[v.type->getSize()];
	memcpy(buffer, (char *)v.value+v.offset, v.type->getSize());

	int startIndex = -1;
	int defaultIndex = -1;

	for (int i = 0; i < conditions.size(); ++i)
	{
		if (!conditions[i])
		{
			defaultIndex = i;
		}
		else
		{
			Result s = conditions[i]->eval();
			memcpy(buffer, (char *)s.value + s.offset, s.type->getSize());

			if (memcmp(buffer, test, s.type->getSize()) == 0)
			{
				startIndex = i;
				break;
			}
		}
	}

	if (startIndex == -1 && defaultIndex != -1)
	{
		startIndex = defaultIndex;
	}

	if (startIndex != -1)
	{
		for (int i = startIndex; i < conditions.size(); ++i)
		{
			for (int j = 0; j < states[i].size(); ++j)
			{
				states[i][j]->eval();
				if (isBreak || function->isReturned())
				{
					break;
				}
			}

			if (isBreak||function->isReturned())
			{
				break;
			}
		}
	}

	delete test;
	delete buffer;
	return Result();
}

bool SwitchState::isLeftValue()
{
	return false;
}

bool SwitchState::isSwitchState()
{
	return true;
}

Types * ForState::getType()
{
	return nullptr;
}

Result ForState::eval()
{
	if (state->isBreakState())
	{
		return Result();
	}

	for (ini->eval();parseCondition(con->eval());after->eval())
	{
		state->eval();
		if (isBreak || function->isReturned())
		{
			break;
		}
	}

	return Result();
}

bool ForState::isLeftValue()
{
	return false;
}

bool ForState::isForState()
{
	return true;
}

Types * VariableDefState::getType()
{
	return nullptr;
}

Result VariableDefState::eval()
{
	return Result();
}

bool VariableDefState::isLeftValue()
{
	return false;
}

bool VariableDefState::isVariableDefState()
{
	return true;
}
