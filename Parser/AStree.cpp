#include "AStree.h"
#include "Parser.h"
#include "FunctionDef.h"
#include "Types.h"

#include <iostream>

Types *IntLiteralExpr::thisType;
Types *RealLiteralExpr::thisType;
Types *StringLiteralExpr::thisType;
Types *LessExpr::thisType;

char getChar(Result t)
{
	return *((char*)(t.value) + t.offset);
}

unsigned char getUchar(Result t)
{
	return *((unsigned char*)(t.value) + t.offset);
}

short getShort(Result t)
{
	return *(short *)((char*)(t.value) + t.offset);
}

unsigned short getUShort(Result t)
{
	return *(unsigned short *)((char*)(t.value) + t.offset);
}

int getInt(Result t)
{
	return *(int *)((char*)(t.value) + t.offset);
}

unsigned int getUInt(Result t)
{
	return *(unsigned int *)((char*)(t.value) + t.offset);
}

long long getLong(Result t)
{
	return *(long long *)((char*)(t.value) + t.offset);
}

long long getULong(Result t)
{
	return *(unsigned long long *)((char*)(t.value) + t.offset);
}

float getFloat(Result t)
{
	return *(float *)((char*)(t.value) + t.offset);
}

double getDouble(Result t)
{
	return *(double *)((char*)(t.value) + t.offset);
}

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

bool AStree::isIdExpr()
{
	return false;
}

bool AStree::isEmptyState()
{
	return false;
}

bool AStree::isNegative()
{
	return false;
}

bool AStree::isPreInc()
{
	return false;
}

bool AStree::isPreDec()
{
	return false;
}

bool AStree::isGetValue()
{
	return false;
}

bool AStree::isGetAddr()
{
	return false;
}

bool AStree::isBitNegative()
{
	return false;
}

bool AStree::isTypeTran()
{
	return false;
}

bool AStree::isSizeOf()
{
	return false;
}

bool AStree::isIntLiteral()
{
	return false;
}

bool AStree::isRealLiteral()
{
	return false;
}

bool AStree::isStringLiteral()
{
	return false;
}

bool AStree::isPostInc()
{
	return false;
}

bool AStree::isPostDec()
{
	return false;
}

bool AStree::isArrayAccess()
{
	return false;
}

bool AStree::isMemberAccess()
{
	return false;
}

bool AStree::isMemberAccessPtr()
{
	return false;
}

bool AStree::isFuncall()
{
	return false;
}

bool AStree::isCondition()
{
	return false;
}

bool AStree::isAdd()
{
	return false;
}

bool AStree::isSub()
{
	return false;
}

bool AStree::isLess()
{
	return false;
}

bool AStree::isMul()
{
	return false;
}

bool AStree::isDiv()
{
	return false;
}

bool AStree::isAssign()
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

AssignExpr * AStree::toAssign()
{
	return dynamic_cast<AssignExpr *>(this);
}

DivExpr * AStree::toDiv()
{
	return dynamic_cast<DivExpr *>(this);
}

MulExpr * AStree::toMul()
{
	return dynamic_cast<MulExpr *>(this);
}

SubExpr * AStree::toSub()
{
	return dynamic_cast<SubExpr *>(this);
}

LessExpr * AStree::toLess()
{
	return dynamic_cast<LessExpr *>(this);
}

AddExpr * AStree::toAdd()
{
	return dynamic_cast<AddExpr *>(this);
}

ConditionExpr * AStree::toCondition()
{
	return dynamic_cast<ConditionExpr *>(this);
}

FuncallExpr * AStree::toFuncall()
{
	return dynamic_cast<FuncallExpr *>(this);
}

MemberAccessPtr * AStree::toMemberAccessPtr()
{
	return dynamic_cast<MemberAccessPtr *>(this);
}

MemberAccessExpr * AStree::toMemberAccess()
{
	return dynamic_cast<MemberAccessExpr *>(this);
}

ArrayAccessExpr * AStree::toArrayAccess()
{
	return dynamic_cast<ArrayAccessExpr *>(this);
}

PostDecExpr * AStree::toPostDec()
{
	return dynamic_cast<PostDecExpr *>(this);
}

PostIncExpr * AStree::toPostInc()
{
	return dynamic_cast<PostIncExpr *>(this);
}

StringLiteralExpr * AStree::toStringLiteral()
{
	return dynamic_cast<StringLiteralExpr *>(this);
}

RealLiteralExpr * AStree::toRealLiteral()
{
	return dynamic_cast<RealLiteralExpr *>(this);
}

IntLiteralExpr * AStree::toIntLiteral()
{
	return dynamic_cast<IntLiteralExpr *>(this);
}

SizeOfExpr * AStree::toSizeOf()
{
	return dynamic_cast<SizeOfExpr *>(this);
}

TypeTranExpr * AStree::toTypeTran()
{
	return dynamic_cast<TypeTranExpr *>(this);
}

BitNegativeExpr * AStree::toBitNegative()
{
	return dynamic_cast<BitNegativeExpr *>(this);
}

GetAddrExpr * AStree::toGetAddr()
{
	return dynamic_cast<GetAddrExpr *>(this);
}

GetValueExpr * AStree::toGetValue()
{
	return dynamic_cast<GetValueExpr *>(this);
}

PreDecExpr * AStree::toPreDec()
{
	return dynamic_cast<PreDecExpr *>(this);
}

PreIncExpr * AStree::toPreInc()
{
	return dynamic_cast<PreIncExpr *>(this);
}

NegativeExpr * AStree::toNegative()
{
	return dynamic_cast<NegativeExpr *>(this);
}

EmptyState * AStree::toEmptyState()
{
	return dynamic_cast<EmptyState *>(this);
}

IdExpr * AStree::toIdExpr()
{
	return dynamic_cast<IdExpr *>(this);
}

template<class T>
void doCast(EvalValue v,Result t,Types *tp)
{
	if (!tp->isBasic())
	{
		*(T *)v.buffer = (T)getInt(t);
		return;
	}

	BasicType *bs = tp->toBasic();

	if (bs->isFloat)
	{
		if (bs->size == 4)
		{
			*(T *)v.buffer = (T)getFloat(t);
		}
		else
		{
			*(T *)v.buffer = (T)getDouble(t);
		}
	}
	else
	{
		if (bs->size == 1)
		{
			if (bs->isSigned)
			{
				*(T *)v.buffer = (T)getChar(t);
			}
			else
			{
				*(T *)v.buffer = (T)getUchar(t);
			}
		}
		else if (bs->size == 2)
		{
			if (bs->isSigned)
			{
				*(T *)v.buffer = (T)getShort(t);
			}
			else
			{
				*(T *)v.buffer = (T)getUShort(t);
			}
		}
		else if (bs->size == 4)
		{
			if (bs->isSigned)
			{
				*(T *)v.buffer = (T)getInt(t);
			}
			else
			{
				*(T *)v.buffer = (T)getUInt(t);
			}
		}
		else if (bs->size == 8)
		{
			if (bs->isSigned)
			{
				*(T *)v.buffer = (T)getLong(t);
			}
			else
			{
				*(T *)v.buffer = (T)getULong(t);
			}
		}
	}
}

