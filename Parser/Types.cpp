#include "Types.h"
#include "Custom.h"

#include "Parser.h"

Parser * Types::parser;

Types::Types()
{
	isConst=false;
	isStatic = false;
}

Types::~Types()
{

}

StructType * Types::toStruct()
{
	return dynamic_cast<StructType *>(this);
}

UnionType * Types::toUnion()
{
	return dynamic_cast<UnionType *>(this);
}

BasicType * Types::toBasic()
{
	return dynamic_cast<BasicType *>(this);
}

ArrayType * Types::toArray()
{
	return dynamic_cast<ArrayType *>(this);
}

VoidType * Types::toVoid()
{
	return dynamic_cast<VoidType *>(this);
}

PointerType * Types::toPointer()
{
	return dynamic_cast<PointerType *>(this);
}

FunctionType * Types::toFunction()
{
	return dynamic_cast<FunctionType *>(this);
}

bool Types::isBasic()
{
	return false;
}

bool Types::isPointer()
{
	return false;
}

bool Types::isArray()
{
	return false;
}

bool Types::isFunction()
{
	return false;
}

bool Types::isStruct()
{
	return false;
}

bool Types::isUnion()
{
	return false;
}

bool Types::isEnum()
{
	return false;
}

bool Types::isVoid()
{
	return false;
}

BasicType::BasicType()
{
	isSigned=true;
	isFloat=false;
	size=0;
}

bool BasicType::equal(Types * tp)
{
	if (!tp->isBasic())
	{
		return false;
	}

	return memcmp(this, tp, sizeof(BasicType))==0;
}

bool BasicType::compatible(Types * tp)
{
	return tp->isBasic();
}

std::unique_ptr<Types> BasicType::copy()
{
	BasicRef p = std::make_unique<BasicType>();
	*p = *this;
	return std::move(p);
}

bool BasicType::isBasic()
{
	return true;
}

int BasicType::getSize()
{
	return 4;
}

bool BasicType::canInstance()
{
	return true;
}

bool StructType::equal(Types * tp)
{
	if (!tp->isStruct())
	{
		return false;
	}

	auto tp1 = tp->toStruct();
	if (isStatic == tp1->isStatic&&isConst == tp1->isConst&&name == tp1->name&&structDef == tp1->structDef)
	{
		return true;
	}

	return false;
}

bool StructType::compatible(Types * tp)
{
	return tp->isStruct() && !tp->toStruct()->name.empty() && tp->toStruct()->name == name;
}

std::unique_ptr<Types> StructType::copy()
{
	StructRef p = std::make_unique<StructType>();
	p->isConst = isConst;
	p->isStatic = isStatic;
	p->name = name;
	p->structDef = structDef;
	return std::move(p);
}

bool StructType::isStruct()
{
	return true;
}

int StructType::getSize()
{
	return parser->getStructDef(structDef)->size;
}

bool StructType::canInstance()
{
	return getSize()>0;
}

bool UnionType::equal(Types * tp)
{
	if (!tp->isUnion())
	{
		return false;
	}

	auto tp1 = tp->toUnion();
	if (isStatic == tp1->isStatic&&isConst == tp1->isConst&&name == tp1->name&&unionDef == tp1->unionDef)
	{
		return true;
	}

	return false;
}

bool UnionType::compatible(Types * tp)
{
	return tp->isUnion() && !tp->toUnion()->name.empty() && tp->toUnion()->name == name;
}

std::unique_ptr<Types> UnionType::copy()
{
	UnionRef p = std::make_unique<UnionType>();
	p->isConst = isConst;
	p->isStatic = isStatic;
	p->name = name;
	p->unionDef = unionDef;
	return std::move(p);
}

bool UnionType::isUnion()
{
	return true;
}

int UnionType::getSize()
{
	return parser->getUnionDef(unionDef)->size;
}

bool UnionType::canInstance()
{
	return getSize()>0;
}

bool VoidType::equal(Types * tp)
{
	if (!tp->isVoid())
	{
		return false;
	}

	return true;
}

