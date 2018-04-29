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
	bool isRunning();
	void setReturned();
	void *getLocal();
	void *getStack();
	void setOffset(int off);
	void setStack(void *s);
	void addArgValue(void *s, int size);
	void setReturnValue(void *s);
	void *getReturnValue();
	void run();
	int getOffset();
	Types *getType();
	FunctionDef *copy();
public:
	static Parser *parser;
protected:
	struct Data;
	Data *data;
};