EvalValue AStree::cast(Result t, Types * target)
{
	EvalValue v;
	v.buffer = new char[target->getSize()];
	v.type = target;
	
	void *pos = (char *)t.value + t.offset;

	if (target->isBasic())
	{
		Types *bs=t.type;
		BasicType *bt = target->toBasic();

		if (bt->isFloat)
		{
			if (bt->size == 4)
			{
				doCast<float>(v,t,bs);
			}
			else
			{
				doCast<double>(v, t, bs);
			}
		}
		else
		{
			if (bt->size == 1)
			{
				if (bt->isSigned)
				{
					doCast<char>(v, t, bs);
				}
				else
				{
					doCast<unsigned char>(v, t, bs);
				}
			}
			else if (bt->size == 2)
			{
				if (bt->isSigned)
				{
					doCast<short>(v, t, bs);
				}
				else
				{
					doCast<unsigned short>(v, t, bs);
				}
			}
			else if (bt->size == 4)
			{
				if (bt->isSigned)
				{
					doCast<int>(v, t, bs);
				}
				else
				{
					doCast<unsigned int>(v, t, bs);
				}
			}
			else if (bt->size == 8)
			{
				if (bt->isSigned)
				{
					doCast<long long>(v, t, bs);
				}
				else
				{
					doCast<unsigned long long>(v, t, bs);
				}
			}
		}
	}
	else if (target->isEnum())
	{
		if (v.type->isEnum())
		{
			memcpy(v.buffer, pos, v.type->getSize());
		}
		else
		{
			int size;
			BasicType *basic = v.type->toBasic();

			if (basic->getSize() == 1)
			{
				if (basic->isSigned)
				{
					size = getChar(t);
				}
				else
				{
					size = getUchar(t);
				}
			}
			else if (v.type->getSize() == 2)
			{
				if (basic->isSigned)
				{
					size = getShort(t);
				}
				else
				{
					size = getUShort(t);
				}
			}
			else if (v.type->getSize() == 4)
			{
				if (basic->isSigned)
				{
					size = getInt(t);
				}
				else
				{
					size = getUInt(t);
				}
			}
			else if (v.type->getSize() == 8)
			{
				if (basic->isSigned)
				{
					size = getLong(t);
				}
				else
				{
					size = getULong(t);
				}
			}

			memcpy(v.buffer, &size, v.type->getSize());
		}


	}
	else if (target->isPointer())
	{
		if (v.type->isPointer())
		{
			memcpy(v.buffer,pos,v.type->getSize());
		}
		else
		{
			int size;
			BasicType *basic = v.type->toBasic();

			if (basic->getSize() == 1)
			{
				if (basic->isSigned)
				{
					size = getChar(t);
				}
				else
				{
					size = getUchar(t);
				}
			}
			else if (v.type->getSize() == 2)
			{
				if (basic->isSigned)
				{
					size = getShort(t);
				}
				else
				{
					size = getUShort(t);
				}
			}
			else if (v.type->getSize() == 4)
			{
				if (basic->isSigned)
				{
					size = getInt(t);
				}
				else
				{
					size = getUInt(t);
				}
			}
			else if (v.type->getSize() == 8)
			{
				if (basic->isSigned)
				{
					size = getLong(t);
				}
				else
				{
					size = getULong(t);
				}
			}

			memcpy(v.buffer,&size,v.type->getSize());
		}
	}
	else
	{
		memcpy(v.buffer,pos, target->getSize());
	}

	return v;
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

AStreeRef Block::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<Block>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *blo = t->toBlock();
	for (int i = 0; i < statements.size(); ++i)
	{
		blo->statements.push_back(statements[i]->copy(fun,blo));
	}

	return t;
}

Types * Block::getType()
{
	return nullptr;
}

Result Block::eval()
{
	AStree *loop=nullptr;
	AStree *temp = block;

	int off = function->getOffset();

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

	function->setOffset(off);

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

AStreeRef ReturnState::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<ReturnState>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *blo = t->toReturnState();
	blo->expr = expr->copy(fun,block);

	return t;
}

Types * ReturnState::getType()
{
	return function->getType()->toFunction()->returnType;
}

Result ReturnState::eval()
{
	if (expr.get() != nullptr)
	{
		Result t=expr->eval();
		char *s = (char *)t.value + t.offset;
		int *ttt = (int *)s;
		function->setReturned();
		auto vv = cast(t, getType());
		function->setReturnValue(vv.buffer);
		vv.release();
		t.value = function->getReturnValue();
		t.type = getType();
		t.offset =0;
		return t;
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

AStreeRef IfState::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<IfState>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *ifs = t->toIfState();
	ifs->condition = condition->copy(fun,block);
	ifs->conTrue = conTrue->copy(fun, block);
	if (ifs->conFalse.get())
	{
		ifs->conFalse = conFalse->copy(fun, block);
	}

	return t;
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

AStreeRef WhileState::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<WhileState>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *wls = t->toWhileState();
	wls->condition = condition->copy(fun, block);
	wls->state = state->copy(fun, block);

	return t;
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

AStreeRef DoWhileState::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<DoWhileState>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *wls = t->toDoWhileState();
	wls->condition = condition->copy(fun, block);
	wls->state = state->copy(fun, block);

	return t;
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

AStreeRef BreakState::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<BreakState>();
	t->block = block;
	t->function = fun;
	t->type = type;
	return t;
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

	return Result();
}

bool BreakState::isLeftValue()
{
	return false;
}

bool BreakState::isBreakState()
{
	return true;
}

AStreeRef SwitchState::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<SwitchState>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *sws = t->toSwitchState();
	sws->target = target->copy(fun,block);
	for (int i = 0; i < conditions.size(); ++i)
	{
		if (conditions[i] == nullptr)
		{
			sws->conditions.push_back(nullptr);
		}
		else
		{
			sws->conditions.push_back(conditions[i]->copy(fun,sws));
		}
		for (int j = 0; j < sws->states[i].size(); ++j)
		{
			sws->states[i].push_back(states[i][j]->copy(fun, sws));
		}
	}

	return t;
}

Types * SwitchState::getType()
{
	return nullptr;
}

