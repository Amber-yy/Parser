#pragma once

#include <vector>
#include <memory>

class Types;
class Parser;
class FunctionDef;
class Block;
class ReturnState;
class IfState;
class WhileState;
class DoWhileState;
class BreakState;
class SwitchState;

enum BlockType
{
	StateBlock,
	SwitchBlock,
	LoopBlock
};

struct Result
{
	Types *type;
	void *value;
	int offset;
};

class AStree
{
public:
	AStree();
	~AStree();
	virtual Types*getType() = 0;
	virtual Result eval()=0;
	virtual bool isLeftValue() = 0;
	virtual bool isBlock();
	virtual bool isReturnState();
	virtual bool isIfState();
	virtual bool isWhileState();
	virtual bool isDoWhileState();
	virtual bool isBreakState();
	virtual bool isSwitchState();
	bool parseCondition(Result t);
	SwitchState *toSwitchState();
	BreakState *toBreakState();
	DoWhileState *toDoWhileState();
	WhileState *toWhileState();
	IfState *toIfState();
	ReturnState *toReturnState();
	Block *toBlock();
public:
	bool isBreak;
	FunctionDef *function;
	AStree *block;
public:
	static Parser *parser;
};

using AStreeRef = std::unique_ptr<AStree>;

class Block :public AStree
{
public:
	Block();
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isBlock()override;
	BlockType type;
	std::vector<AStreeRef> statements;
};

class ReturnState :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isReturnState()override;
	AStreeRef expr;
};

class IfState :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isIfState()override;
	AStreeRef condition;
	AStreeRef conTrue;
	AStreeRef conFalse;
};

class WhileState :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isWhileState()override;
	AStreeRef condition;
	AStreeRef state;
};

class DoWhileState :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isDoWhileState()override;
	AStreeRef condition;
	AStreeRef state;
};

class BreakState :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isBreakState()override;
};

class SwitchState :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isSwitchState()override;
	AStreeRef condition;
	std::vector<AStreeRef> state;
};