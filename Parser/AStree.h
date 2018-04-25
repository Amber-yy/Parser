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
class ForState;
class VariableDefState;
class IdExpr;
class NegativeExpr;
class EmptyState;
class PreIncExpr;
class PreDecExpr;
class GetValueExpr;
class GetAddrExpr;

enum BlockType
{
	StateBlock,
	SwitchBlock,
	LoopBlock
};

enum VariableRegion
{
	GlobalVariable,
	StaticVariable,
	Arg,
	Local
};

struct Result
{
	Result();
	Types *type;
	void *value;
	int offset;
};

struct EvalValue
{
	void release()
	{
		delete[]buffer;
	}
	char *buffer;
	Types *type;
};

class AStree
{
public:
	static EvalValue cast(Result t, Types *target);
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
	virtual bool isForState();
	virtual bool isVariableDefState();
	virtual bool isIdExpr();
	virtual bool isEmptyState();
	virtual bool isNegative();
	virtual bool isPreInc();
	virtual bool isPreDec();
	virtual bool isGetValue();
	virtual bool isGetAddr();
	bool parseCondition(Result t);
	GetAddrExpr *toGetAddr();
	GetValueExpr *toGetValue();
	PreDecExpr *toPreDec();
	PreIncExpr *toPreInc();
	NegativeExpr *toNegative();
	EmptyState *toEmptyState();
	IdExpr *toIdExpr();
	VariableDefState *toVariableDefState();
	ForState *toForState();
	SwitchState *toSwitchState();
	BreakState *toBreakState();
	DoWhileState *toDoWhileState();
	WhileState *toWhileState();
	IfState *toIfState();
	ReturnState *toReturnState();
	Block *toBlock();
public:
	bool isBreak;
	BlockType type;
	FunctionDef *function;
	AStree *block;
public:
	static Parser *parser;
};

using AStreeRef = std::unique_ptr<AStree>;

struct IniList;
using IniRef= std::unique_ptr<IniList>;

struct IniList
{
	EvalValue eval();
	AStreeRef expr;
	std::vector<IniRef> nexts;
	std::vector<int> offset;
	Types *type;
};

class Block :public AStree
{
public:
	Block();
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isBlock()override;
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
	AStreeRef target;
	std::vector<AStreeRef> conditions;
	std::vector<std::vector<AStreeRef>> states;
};

class ForState :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isForState()override;
	AStreeRef ini;
	AStreeRef con;
	AStreeRef after;
	AStreeRef state;
};

class VariableDefState :public AStree
{
public:
	VariableDefState();
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isVariableDefState()override;
	AStreeRef id;
	IniRef value;
	bool isInied;
};

class IdExpr :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isIdExpr()override;
	Types *type;
	VariableRegion reg;
	int offset;
};

class EmptyState :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isEmptyState()override;
};

class NegativeExpr :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isNegative()override;
	AStreeRef target;
};

class PreIncExpr :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isPreInc()override;
	AStreeRef target;
};

class PreDecExpr :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isPreDec()override;
	AStreeRef target;
};

class GetValueExpr :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isGetValue()override;
	AStreeRef target;
};

class GetAddrExpr :public AStree
{
public:
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isGetAddr()override;
	Types *type;
	AStreeRef target;
};

class StringLiteral :public AStree
{

};