Result SwitchState::eval()
{
	int off = function->getOffset();

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
			EvalValue vv = cast(s, v.type);
			memcpy(test,vv.buffer, v.type->getSize());
			if (memcmp(buffer, test, s.type->getSize()) == 0)
			{
				startIndex = i;
				break;
			}

			vv.release();
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

	function->setOffset(off);
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

AStreeRef ForState::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<ForState>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *sws = t->toForState();
	sws->ini = ini->copy(fun,block);
	sws->con=con->copy(fun, block);
	sws->after=after->copy(fun, block);
	sws->state=state->copy(fun, block);

	return t;
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

	if (ini.get() != nullptr)
	{
		ini->eval();
	}

	isBreak = false;

	for (;;)
	{
		state->eval();
		if (isBreak || function->isReturned())
		{
			break;
		}
		if (after.get() != nullptr)
		{
			after->eval();
		}
		if (con.get() != nullptr && !parseCondition(con->eval()))
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

AStreeRef VariableDefState::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<VariableDefState>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *vdf = t->toVariableDefState();
	vdf->isInied = isInied;
	
	for (int i = 0; i < id.size(); ++i)
	{
		vdf->id.push_back(id[i]->copy(fun, block));
		vdf->value.push_back(value[i]->copy(fun,block));
	}

	return t;
}

VariableDefState::VariableDefState()
{
	isInied = false;
}

Types * VariableDefState::getType()
{
	return nullptr;
}

Result VariableDefState::eval()
{

	for (int i = 0; i < id.size(); ++i)
	{
		int off = function->getOffset();

		if (isInied&&id[i]->getType()->isStatic)
		{
			return Result();
		}

		EvalValue v = value[i]->eval();
		Result t = id[i]->eval();
		memcpy((char *)t.value + t.offset, v.buffer, t.type->getSize());
		v.release();

		function->setOffset(off + id[i]->getType()->getSize());
	}

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

EvalValue IniList::eval()
{	
	if (nexts.size() == 0)
	{
		if (expr.get() != nullptr)
		{
			Result t = expr->eval();
			return AStree::cast(t, type);
		}
		EvalValue v;
		v.type = type;
		v.buffer = new char[type->getSize()];
		memset(v.buffer, 0, type->getSize());
		return v;
	}

	EvalValue v;
	v.type = type;
	v.buffer = new char[type->getSize()];
	memset(v.buffer, 0, type->getSize());

	for (int i = 0; i < nexts.size(); ++i)
	{
		EvalValue t = nexts[i]->eval();
		memcpy(v.buffer + offset[i], t.buffer,nexts[i]->type->getSize());
		t.release();
	}

	return v;
}

IniRef IniList::copy(FunctionDef *fun,AStree *block)
{
	IniRef ir = std::make_unique<IniList>();
	ir->type = type;

	if (expr.get())
	{
		ir->expr = expr->copy(fun, block);
		return ir;
	}

	for (int i = 0; i < offset.size(); ++i)
	{
		ir->offset.push_back(offset[i]);
		ir->nexts.push_back(nexts[i]->copy(fun,block));
	}

	return ir;
}

AStreeRef IdExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<IdExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *ide = t->toIdExpr();
	ide->thisType = thisType;
	ide->reg = reg;
	ide->offset = offset;

	return t;
}

Types * IdExpr::getType()
{
	return thisType;
}

Result IdExpr::eval()
{
	Result t;
	t.type = thisType;
	t.offset = offset;
	if (reg == GlobalVariable)
	{
		t.value = parser->getGlobalRegion();
	}
	else if (reg == StaticVariable)
	{
		t.value = parser->getStaticRegion();
	}
	else if(reg==Arg)
	{
		t.value = function->getStack();
	}
	else
	{
		t.value = function->getLocal();
	}

	/*
	对数组求值不返回值，返回地址，但对于参数中的数组来说，它的值本身就是地址（参数中的数组视为指针）
	*/
	if (reg!=Arg&&thisType->isArray())
	{
		int off = function->getOffset();
		char *local = (char *)function->getLocal()+off;//指针指向栈空间
		char *add=(char *)t.value + t.offset;//这个地方是数组首元素的地址，也应该是数组的地址
		*(char **)local = add;//把栈空间强转为二级指针，并赋值为数组地址
		t.value = local;
		t.offset = 0;
	}

	return t;
}

bool IdExpr::isLeftValue()
{
	//if (type->isArray() || type->isFunction())
	//{
	//	return false;
	//}

	return true;
}

bool IdExpr::isIdExpr()
{
	return true;
}

AStreeRef EmptyState::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<EmptyState>();
	t->block = block;
	t->function = fun;
	t->type = type;

	return t;
}

Types * EmptyState::getType()
{
	return nullptr;
}

Result EmptyState::eval()
{
	return Result();
}

bool EmptyState::isLeftValue()
{
	return false;
}

bool EmptyState::isEmptyState()
{
	return true;
}

Result::Result()
{
	offset = 0;
	value = nullptr;
	type = nullptr;
}

AStreeRef NegativeExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<NegativeExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toNegative();
	neg->target = target->copy(fun, block);

	return t;
}

Types * NegativeExpr::getType()
{
	return target->getType();
}

template<class T>
void getNegative(void *a,void *b)
{
	*(T *)a=-*(T *)b;
}

Result NegativeExpr::eval()
{
	Result t;
	int off = function->getOffset();
	t.type = target->getType();
	t.value = (char *)function->getLocal()+off;
	t.offset = 0;
	Result r=target->eval();

	char *x = (char *)r.value;
	x += r.offset;
	r.value = x;

	BasicType *basic = t.type->toBasic();

	if (basic->isFloat)
	{
		if (basic->size == 4)
		{
			getNegative<float>(t.value, r.value);
		}
		else
		{
			getNegative<double>(t.value, r.value);
		}
	}
	else
	{
		if (basic->size == 1)
		{
			if (basic->isSigned)
			{
				getNegative<char>(t.value, r.value);
			}
			else
			{
				getNegative<unsigned char>(t.value, r.value);
			}
		}
		else if (basic->size == 2)
		{
			if (basic->isSigned)
			{
				getNegative<short>(t.value, r.value);
			}
			else
			{
				getNegative<unsigned short>(t.value, r.value);
			}
		}
		else if (basic->size == 4)
		{
			if (basic->isSigned)
			{
				getNegative<int>(t.value, r.value);
			}
			else
			{
				getNegative<unsigned int>(t.value, r.value);
			}
		}
		else if (basic->size == 8)
		{
			if (basic->isSigned)
			{
				getNegative<long long>(t.value, r.value);
			}
			else
			{
				getNegative<unsigned long long>(t.value, r.value);
			}
		}
	}

	return t;
}

