#include "AStree.h"
#include "Parser.h"
#include "FunctionDef.h"
#include "Types.h"

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
	return *((short*)(t.value) + t.offset);
}

unsigned short getUShort(Result t)
{
	return *((unsigned short*)(t.value) + t.offset);
}

int getInt(Result t)
{
	return *((int*)(t.value) + t.offset);
}

unsigned int getUInt(Result t)
{
	return *((unsigned int*)(t.value) + t.offset);
}

long long getLong(Result t)
{
	return *((long long*)(t.value) + t.offset);
}

long long getULong(Result t)
{
	return *((unsigned long long*)(t.value) + t.offset);
}

float getFloat(Result t)
{
	return *((float*)(t.value) + t.offset);
}

double getDouble(Result t)
{
	return *((double*)(t.value) + t.offset);
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
void doCast(EvalValue v,Result t,BasicType *bs)
{
	if (bs->isFloat)
	{
		if (bs->size == 4)
		{
			*(T *)v.buffer = getFloat(t);
		}
		else
		{
			*(T *)v.buffer = getDouble(t);
		}
	}
	else
	{
		if (bs->size == 1)
		{
			if (bs->isSigned)
			{
				*(T *)v.buffer = getChar(t);
			}
			else
			{
				*(float *)v.buffer = getUchar(t);
			}
		}
		else if (bs->size == 2)
		{
			if (bs->isSigned)
			{
				*(T *)v.buffer = getShort(t);
			}
			else
			{
				*(T *)v.buffer = getUShort(t);
			}
		}
		else if (bs->size == 4)
		{
			if (bs->isSigned)
			{
				*(T *)v.buffer = getInt(t);
			}
			else
			{
				*(T *)v.buffer = getUInt(t);
			}
		}
		else if (bs->size == 8)
		{
			if (bs->isSigned)
			{
				*(T *)v.buffer = getLong(t);
			}
			else
			{
				*(T *)v.buffer = getULong(t);
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
		BasicType *bs = t.type->toBasic();
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
		memcpy((char *)t.value + t.offset, v.buffer, target->getSize());
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

	for (;;)
	{
		state->eval();
		if (isBreak || function->isReturned())
		{
			break;
		}

		if (con.get() != nullptr & !parseCondition(con->eval()))
		{
			break;
		}

		if (after.get() != nullptr)
		{
			after->eval();
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
	int off = function->getOffset();

	if (isInied&&id->getType()->isStatic)
	{
		return Result();
	}

	EvalValue v= value->eval();
	Result t = id->eval();
	memcpy((char *)t.value + t.offset, v.buffer,t.type->getSize());
	v.release();

	function->setOffset(off + id->getType()->getSize());

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
		Result t = expr->eval();
		return AStree::cast(t, type);
	}

	EvalValue v;
	v.type = type;
	v.buffer = new char[type->getSize()];
	memset(v.buffer, 0, type->getSize());

	for (int i = 0; i < nexts.size(); ++i)
	{
		EvalValue t = nexts[i]->eval();
		memcpy(v.buffer + offset[i], t.buffer,nexts[i]->type->getSize());
		v.release();
	}

	return v;
}

Types * IdExpr::getType()
{
	return nullptr;
}

Result IdExpr::eval()
{
	Result t;
	t.type = type;
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
	��������ֵ������ֵ�����ص�ַ
	*/
	if (type->isArray())
	{
		int off = function->getOffset();
		void *local = function->getLocal();
		char *add=(char *)t.value + t.offset;//���������׵�ַ������ջ�ռ�
		*(char **)local = add;
		t.value = (char *)local+off;
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

Types * PreDecExpr::getType()
{
	return target->getType();
}

template<class T>
void Dec(void *a)
{
	*(T *)a += 1;
}

Result PreDecExpr::eval()
{
	Result r = target->eval();
	char *x = (char *)r.value;
	x += r.offset;
	r.value = x;

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

	char *s = (char *)t.value + t.offset;//ָ�뱾��ĵ�ַ����һ������ָ��
	r.value = *(char **)s;//������ö���ָ�룬�õ�ָ���ֵ
	r.offset = 0;//û��ƫ��

	if (r.type->isArray())//���������õ�Ŀ�������飬����Ҫ��һ������ʹ���ص�ֵ�ǵ�ַ
	{
		int off = function->getOffset();
		void *local = function->getLocal();
		char *add = (char *)r.value + r.offset;//���������׵�ַ������ջ�ռ�
		*(char **)local = add;
		r.value = (char *)local+off;
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

Types * GetAddrExpr::getType()
{
	return type;
}

Result GetAddrExpr::eval()
{
	Result t=target->eval();

	int off = function->getOffset();
	char *local = (char *)function->getLocal();
	char *add = (char *)t.value + t.offset;//������ݵĵ�ַ������ջ�ռ�
	t.value = (char *)local + off;
	*(char **)t.value = add;
	t.offset = 0;

	t.type = type;

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
