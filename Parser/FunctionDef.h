#pragma once

#include <memory>

class Parser;
class Types;
class AStree;

using AStreeRef = std::unique_ptr<AStree>;

class FunctionDef
{
public:
	FunctionDef();
	~FunctionDef();
	void setBlock(AStreeRef &ast);
	void setType(Types *tp);
	bool isReturned();
	void setReturned();
	void *getLocal();
	void *getStack();
	void setOffset(int off);
	int getOffset();
	Types *getType();
public:
	static Parser *parser;
protected:
	struct Data;
	Data *data;
};