bool NegativeExpr::isLeftValue()
{
	return target->isLeftValue();
}

bool NegativeExpr::isNegative()
{
	return true;
}

AStreeRef PreIncExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<PreIncExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toPreInc();
	neg->target = target->copy(fun, block);

	return t;
}

Types * PreIncExpr::getType()
{
	return target->getType();
}

template<class T>
void Inc(void *a)
{
	*(T *)a += 1;
}

Result PreIncExpr::eval()
{
	Result r = target->eval();
	char *x = (char *)r.value;
	x += r.offset;
	r.value = x;
	r.offset = 0;

	if (r.type->isPointer())
	{
		*(int *)r.value += r.type->getSize();
		return r;
	}

	BasicType *basic = r.type->toBasic();

	if (basic->isFloat)
	{
		if (basic->size == 4)
		{
			Inc<float>(r.value);
		}
		else
		{
			Inc<double>(r.value);
		}
	}
	else
	{
		if (basic->size == 1)
		{
			if (basic->isSigned)
			{
				Inc<char>(r.value);
			}
			else
			{
				Inc<unsigned char>(r.value);
			}
		}
		else if (basic->size == 2)
		{
			if (basic->isSigned)
			{
				Inc<short>(r.value);
			}
			else
			{
				Inc<unsigned short>(r.value);
			}
		}
		else if (basic->size == 4)
		{
			if (basic->isSigned)
			{
				Inc<int>(r.value);
			}
			else
			{
				Inc<unsigned int>(r.value);
			}
		}
		else if (basic->size == 8)
		{
			if (basic->isSigned)
			{
				Inc<long long>(r.value);
			}
			else
			{
				Inc<unsigned long long>(r.value);
			}
		}
	}

	return r;
}

bool PreIncExpr::isLeftValue()
{
	return true;
}

bool PreIncExpr::isPreInc()
{
	return true;
}

AStreeRef PreDecExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<PreDecExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toPreDec();
	neg->target = target->copy(fun, block);

	return t;
}

Types * PreDecExpr::getType()
{
	return target->getType();
}

template<class T>
void Dec(void *a)
{
	*(T *)a -= 1;
}

Result PreDecExpr::eval()
{
	Result r = target->eval();
	char *x = (char *)r.value;
	x += r.offset;
	r.value = x;
	r.offset = 0;

	if (r.type->isPointer())
	{
		*(int *)r.value -= r.type->getSize();
		return r;
	}

	BasicType *basic = r.type->toBasic();

	if (basic->isFloat)
	{
		if (basic->size == 4)
		{
			Dec<float>(r.value);
		}
		else
		{
			Dec<double>(r.value);
		}
	}
	else
	{
		if (basic->size == 1)
		{
			if (basic->isSigned)
			{
				Dec<char>(r.value);
			}
			else
			{
				Dec<unsigned char>(r.value);
			}
		}
		else if (basic->size == 2)
		{
			if (basic->isSigned)
			{
				Dec<short>(r.value);
			}
			else
			{
				Dec<unsigned short>(r.value);
			}
		}
		else if (basic->size == 4)
		{
			if (basic->isSigned)
			{
				Dec<int>(r.value);
			}
			else
			{
				Dec<unsigned int>(r.value);
			}
		}
		else if (basic->size == 8)
		{
			if (basic->isSigned)
			{
				Dec<long long>(r.value);
			}
			else
			{
				Dec<unsigned long long>(r.value);
			}
		}
	}

	return r;
}

bool PreDecExpr::isLeftValue()
{
	return true;
}

bool PreDecExpr::isPreDec()
{
	return true;
}

AStreeRef GetValueExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<GetValueExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toGetValue();
	neg->target = target->copy(fun, block);

	return t;
}

Types * GetValueExpr::getType()
{
	if (target->getType()->isPointer())
	{
		return target->getType()->toPointer()->targetType;
	}

	return target->getType()->toArray()->dataType;
}

Result GetValueExpr::eval()
{
	Result t = target->eval();
	Result r;

	if (target->getType()->isPointer())
	{
		r.type = target->getType()->toPointer()->targetType;
	}
	else
	{
		r.type = target->getType()->toArray()->dataType;
	}

	char *s = (char *)t.value + t.offset;//指针本身的地址，是一个二级指针
	r.value = *(char **)s;//解除引用二级指针，得到指针的值
	r.offset = 0;//没有偏移

	if (r.type->isArray())//如果解除引用的目标是数组，还需要进一步处理，使返回的值是地址
	{
		int off = function->getOffset();
		char *local = (char *)function->getLocal() + off;//指针指向栈空间
		char *add = (char *)r.value + r.offset;//这个地方是数组首元素的地址，也应该是数组的地址
		*(char **)local = add;//把栈空间强转为二级指针，并赋值为数组地址
		r.value = local;
		r.offset = 0;
	}

	return r;
}

bool GetValueExpr::isLeftValue()
{
	//if (target->getType()->isPointer())
	//{
	//	if (target->getType()->toPointer()->targetType->isArray())
	//	{
	//		return false;
	//	}
	//	if (target->getType()->toPointer()->targetType->isConst)
	//	{
	//		return false;
	//	}
	//}
	//else
	//{
	//	if (target->getType()->toArray()->dataType->isConst)
	//	{
	//		return false;
	//	}
	//	if (target->getType()->toArray()->dataType->isArray())
	//	{
	//		return false;
	//	}
	//}
	
	return true;
}

bool GetValueExpr::isGetValue()
{
	return true;
}

AStreeRef GetAddrExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<GetAddrExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toGetAddr();
	neg->target = target->copy(fun, block);
	neg->thistype = thistype;

	return t;
}

Types * GetAddrExpr::getType()
{
	return thistype;
}

Result GetAddrExpr::eval()
{
	Result t=target->eval();

	int off = function->getOffset();
	char *local = (char *)function->getLocal() + off;//指针指向栈空间
	char *add;
	if (target->getType()->isArray())
	{
		add = (char *)t.value + t.offset;//数组的求值得到一个地址，地址存放于此地址
		char *t = *(char **)add;
		add = t;
	}
	else
	{
		add = (char *)t.value + t.offset;//这个地方是目标的地址
	}
	*(char **)local = add;//把栈空间强转为二级指针，并赋值为数组地址
	t.value = local;
	t.offset = 0;

	t.type = thistype;

	return t;
}

bool GetAddrExpr::isLeftValue()
{
	return false;
}

bool GetAddrExpr::isGetAddr()
{
	return true;
}

AStreeRef BitNegativeExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<BitNegativeExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toBitNegative();
	neg->target = target->copy(fun, block);

	return t;
}

