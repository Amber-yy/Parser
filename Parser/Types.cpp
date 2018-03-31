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

std::unique_ptr<Types> VoidType::copy()
{
	VoidRef p = std::make_unique<VoidType>();
	*p = *this;
	return std::move(p);
}

bool VoidType::isVoid()
{
	return true;
}

int VoidType::getSize()
{
	return 0;
}

std::unique_ptr<Types> PointerType::copy()
{
	PointerRef p = std::make_unique<PointerType>();
	*p = *this;
	return std::move(p);
}

bool PointerType::isPointer()
{
	return true;
}

int PointerType::getSize()
{
	return sizeof(void *);
}

std::unique_ptr<Types> ArrayType::copy()
{
	ArrayRef p = std::make_unique<ArrayType>();
	*p = *this;
	return std::move(p);
}

bool ArrayType::isArray()
{
	return true;
}

int ArrayType::getSize()
{
	return capacity*dataType->getSize();
}

std::unique_ptr<Types> EnumType::copy()
{
	EnumRef p = std::make_unique<EnumType>();
	*p = *this;
	return std::move(p);
}

bool EnumType::isEnum()
{
	return true;
}

int EnumType::getSize()
{
	return sizeof(int);
}

std::unique_ptr<Types> FunctionType::copy()
{
	FunctionRef p = std::make_unique<FunctionType>();
	*p = *this;
	return std::move(p);
}

bool FunctionType::isFunction()
{
	return true;
}

int FunctionType::getSize()
{
	return sizeof(void *);
}
