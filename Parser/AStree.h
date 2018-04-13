#pragma once

class Types;
class Parser;

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
public:
	static Parser *parser;
};