Types * BitNegativeExpr::getType()
{
	return target->getType();
}

template<class T>
void doBitNeg(Result t,void *local)
{
	char *temp=(char *)t.value+t.offset;
	T n = *(T *)temp;
	n = ~n;
	memcpy(local, &n, sizeof(T));
}

Result BitNegativeExpr::eval()
{
	Result t = target->eval();
	
	Result r;
	r.type = t.type;
	r.value = (char *)function->getLocal() + function->getOffset();
	r.offset = 0;

	BasicType *basic = t.type->toBasic();

	if (basic->size == 1)
	{
		if (basic->isSigned)
		{
			doBitNeg<char>(t,r.value);
		}
		else
		{
			doBitNeg<unsigned char>(t,r.value);
		}
	}
	else if (basic->size == 2)
	{
		if (basic->isSigned)
		{
			doBitNeg<short>(t, r.value);
		}
		else
		{
			doBitNeg<unsigned short>(t, r.value);
		}
	}
	else if (basic->size == 4)
	{
		if (basic->isSigned)
		{
			doBitNeg<int>(t, r.value);
		}
		else
		{
			doBitNeg<unsigned int>(t, r.value);
		}
	}
	else if (basic->size == 8)
	{
		if (basic->isSigned)
		{
			doBitNeg<long long>(t, r.value);
		}
		else
		{
			doBitNeg<unsigned long long>(t, r.value);
		}
	}

	return r;
}

bool BitNegativeExpr::isLeftValue()
{
	return false;
}

bool BitNegativeExpr::isNegative()
{
	return true;
}

AStreeRef TypeTranExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<TypeTranExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toTypeTran();
	neg->target = target->copy(fun, block);
	neg->targetType = targetType;

	return t;
}

Types * TypeTranExpr::getType()
{
	return targetType;
}

Result TypeTranExpr::eval()
{
	Result t=target->eval();
	auto vv=cast(t, targetType);
	char *buffer= (char *)function->getLocal() + function->getOffset();
	t.value = buffer;
	t.offset = 0;
	memcpy(buffer, vv.buffer, targetType->getSize());
	t.type = targetType;
	vv.release();
	return t;
}

bool TypeTranExpr::isLeftValue()
{
	return false;
}

bool TypeTranExpr::isTypeTran()
{
	return true;
}

AStreeRef SizeOfExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<SizeOfExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toSizeOf();
	neg->thisType = thisType;
	neg->size = size;

	return t;
}

Types * SizeOfExpr::getType()
{
	return thisType;
}

Result SizeOfExpr::eval()
{
	Result r;
	r.value = (char *)function->getLocal() + function->getOffset();
	r.offset = 0;
	*(int *)r.value =size;
	r.type = thisType;
	return r;
}

bool SizeOfExpr::isLeftValue()
{
	return false;
}

bool SizeOfExpr::isTypeTran()
{
	return true;
}

AStreeRef IntLiteralExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<IntLiteralExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toIntLiteral();
	neg->value = value;

	return t;
}

Types * IntLiteralExpr::getType()
{
	return thisType;
}

Result IntLiteralExpr::eval()
{
	Result r;
	r.type = thisType;
	r.value = (char *)function->getLocal() + function->getOffset();
	r.offset = 0;
	*(long long *)r.value = value;
	return r;
}

bool IntLiteralExpr::isLeftValue()
{
	return false;
}

bool IntLiteralExpr::isIntLiteral()
{
	return true;
}

AStreeRef RealLiteralExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<RealLiteralExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toRealLiteral();
	neg->value = value;

	return t;
}

Types * RealLiteralExpr::getType()
{
	return thisType;
}

Result RealLiteralExpr::eval()
{
	Result r;
	r.type = thisType;
	r.value = (char *)function->getLocal() + function->getOffset();
	r.offset = 0;
	*(double *)r.value = value;
	return r;
}

bool RealLiteralExpr::isLeftValue()
{
	return false;
}

bool RealLiteralExpr::isRealLiteral()
{
	return true;
}

AStreeRef StringLiteralExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<StringLiteralExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toStringLiteral();
	neg->value = value;

	return t;
}

Types * StringLiteralExpr::getType()
{
	return thisType;
}

Result StringLiteralExpr::eval()
{
	Result r;
	r.type = thisType;
	r.value = (char *)function->getLocal() + function->getOffset();
	r.offset = 0;
	*(char **)r.value = value;
	return r;
}

bool StringLiteralExpr::isLeftValue()
{
	return false;
}

bool StringLiteralExpr::isStringLiteral()
{
	return true;
}

AStreeRef PostIncExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<PostIncExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toPreInc();
	neg->target = target->copy(fun,block);

	return t;
}

Types * PostIncExpr::getType()
{
	return target->getType();
}

Result PostIncExpr::eval()
{
	Result r = target->eval();
	char *x = (char *)r.value;
	x += r.offset;
	r.value = x;
	r.offset = 0;

	if (r.type->isPointer())
	{
		int *p = *(int **)r.value;
		*(int *)r.value += r.type->getSize();
		r.value = (char *)function->getLocal() + function->getOffset();
		*(int **)r.value = p;
		r.offset = 0;
		return r;
	}

	char buffer[128];
	memcpy(buffer,r.value,r.type->getSize());

	BasicType *basic = r.type->toBasic();

	if (basic->isFloat)
	{
		if (basic->size == 4)
		{
			Inc<float>(r.value);
		}
		else
		{
			Inc<double>(r.value);
		}
	}
	else
	{
		if (basic->size == 1)
		{
			if (basic->isSigned)
			{
				Inc<char>(r.value);
			}
			else
			{
				Inc<unsigned char>(r.value);
			}
		}
		else if (basic->size == 2)
		{
			if (basic->isSigned)
			{
				Inc<short>(r.value);
			}
			else
			{
				Inc<unsigned short>(r.value);
			}
		}
		else if (basic->size == 4)
		{
			if (basic->isSigned)
			{
				Inc<int>(r.value);
			}
			else
			{
				Inc<unsigned int>(r.value);
			}
		}
		else if (basic->size == 8)
		{
			if (basic->isSigned)
			{
				Inc<long long>(r.value);
			}
			else
			{
				Inc<unsigned long long>(r.value);
			}
		}
	}

	r.value = (char *)function->getLocal() + function->getOffset();
	memcpy(r.value, buffer, r.type->getSize());
	r.offset = 0;

	return r;
}

bool PostIncExpr::isLeftValue()
{
	return false;
}

bool PostIncExpr::isPostInc()
{
	return true;
}

AStreeRef PostDecExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<PostDecExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toPostDec();
	neg->target = target->copy(fun, block);

	return t;
}

