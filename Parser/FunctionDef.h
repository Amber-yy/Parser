#pragma once

class Parser;

class FunctionDef
{
public:
	FunctionDef();
	~FunctionDef();
protected:
	static Parser *parser;
protected:
	struct Data;
	Data *data;
};

