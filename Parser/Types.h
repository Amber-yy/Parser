#pragma once

struct StructDef;
struct UnionDef;
struct EnumDef;

#include <memory>
#include <string>

class StructType;
class UnionType;
class BasicType;
class ArrayType;
class VoidType;

class Types
{
public:
	Types();
	~Types();
	StructType *toStruct();
	UnionType *toUnion();
	BasicType *toBasic();
	ArrayType *toArray();
	VoidType *toVoid();
	virtual bool isBasic();
	virtual bool isPointer();
	virtual bool isArray();
	virtual bool isFunction();
	virtual bool isStruct();
	virtual bool isUnion();
	virtual bool isEnum();
	virtual bool isVoid();
	virtual int getSize() = 0;
	virtual std::unique_ptr<Types> copy()=0;
	bool isStatic;
	bool isConst;
};

class BasicType :public Types
{
public:
	BasicType();
	virtual std::unique_ptr<Types> copy()override;
	virtual bool isBasic()override;
	virtual int getSize()override;
	bool isSigned;
	bool isFloat;
	int size;
};

class StructType :public Types
{
public:
	virtual std::unique_ptr<Types> copy()override;
	virtual bool isStruct()override;
	virtual int getSize()override;
	std::string name;
	StructDef *structDef;
};

class UnionType :public Types
{
public:
	virtual std::unique_ptr<Types> copy()override;
	virtual bool isUnion()override;
	virtual int getSize()override;
	std::string name;
	UnionDef *unionDef;
};

class VoidType :public Types
{
public:
	virtual std::unique_ptr<Types> copy()override;
	virtual bool isVoid()override;
	virtual int getSize()override;
};

class PointerType :public Types
{
public:
	virtual std::unique_ptr<Types> copy()override;
	virtual bool isPointer()override;
	virtual int getSize()override;
	Types *targetType;
};

class ArrayType :public Types
{
public:
	virtual std::unique_ptr<Types> copy()override;
	virtual bool isArray()override;
	virtual int getSize()override;
	int capacity;
	Types *dataType;
};

class EnumType :public Types
{
public:
	virtual std::unique_ptr<Types> copy()override;
	virtual bool isEnum()override;
	virtual int getSize()override;
	EnumDef *enumDef;
};

class FunctionType :public Types
{
public:
	virtual std::unique_ptr<Types> copy()override;
	virtual bool isFunction()override;
	virtual int getSize()override;
	Types *returnType;
};

using TypeRef = std::unique_ptr<Types>;
using StructRef = std::unique_ptr<StructType>;
using UnionRef = std::unique_ptr<UnionType>;
using EnumRef = std::unique_ptr<EnumType>;
using BasicRef = std::unique_ptr<BasicType>;
using PointerRef = std::unique_ptr<PointerType>;
using ArrayRef = std::unique_ptr<ArrayType>;
using VoidRef = std::unique_ptr<VoidType>;
using FunctionRef = std::unique_ptr<FunctionType>;
