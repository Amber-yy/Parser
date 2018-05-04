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
struct IniList;
struct TypeToken;
struct StructDef;
struct UnionDef;
struct EnumDef;
struct OperatorValue;

using IniRef = std::unique_ptr<IniList>;
using AStreeRef = std::unique_ptr<AStree>;
typedef char *(*LibFunction)(void *,int);

class Parser
{
public:
	Parser();
	~Parser();
	void parse(const std::string &file);
	void run();
	void getLine();
	void getStruct(bool isTypedef=false);
	void getUnion(bool isTypedef = false);
	void getEnum(bool isTypedef = false);
	void parseTypedef();
	void parseVariable();
	void addError(std::string &info);
	void requireToken(const std::string &token);
	void createBaseType();
	void createBinary();
	void createLib();
	void getFunctionDef();
	void addLibFunction(const std::string &name,LibFunction fun);
	void getStructVariable(bool isStatic,bool isConst,Types *type,Types *constType);
	void getStructTypedef(bool isStatic, bool isConst, Types *type, Types *constType,std::string &name);
	void getUnionVariable(bool isStatic, bool isConst, Types *type, Types *constType);
	void getUnionTypedef(bool isStatic, bool isConst, Types *type, Types *constType, std::string &name);
	void getEnumVariable(bool isStatic, bool isConst, Types *type, Types *constType);
	void getEnumTypedef(bool isStatic, bool isConst, Types *type, Types *constType, std::string &name);
	void *getStackRegion();
	void *getStaticRegion();
	void *getGlobalRegion();
	AStreeRef getAtomic(AStree *block);
	AStreeRef getPrimary(AStree *block);
	AStreeRef getStatement(AStree *block);
	AStreeRef getVariableDefState(AStree *block);
	AStreeRef getExprState(AStree *block);
	AStreeRef getExpr(AStree *block,bool comma=true);
	AStreeRef doShift(AStree *block,OperatorValue *op, AStreeRef left,bool comma = true);
	AStreeRef getConditionExpr(AStree *block,AStreeRef left);
	AStreeRef getSwitchState(AStree *block);
	AStreeRef getIfState(AStree *block);
	AStreeRef getWhileState(AStree *block);
	AStreeRef getDoWhileState(AStree *block);
	AStreeRef getForState(AStree *block);
	AStreeRef getBreakState(AStree *block);
	AStreeRef getReturnState(AStree *block);
	AStreeRef getStringLiteral(AStree *block);
	AStreeRef getBlock(FunctionDef *fun=nullptr,AStree *statement=nullptr);
	AStreeRef makeAddExpr(AStreeRef left,AStreeRef right);
	AStreeRef makeSubExpr(AStreeRef left, AStreeRef right);
	AStreeRef makeMulExpr(AStreeRef left, AStreeRef right);
	AStreeRef makeDiviExpr(AStreeRef left, AStreeRef right);
	AStreeRef makeAssignExpr(AStreeRef left, AStreeRef right);
	AStreeRef makeLess(AStreeRef left, AStreeRef right);
	StructDef *getStructDef(int index);
	UnionDef *getUnionDef(int index);
	EnumDef *getEnumDef(int index);
	Types *parseType(Types *pre,std::list<TypeToken> &tokens,int name,int level);//0表示需要变量名，1表示刚刚有名字，2表示不能有
	VariableValue getConstIni(Types *type);
	VariableValue getConstIniCore(Types *type);
	IniRef getIni(Types *type, AStree *block);
	IniRef getIniCore(Types *type, AStree *block);
	variable getVariables(Types *pre,bool typeTran=false);
	OperatorValue *findOperator(const std::string &name,bool comma);
	symbol *findSymbol(const std::string &name);
	Types *peekType(bool args=false);
	int getBasicType(std::vector<std::string> &basic);
protected:
	struct Data;
	Data *data;
};