Types * PostDecExpr::getType()
{
	return target->getType();
}

Result PostDecExpr::eval()
{
	Result r = target->eval();
	char *x = (char *)r.value;
	x += r.offset;
	r.value = x;
	r.offset = 0;

	if (r.type->isPointer())
	{
		int *p = *(int **)r.value;
		*(int *)r.value -= r.type->getSize();
		r.value = (char *)function->getLocal() + function->getOffset();
		*(int **)r.value = p;
		r.offset = 0;
		return r;
	}

	char buffer[128];
	memcpy(buffer, r.value, r.type->getSize());

	BasicType *basic = r.type->toBasic();

	if (basic->isFloat)
	{
		if (basic->size == 4)
		{
			Dec<float>(r.value);
		}
		else
		{
			Dec<double>(r.value);
		}
	}
	else
	{
		if (basic->size == 1)
		{
			if (basic->isSigned)
			{
				Dec<char>(r.value);
			}
			else
			{
				Dec<unsigned char>(r.value);
			}
		}
		else if (basic->size == 2)
		{
			if (basic->isSigned)
			{
				Dec<short>(r.value);
			}
			else
			{
				Dec<unsigned short>(r.value);
			}
		}
		else if (basic->size == 4)
		{
			if (basic->isSigned)
			{
				Dec<int>(r.value);
			}
			else
			{
				Dec<unsigned int>(r.value);
			}
		}
		else if (basic->size == 8)
		{
			if (basic->isSigned)
			{
				Dec<long long>(r.value);
			}
			else
			{
				Dec<unsigned long long>(r.value);
			}
		}
	}

	r.value = (char *)function->getLocal() + function->getOffset();
	memcpy(r.value, buffer, r.type->getSize());
	r.offset = 0;

	return r;
}

bool PostDecExpr::isLeftValue()
{
	return false;
}

bool PostDecExpr::isPostDec()
{
	return true;
}

AStreeRef ArrayAccessExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<ArrayAccessExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toArrayAccess();
	neg->index = index->copy(fun, block);
	neg->addr=addr->copy(fun, block);

	return t;
}

Types * ArrayAccessExpr::getType()
{
	if (addr->getType()->isPointer())
	{
		return addr->getType()->toPointer()->targetType;
	}

	return addr->getType()->toArray()->dataType;
}

Result ArrayAccessExpr::eval()
{
	Result i = index->eval();
	char *idx =(char *)i.value + i.offset;
	int size = i.type->getSize();
	long long id;
	bool isArray = false;
	int maxIndex = -1;

	if (size == 1)
	{
		if (i.type->toBasic()->isSigned)
		{
			id = *idx;
		}
		else
		{
			id = *(unsigned char *)idx;
		}
	}
	else if (size == 2)
	{
		if (i.type->toBasic()->isSigned)
		{
			id = *(short *)idx;
		}
		else
		{
			id = *(unsigned short *)idx;
		}
	}
	else if (size == 4)
	{
		if (i.type->toBasic()->isSigned)
		{
			id = *(int *)idx;
		}
		else
		{
			id = *(unsigned int *)idx;
		}
	}
	else if (size == 8)
	{
		if (i.type->toBasic()->isSigned)
		{
			id = *(long long *)idx;
		}
		else
		{
			id = *(unsigned long long *)idx;
		}
	}

	Result t = addr->eval();
	Result r;

	if (addr->getType()->isPointer())
	{
		r.type = addr->getType()->toPointer()->targetType;
	}
	else
	{
		ArrayType *arr = addr->getType()->toArray();
		r.type = arr->dataType;
		isArray = true;
		maxIndex = arr->capacity;
	}

	if (isArray&&id >= maxIndex)
	{
		throw std::string("数组越界");
	}

	char *s = (char *)t.value + t.offset;//指针本身的地址，是一个二级指针
	char *ptr = *(char **)s;//解除引用二级指针，得到指针的值
	r.value = ptr+id*getType()->getSize();//重新计算偏移
	r.offset = 0;//这一层没有偏移

	if (r.type->isArray())//如果解除引用的目标是数组，还需要进一步处理，使返回的值是地址
	{
		int off = function->getOffset();
		char *local = (char *)function->getLocal() + off;//指针指向栈空间
		char *add = (char *)r.value + r.offset;//这个地方是数组首元素的地址，也应该是数组的地址
		*(char **)local = add;//把栈空间强转为二级指针，并赋值为数组地址
		r.value = local;
		r.offset = 0;
	}

	return r;
}

bool ArrayAccessExpr::isLeftValue()
{
	return true;
}

bool ArrayAccessExpr::isArrayAccess()
{
	return true;
}

AStreeRef MemberAccessExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<MemberAccessExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toMemberAccess();
	neg->offset = offset;
	neg->thisType = thisType;
	neg->target = target->copy(fun, block);

	return t;
}

Types * MemberAccessExpr::getType()
{
	return thisType;
}

Result MemberAccessExpr::eval()
{
	Result r=target->eval();
	Result t;
	t.type = thisType;

	if (isLeftValue())
	{
		if (thisType->isArray())
		{
			char *s = (char *)r.value + r.offset+offset;
			t.value = (char *)function->getLocal() + function->getOffset();
			t.offset = 0;
			*(char **)t.value = s;
			return t;
		}
		else
		{
			char *s = (char *)r.value + r.offset;
			t.value = s+offset;
			t.offset = 0;
			return t;
		}
	}
	else
	{
		char *s = (char *)r.value + r.offset + offset;
		char *des = (char *)function->getLocal() + function->getOffset();
		memcpy(des, s, thisType->getSize());

		if (thisType->isArray())
		{
			char *n = des + thisType->getSize();
			*(char **)n = des;
			t.value = n;
			t.offset = 0;
			return t;
		}
		else
		{
			t.value = des;
			t.offset = 0;
			return t;
		}
	}

	return Result();
}

bool MemberAccessExpr::isLeftValue()
{
	return target->isLeftValue();
}

bool MemberAccessExpr::isMemberAccess()
{
	return true;
}

AStreeRef MemberAccessPtr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<MemberAccessPtr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toMemberAccessPtr();
	neg->offset = offset;
	neg->thisType = thisType;
	neg->target = target->copy(fun, block);

	return t;
}

Types * MemberAccessPtr::getType()
{
	return thisType;
}

Result MemberAccessPtr::eval()
{
	Result r = target->eval();

	Result t;
	t.type = thisType;
	char *s = (char *)r.value + r.offset;

	if (thisType->isArray())
	{
		char *x = *(char **)s;
		x += offset;
		t.value = (char *)function->getLocal() + function->getOffset();
		t.offset = 0;
		*(char **)t.value = x;
	}
	else
	{
		char *x = *(char **)s;
		x += offset;
		t.value = x;
		t.offset = 0;
	}

	return t;
}

