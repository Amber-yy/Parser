#pragma once

#include <string>

class Type;
struct symbol;
struct variables;

class Parser
{
public:
	Parser();
	~Parser();
	void parse(const std::string &file);
	void getLine();
	void getStruct(bool isTypedef=false);
	void getUnion(bool isTypedef = false);
	void addError(std::string &info);
	void requireToken(const std::string &token);
	void createBaseType();
	variables getVariables(bool ini=true);
	symbol *findSymbol(const std::string &name);
	Types *peekType();
	int getBasicType(std::vector<std::string> &basic);
protected:
	struct Data;
	Data *data;
};

