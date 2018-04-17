#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>

class Types;
class FunctionDef;
class AStree;
struct symbol;
struct variable;
struct VariableValue;
struct TypeToken;
struct StructDef;
struct UnionDef;
struct EnumDef;

using AStreeRef = std::unique_ptr<AStree>;

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
	void parseVariable();
	void addError(std::string &info);
	void requireToken(const std::string &token);
	void createBaseType();
	void getFunctionDef();
	void getStructVariable(bool isStatic,bool isConst,Types *type,Types *constType);
	void getStructTypedef(bool isStatic, bool isConst, Types *type, Types *constType,std::string &name);
	void getUnionVariable(bool isStatic, bool isConst, Types *type, Types *constType);
	void getUnionTypedef(bool isStatic, bool isConst, Types *type, Types *constType, std::string &name);
	void getEnumVariable(bool isStatic, bool isConst, Types *type, Types *constType);
	void getEnumTypedef(bool isStatic, bool isConst, Types *type, Types *constType, std::string &name);
	AStreeRef getStatement(AStree *block);
	AStreeRef getVariableDefState(AStree *block);
	AStreeRef getExprState(AStree *block);
	AStreeRef getExpr(AStree *block);
	AStreeRef getSwitchState(AStree *block);
	AStreeRef getIfState(AStree *block);
	AStreeRef getWhileState(AStree *block);
	AStreeRef getDoWhileState(AStree *block);
	AStreeRef getForState(AStree *block);
	AStreeRef getBreakState(AStree *block);
	AStreeRef getReturnState(AStree *block);
	AStreeRef getBlock(FunctionDef *fun=nullptr,AStree *statement=nullptr);
	StructDef *getStructDef(int index);
	UnionDef *getUnionDef(int index);
	EnumDef *getEnumDef(int index);
	Types *parseType(Types *pre,std::list<TypeToken> &tokens,int name,int level);//0表示需要变量名，1表示刚刚有名字，2表示不能有
	VariableValue getConstIni(Types *type);
	VariableValue getConstIniCore(Types *type);
	variable getVariables(Types *pre);
	symbol *findSymbol(const std::string &name);
	Types *peekType(bool args=false);
	int getBasicType(std::vector<std::string> &basic);
protected:
	struct Data;
	Data *data;
};