bool MemberAccessPtr::isLeftValue()
{
	return true;
}

bool MemberAccessPtr::isMemberAccessPtr()
{
	return true;
}

AStreeRef FuncallExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<FuncallExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toFuncall();
	neg->thisType = thisType;
	neg->target = target->copy(fun, block);
	
	for (int i = 0; i < args.size(); ++i)
	{
		neg->args.push_back(args[i]->copy(fun,block));
	}

	return t;
}

Types * FuncallExpr::getType()
{
	return thisType;
}

Result FuncallExpr::eval()
{
	Result foo = target->eval();
	FunctionDef *f;
	if (target->getType()->isFunction())
	{
		char *s = (char *)foo.value + foo.offset;
		char *v = *(char **)s;
		f = (FunctionDef *)v;
	}
	else
	{
		char *s = (char *)foo.value + foo.offset;
		char *v = *(char **)s;
		f = (FunctionDef *)v;
	}

	if (f->isRunning())
	{
		f = f->copy();
	}

	int off = function->getOffset();
	f->setStack((char *)function->getStack() + off);
	FunctionType *ft =f->getType()->toFunction();

	int i;
	for (i = 0; i < ft->args.size(); ++i)
	{
		int ofs= function->getOffset();
		if (ft->argsType[i] == nullptr)
		{
			break;
		}
		Result r = args[i]->eval();
		if (ft->argsType[i]->isArray())
		{
			char *s = (char *)r.value + r.offset;
			int *x = *(int **)s;
			f->addArgValue(&x,sizeof(void *));
			function->setOffset(ofs + sizeof(void *));
		}
		else
		{
			auto vv = cast(r, ft->argsType[i]);
			f->addArgValue(vv.buffer, ft->argsType[i]->getSize());
			function->setOffset(ofs + ft->argsType[i]->getSize());
			vv.release();
		}
	}

	for (;i<args.size(); ++i)
	{
		int ofs = function->getOffset();
		Result r=args[i]->eval();
		if (args[i]->getType()->isArray())
		{
			char *s = (char *)r.value + r.offset;
			int *x = *(int **)s;
			f->addArgValue(&x, sizeof(void *));
			function->setOffset(ofs + sizeof(void *));
		}
		else
		{
			f->addArgValue((char *)r.value + r.offset, r.type->getSize());
			function->setOffset(ofs + r.type->getSize());
		}
	}

	function->setOffset(off);
	f->run();

	Result result;
	result.value = f->getReturnValue();
	result.offset = 0;
	result.type = getType();

	return result;
}

bool FuncallExpr::isLeftValue()
{
	return false;
}

bool FuncallExpr::isFuncall()
{
	return true;
}

AStreeRef ConditionExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<ConditionExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toCondition();
	neg->con = std::move(con->copy(fun,block));
	neg->conTrue = std::move(con->copy(fun,block));
	neg->conFalse = std::move(con->copy(fun,block));

	return t;
}

Types * ConditionExpr::getType()
{
	return conTrue->getType();
}

Result ConditionExpr::eval()
{
	Result res = con->eval();
	bool ok = parseCondition(res);

	if (ok)
	{
		return conTrue->eval();
	}
	else
	{
		return conFalse->eval();
	}

	return Result();
}

bool ConditionExpr::isLeftValue()
{
	return false;
}

bool ConditionExpr::isCondition()
{
	return true;
}

AStreeRef AddExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<AddExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toAdd();
	neg->thisType = thisType;
	neg->left = std::move(left->copy(fun, block));
	neg->right = std::move(right->copy(fun, block));

	return t;
}

Types * AddExpr::getType()
{
	return thisType;
}

template<class T>
T toT(Result t)
{
	char *s = (char *)t.value + t.offset;
	T r;

	auto *basic = t.type->toBasic();

	if (basic->isFloat)
	{
		if (basic->size == 4)
		{
			r = (T)*(float *)s;
		}
		else
		{
			r = (T)*(double *)s;
		}
	}
	else
	{
		if (basic->size == 8)
		{
			if (basic->isSigned)
			{
				r = (T)*(long long *)s;
			}
			else
			{
				r = (T)*(unsigned long long *)s;
			}
		}
		else if (basic->size == 4)
		{
			if (basic->isSigned)
			{
				r = (T)*(int *)s;
			}
			else
			{
				r = (T)*(unsigned int *)s;
			}
		}
		else if (basic->size == 2)
		{
			if (basic->isSigned)
			{
				r = (T)*(short *)s;
			}
			else
			{
				r = (T)*(unsigned short *)s;
			}
		}
		else if (basic->size == 1)
		{
			if (basic->isSigned)
			{
				r = (T)*(char *)s;
			}
			else
			{
				r = (T)*(unsigned char *)s;
			}
		}
	}
	
	return r;
}

void writeDouble(Result t, Types *tp,double c)
{
	char *s = (char *)t.value + t.offset;
	if (tp->getSize() == 4)
	{
		*(float *)s = (float)c;
		return;
	}

	*(double *)s = c;
}

void wirteLong(Result t, Types *tp, long long c)
{
	char *s = (char *)t.value + t.offset;

	auto *basic = tp->toBasic();

	if (tp->getSize() == 8)
	{
		if (basic->isSigned)
		{
			*(long long *)s = c;
		}
		else
		{
			*(unsigned long long *)s = (unsigned long long)c;
		}
	}
	else if (tp->getSize() == 4)
	{
		if (basic->isSigned)
		{
			*(int *)s = (int)c;
		}
		else
		{
			*(unsigned int *)s = (unsigned int)c;
		}
	}
	else if (tp->getSize() == 2)
	{
		if (basic->isSigned)
		{
			*(short *)s = (short)c;
		}
		else
		{
			*(unsigned short *)s = (unsigned short)c;
		}
	}
	else if (tp->getSize() == 1)
	{
		if (basic->isSigned)
		{
			*(char *)s = (char)c;
		}
		else
		{
			*(unsigned char *)s = (unsigned char)c;
		}
	}
}

