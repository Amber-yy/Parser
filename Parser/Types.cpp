#include "Types.h"
#include "Custom.h"

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

bool BasicType::isBasic()
{
	return true;
}

int BasicType::getSize()
{
	return 4;
}

bool StructType::isStruct()
{
	return true;
}

int StructType::getSize()
{
	return structDef->size;
}

bool UnionType::isUnion()
{
	return true;
}

int UnionType::getSize()
{
	return unionDef->size;
}

bool VoidType::isVoid()
{
	return true;
}

int VoidType::getSize()
{
	return 0;
}

bool PointerType::isPointer()
{
	return true;
}

int PointerType::getSize()
{
	return sizeof(void *);
}

bool ArrayType::isArray()
{
	return true;
}

int ArrayType::getSize()
{
	return capacity*dataType->getSize();
}
