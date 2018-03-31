#pragma once

#include <string>
#include <vector>
#include <list>

class Types;
struct symbol;
struct variable;
struct VariableValue;
struct TypeToken;
struct StructDef;
struct UnionDef;
struct EnumDef;

class Parser
{
public:
	Parser();
	~Parser();
	void parse(const std::string &file);
	void getLine();
	void getStruct(bool isTypedef=false);
	void getUnion(bool isTypedef = false);
	void getEnum(bool isTypedef = false);
	void parseTypedef();
	void addError(std::string &info);
	void requireToken(const std::string &token);
	void createBaseType();
	void getStructVariable(bool isStatic,bool isConst,Types *type,Types *constType);
	void getStructTypedef(bool isStatic, bool isConst, Types *type, Types *constType,std::string &name);
	void getUnionVariable(bool isStatic, bool isConst, Types *type, Types *constType);
	void getUnionTypedef(bool isStatic, bool isConst, Types *type, Types *constType, std::string &name);
	void getEnumVariable(bool isStatic, bool isConst, Types *type, Types *constType);
	void getEnumTypedef(bool isStatic, bool isConst, Types *type, Types *constType, std::string &name);
	StructDef *getStructDef(int index);
	UnionDef *getUnionDef(int index);
	EnumDef *getEnumDef(int index);
	Types *parseType(Types *pre,std::list<TypeToken> &tokens,int name,int level);//0表示需要变量名，1表示刚刚有名字，2表示不能有
	VariableValue getConstIni(Types *type);
	variable getVariables(Types *pre);
	symbol *findSymbol(const std::string &name);
	Types *peekType(bool args=false);
	int getBasicType(std::vector<std::string> &basic);
protected:
	struct Data;
	Data *data;
};

