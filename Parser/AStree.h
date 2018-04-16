#pragma once

#include <vector>
#include <memory>

class Types;
class Parser;
class FunctionDef;
class Block;

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
	virtual Result eval()=0;
	virtual bool isLeftValue() = 0;
	virtual bool isBlock();
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
	virtual Result eval()override;
	virtual bool isLeftValue()override;
	virtual bool isBlock()override;
	std::vector<AStreeRef> statements;
};