bool VoidType::compatible(Types * tp)
{
	return false;
}

std::unique_ptr<Types> VoidType::copy()
{
	VoidRef p = std::make_unique<VoidType>();
	*p = *this;
	return std::move(p);
}

bool VoidType::canInstance()
{
	return false;
}

bool VoidType::isVoid()
{
	return true;
}

int VoidType::getSize()
{
	return 0;
}

bool PointerType::equal(Types * tp)
{
	if (!tp->isPointer())
	{
		return false;
	}

	auto tp1 = tp->toPointer();
	if (isStatic == tp1->isStatic&&isConst == tp1->isConst&&targetType->equal(tp1->targetType))
	{
		return true;
	}

	return false;
}

bool PointerType::compatible(Types * tp)
{
	if (!tp->isPointer())
	{
		return false;
	}

	PointerType *ptr = tp->toPointer();
	if (targetType->isVoid() || ptr->targetType->isVoid())
	{
		return true;
	}

	return targetType->equal(ptr);
}

std::unique_ptr<Types> PointerType::copy()
{
	PointerRef p = std::make_unique<PointerType>();
	*p = *this;
	return std::move(p);
}

bool PointerType::canInstance()
{
	return true;
}

bool PointerType::isPointer()
{
	return true;
}

int PointerType::getSize()
{
	return sizeof(void *);
}

bool ArrayType::equal(Types * tp)
{
	if (!tp->isArray())
	{
		return false;
	}

	auto tp1 = tp->toArray();
	if (isStatic == tp1->isStatic&&isConst == tp1->isConst&&capacity==tp1->capacity&&dataType->equal(tp1->dataType))
	{
		return true;
	}

	return false;
}

bool ArrayType::compatible(Types * tp)
{
	return equal(tp);
}

std::unique_ptr<Types> ArrayType::copy()
{
	ArrayRef p = std::make_unique<ArrayType>();
	*p = *this;
	return std::move(p);
}

bool ArrayType::canInstance()
{
	return dataType->canInstance();
}

bool ArrayType::isArray()
{
	return true;
}

int ArrayType::getSize()
{
	return capacity*dataType->getSize();
}

bool EnumType::equal(Types * tp)
{
	if (!tp->isEnum())
	{
		return false;
	}

	return memcmp(this, tp, sizeof(EnumType)) == 0;
}

bool EnumType::compatible(Types * tp)
{
	if (tp->isEnum())
	{
		return true;
	}
	if (tp->isBasic())
	{
		BasicType *bp = tp->toBasic();
		return !bp->isFloat;
	}

	return false;
}

std::unique_ptr<Types> EnumType::copy()
{
	EnumRef p = std::make_unique<EnumType>();
	*p = *this;
	return std::move(p);
}

bool EnumType::canInstance()
{
	return true;
}

bool EnumType::isEnum()
{
	return true;
}

int EnumType::getSize()
{
	return sizeof(int);
}

bool FunctionType::equal(Types * tp)
{
	if (!tp->isFunction())
	{
		return false;
	}

	if (isStatic != tp->isStatic)
	{
		return false;
	}

	auto tp1 = tp->toFunction();

	if (argsType.size() != tp1 -> argsType.size())
	{
		return false;
	}

	for (int i = 0; i < argsType.size(); ++i)
	{
		if (argsType[i] == tp1->argsType[i] || argsType[i]->equal(tp1->argsType[i]))
		{
			continue;
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool FunctionType::compatible(Types * tp)
{
	return equal(tp);
}

std::unique_ptr<Types> FunctionType::copy()
{
	FunctionRef p = std::make_unique<FunctionType>();
	*p = *this;
	return std::move(p);
}

bool FunctionType::canInstance()
{
	return false;
}

bool FunctionType::isFunction()
{
	return true;
}

int FunctionType::getSize()
{
	return sizeof(void *);
}

bool FunctionType::hasVariableArg()
{
	return argsType[argsType.size()-1]==nullptr;
}
