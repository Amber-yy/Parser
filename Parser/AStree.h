#pragma once

#include <vector>
#include <memory>

class Types;
class Parser;
class FunctionDef;
class Block;
class ReturnState;
class IfState;

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
	IfState *toIfState();
	ReturnState *toReturnState();
	Block *toBlock();
public:
	FunctionDef *function;
	AStree *block;
public:
	static Parser *parser;
};

using AStreeRef = std::unique_ptr<AStree>;

class Block :public AStree
{
public:
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