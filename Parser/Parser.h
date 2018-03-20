#pragma once

#include <string>

class Types;
struct symbol;
struct variables;
struct VariableValue;

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
	void getStructVariable(bool isStatic,bool isConst,Types *type,Types *constType);
	void getStructTypedef(bool isStatic, bool isConst, Types *type, Types *constType,std::string &name);
	void getUnionVariable(bool isStatic, bool isConst, Types *type, Types *constType);
	void getUnionTypedef(bool isStatic, bool isConst, Types *type, Types *constType, std::string &name);
	VariableValue getConstIni(Types *type);
	int getArrayIndexConst();
	variables getVariables(bool ini=true);
	symbol *findSymbol(const std::string &name);
	Types *peekType();
	int getBasicType(std::vector<std::string> &basic);
protected:
	struct Data;
	Data *data;
};

