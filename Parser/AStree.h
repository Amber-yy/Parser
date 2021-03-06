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
class BitNegativeExpr;
class TypeTranExpr;
class SizeOfExpr;
class IntLiteralExpr;
class RealLiteralExpr;
class StringLiteralExpr;
class PostIncExpr;
class PostDecExpr;
class ArrayAccessExpr;
class MemberAccessExpr;
class MemberAccessPtr;
class FuncallExpr;
class ConditionExpr;
class AddExpr;
class SubExpr;
class MulExpr;
class DivExpr;
class AssignExpr;
class LessExpr;

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
		buffer = nullptr;
	}
	char *buffer;
	Types *type;
};

class AStree;
using AStreeRef = std::unique_ptr<AStree>;

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
	virtual AStreeRef copy(FunctionDef *fun,AStree *block)=0;
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
	virtual bool isBitNegative();
	virtual bool isTypeTran();
	virtual bool isSizeOf();
	virtual bool isIntLiteral();
	virtual bool isRealLiteral();
	virtual bool isStringLiteral();
	virtual bool isPostInc();
	virtual bool isPostDec();
	virtual bool isArrayAccess();
	virtual bool isMemberAccess();
	virtual bool isMemberAccessPtr();
	virtual bool isFuncall();
	virtual bool isCondition();
	virtual bool isAdd();
	virtual bool isSub();
	virtual bool isLess();
	virtual bool isMul();
	virtual bool isDiv();
	virtual bool isAssign();
	bool parseCondition(Result t);
	AssignExpr *toAssign();
	DivExpr *toDiv();
	MulExpr *toMul();
	SubExpr *toSub();
	LessExpr *toLess();
	AddExpr *toAdd();
	ConditionExpr *toCondition();
	FuncallExpr *toFuncall();
	MemberAccessPtr *toMemberAccessPtr();
	MemberAccessExpr *toMemberAccess();
	ArrayAccessExpr *toArrayAccess();
	PostDecExpr *toPostDec();
	PostIncExpr *toPostInc();
	StringLiteralExpr *toStringLiteral();
	RealLiteralExpr *toRealLiteral();
	IntLiteralExpr *toIntLiteral();
	SizeOfExpr *toSizeOf();
	TypeTranExpr *toTypeTran();
	BitNegativeExpr *toBitNegative();
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

struct IniList;
using IniRef= std::unique_ptr<IniList>;

struct IniList
{
	EvalValue eval();
	AStreeRef expr;
	IniRef copy(FunctionDef *fun,AStree *block);
	std::vector<IniRef> nexts;
	std::vector<int> offset;
	Types *type;
};

class Block :public AStree
{ 
public:
	Block();
	virtual AStreeRef copy(FunctionDef *fun, AStree *block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isBlock()override;
	std::vector<AStreeRef> statements;
};

class ReturnState :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isReturnState()override;
	AStreeRef expr;
};

class IfState :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
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
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
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
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
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
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isBreakState()override;
};

class SwitchState :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
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
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
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
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	VariableDefState();
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isVariableDefState()override;
	std::vector<AStreeRef> id;
	std::vector<IniRef> value;
	bool isInied;
};

class IdExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isIdExpr()override;
	Types *thisType;
	VariableRegion reg;
	int offset;
};

class EmptyState :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isEmptyState()override;
};

class NegativeExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isNegative()override;
	AStreeRef target;
};

class PreIncExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isPreInc()override;
	AStreeRef target;
};

class PreDecExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isPreDec()override;
	AStreeRef target;
};

class GetValueExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isGetValue()override;
	AStreeRef target;
};

class GetAddrExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isGetAddr()override;
	Types *thistype;
	AStreeRef target;
};

class BitNegativeExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isNegative()override;
	AStreeRef target;
};

class TypeTranExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isTypeTran()override;
	Types *targetType;
	AStreeRef target;
};

class SizeOfExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isTypeTran()override;
	Types *thisType;
	int size;
};

class IntLiteralExpr :public AStree
{
public:
	static Types *thisType;
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isIntLiteral()override;
	long long value;
};

class RealLiteralExpr :public AStree
{
public:
	static Types *thisType;
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isRealLiteral()override;
	double value;
};

class StringLiteralExpr :public AStree
{
public:
	static Types *thisType;
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isStringLiteral()override;
	char* value;
};

class PostIncExpr : public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isPostInc()override;
	AStreeRef target;
};

class PostDecExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isPostDec()override;
	AStreeRef target;
};

class ArrayAccessExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isArrayAccess()override;
	AStreeRef index;
	AStreeRef addr;
};

class MemberAccessExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isMemberAccess()override;
	int offset;
	Types*thisType;
	AStreeRef target;
};

class MemberAccessPtr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isMemberAccessPtr()override;
	int offset;
	Types*thisType;
	AStreeRef target;
};

class FuncallExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isFuncall()override;
	std::vector<AStreeRef> args;
	Types*thisType;
	AStreeRef target;
};

class ConditionExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isCondition()override;
	AStreeRef con;
	AStreeRef conTrue;
	AStreeRef conFalse;
};

class AddExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isAdd()override;
	Types *thisType;
	AStreeRef left;
	AStreeRef right;
};

class SubExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isSub()override;
	Types *thisType;
	AStreeRef left;
	AStreeRef right;
};

class MulExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isMul()override;
	Types *thisType;
	AStreeRef left;
	AStreeRef right;
};

class DivExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isDiv()override;
	Types *thisType;
	AStreeRef left;
	AStreeRef right;
};

class AssignExpr :public AStree
{
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isAssign()override;
	Types *thisType;
	AStreeRef left;
	AStreeRef right;
};

class LessExpr :public AStree
{
public:
	static Types *thisType;
public:
	virtual AStreeRef copy(FunctionDef *fun, AStree*block)override;
	virtual Types*getType()override;
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isLess()override;
	AStreeRef left;
	AStreeRef right;
};