Result AddExpr::eval()
{
	Result t;

	if (thisType->isPointer())
	{
		Result r = left->eval();
		char *s = (char *)r.value + r.offset;
		char *ptr = *(char **)s;
		Result r1 = right->eval();
		long long off = toT<long long>(r1);
		ptr += off*left->getType()->toPointer()->targetType->getSize();
		t.type = thisType;
		t.value = (char *)function->getLocal() + function->getOffset();
		t.offset = 0;
		*(char **)t.value = ptr;
	}
	else
	{
		if (thisType->toBasic()->isFloat)
		{
			double a = toT<double>(left->eval());
			double b = toT<double>(right->eval());

			t.type = thisType;
			t.value = (char *)function->getLocal() + function->getOffset();
			t.offset = 0;

			writeDouble(t, thisType,a+b);
		}
		else
		{
			long long a = toT<long long>(left->eval());
			long long b = toT<long long>(right->eval());

			t.type = thisType;
			t.value = (char *)function->getLocal() + function->getOffset();
			t.offset = 0;

			wirteLong(t, thisType, a + b);
		}
	}

	return t;
}

bool AddExpr::isLeftValue()
{
	return false;
}

bool AddExpr::isAdd()
{
	return true;
}

AStreeRef LessExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<LessExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toLess();
	neg->left = std::move(left->copy(fun, block));
	neg->right = std::move(right->copy(fun, block));

	return t;
}

Types * LessExpr::getType()
{
	return thisType;
}

Result LessExpr::eval()
{
	Result t;
	bool less;

	if (thisType->isPointer())
	{
		Result r = left->eval();
		char *s = (char *)r.value + r.offset;
		char *ptr = *(char **)s;
		Result r1 = right->eval();
		s = (char *)r1.value + r1.offset;
		char *ptr2= *(char **)s;
		less = ptr < ptr2;
	}
	else
	{
		bool isFloat = left->getType()->toBasic()->isFloat || right->getType()->toBasic()->isFloat;
		if (isFloat)
		{
			double a = toT<double>(left->eval());
			double b = toT<double>(right->eval());
			t.type = thisType;
			t.value = (char *)function->getLocal() + function->getOffset();
			t.offset = 0;
		}
		else
		{
			long long a = toT<long long>(left->eval());
			long long b = toT<long long>(right->eval());
			t.type = thisType;
			t.value = (char *)function->getLocal() + function->getOffset();
			t.offset = 0;
			less = a < b;
		}
	}

	wirteLong(t, thisType, less);
	return t;
}

bool LessExpr::isLeftValue()
{
	return false;
}

bool LessExpr::isLess()
{
	return true;
}

AStreeRef SubExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<SubExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toSub();
	neg->thisType = thisType;
	neg->left = std::move(left->copy(fun, block));
	neg->right = std::move(right->copy(fun, block));

	return t;
}

Types * SubExpr::getType()
{
	return thisType;
}

Result SubExpr::eval()
{
	Result t;

	if (thisType->isPointer())
	{
		Result r = left->eval();
		char *s = (char *)r.value + r.offset;
		char *ptr = *(char **)s;
		Result r1 = right->eval();
		long long off = toT<long long>(r1);
		ptr -= off*left->getType()->toPointer()->targetType->getSize();
		t.type = thisType;
		t.value = (char *)function->getLocal() + function->getOffset();
		t.offset = 0;
		*(char **)t.value = ptr;
	}
	else
	{
		if (thisType->toBasic()->isFloat)
		{
			double a = toT<double>(left->eval());
			double b = toT<double>(right->eval());

			t.type = thisType;
			t.value = (char *)function->getLocal() + function->getOffset();
			t.offset = 0;

			writeDouble(t, thisType, a - b);
		}
		else
		{
			long long a = toT<long long>(left->eval());
			long long b = toT<long long>(right->eval());

			t.type = thisType;
			t.value = (char *)function->getLocal() + function->getOffset();
			t.offset = 0;

			wirteLong(t, thisType, a - b);
		}
	}

	return t;
}

bool SubExpr::isLeftValue()
{
	return false;
}

bool SubExpr::isSub()
{
	return true;
}

AStreeRef MulExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<MulExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toMul();
	neg->thisType = thisType;
	neg->left = std::move(left->copy(fun, block));
	neg->right = std::move(right->copy(fun, block));

	return t;
}

Types * MulExpr::getType()
{
	return thisType;
}

Result MulExpr::eval()
{
	Result t;

	if (thisType->toBasic()->isFloat)
	{
		double a = toT<double>(left->eval());
		double b = toT<double>(right->eval());

		t.type = thisType;
		t.value = (char *)function->getLocal() + function->getOffset();
		t.offset = 0;

		writeDouble(t, thisType, a * b);
	}
	else
	{
		long long a = toT<long long>(left->eval());
		long long b = toT<long long>(right->eval());

		t.type = thisType;
		t.value = (char *)function->getLocal() + function->getOffset();
		t.offset = 0;

		wirteLong(t, thisType, a * b);
	}

	return t;
}

bool MulExpr::isLeftValue()
{
	return false;
}

bool MulExpr::isMul()
{
	return true;
}

AStreeRef DivExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<DivExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toDiv();
	neg->thisType = thisType;
	neg->left = std::move(left->copy(fun, block));
	neg->right = std::move(right->copy(fun, block));

	return t;
}

Types * DivExpr::getType()
{
	return thisType;
}

Result DivExpr::eval()
{
	Result t;

	if (thisType->toBasic()->isFloat)
	{
		double a = toT<double>(left->eval());
		double b = toT<double>(right->eval());

		if (!b)
		{
			throw std::string("被除数不能为0");
		}

		t.type = thisType;
		t.value = (char *)function->getLocal() + function->getOffset();
		t.offset = 0;

		writeDouble(t, thisType, a / b);
	}
	else
	{
		long long a = toT<long long>(left->eval());
		long long b = toT<long long>(right->eval());
		
		if (!b)
		{
			throw std::string("被除数不能为0");
		}

		t.type = thisType;
		t.value = (char *)function->getLocal() + function->getOffset();
		t.offset = 0;

		wirteLong(t, thisType, a / b);
	}

	return t;
}

bool DivExpr::isLeftValue()
{
	return false;
}

bool DivExpr::isDiv()
{
	return true;
}

AStreeRef AssignExpr::copy(FunctionDef * fun, AStree * block)
{
	AStreeRef t = std::make_unique<AssignExpr>();
	t->block = block;
	t->function = fun;
	t->type = type;

	auto *neg = t->toAssign();
	neg->thisType = thisType;
	neg->left = std::move(left->copy(fun, block));
	neg->right = std::move(right->copy(fun, block));

	return t;
}

Types * AssignExpr::getType()
{
	return thisType;
}

Result AssignExpr::eval()
{
	Result t = right->eval();
	EvalValue vv = cast(t, thisType);
	Result r = left->eval();
	char *buffer = (char *)r.value + r.offset;
	memcpy(buffer, vv.buffer, thisType->getSize());
	vv.release();
	return r;
}

bool AssignExpr::isLeftValue()
{
	return false;
}

bool AssignExpr::isAssign()
{
	return true;
}
