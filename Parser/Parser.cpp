#include "Parser.h"
#include "Preprocessor.h"
#include "Tokenizer.h"
#include "Types.h"
#include "Custom.h"
#include "FunctionDef.h"
#include "AStree.h"

#include <iostream>
#include <memory>
#include <list>
#include <map>
#include <algorithm>

using FunctionDefRef = std::unique_ptr<FunctionDef>;

enum ParseType
{
	TYPEDEF,
	STRUCT,
	UNION,
	ENUM,
	VARIABLE,
};

struct TypeToken
{
	Token *token;
	int index;
};

struct variable
{
	std::string name;
	Types * type;
};

struct symbol
{
	symbol()
	{
		isStatic = false;
	}
	bool isVariable;
	bool isDefined;
	bool isStatic;
	int offSet;
	Types *type;
	Types *constType;//只有当isVariable为false时这个字段才有效
};

struct VariableValue
{
	VariableValue()
	{
		buffer = nullptr;
	}
	void release()
	{
		delete buffer;
	}
	char *buffer;
	Types *type;
};

struct Value
{
	bool isDefined;
	int offSet;
	Types *type;
};

struct Region
{
	Region()
	{
		dataSize = 1024;
		data = new char[dataSize];
		currentOffset = 0;
	}
	void addValue(VariableValue &v)
	{
		if (v.type->getSize() + currentOffset > dataSize)
		{
			dataSize *= 2;
			char *temp = new char[dataSize];
			memcpy(temp, data, dataSize / 2);
			delete[] data;
			data = temp;
		}

		memcpy(data + currentOffset, v.buffer, v.type->getSize());
		currentOffset += v.type->getSize();
	}
	char *data;
	int dataSize;
	int currentOffset;
	std::map<std::string, Value> values;
};

struct Parser::Data
{
	Preprocessor pre;
	Tokenizer token;
	Region globalRegion;
	Region staticRegion;
	std::vector<std::map<std::string, symbol>> symbols;
	std::vector<StructDef> allStructDef;
	std::vector<UnionDef> allUnionDef;
	std::vector<EnumDef> allEnumDef;
	std::vector<TypeRef> allTypes;
	std::list<Token *> line;
	std::vector<std::list<TypeToken>> origin;
	std::vector<std::string>vName;
	std::vector<FunctionDefRef> allFunctions;
	std::list<TypeToken> argTokens;
	FunctionDef *currentFunction;
	char *stack;
	ParseType type;
	int currentOffset;
	int currentLevel;
	int index;
	int mainIndex;
};

Parser::Parser()
{
	data = new Data;
	Types::parser = this;
	FunctionDef::parser = this;
	AStree::parser = this;
	//data->statck=new char[1024*1024*2];//2m的栈空间
	createBaseType();
}

Parser::~Parser()
{
	delete data;
	//delete[] stack;
}

void Parser::parse(const std::string &file)
{
	std::string code=data->pre.doFile(file);
	data->token.parse(code);
	data->currentLevel = 0;
	data->symbols.resize(2);
	data->mainIndex = -1;

	while (data->token.hasMore())
	{
		getLine();
		if (data->type == STRUCT)
		{
			getStruct();
		}
		else if (data->type == UNION)
		{
			getUnion();
		}
		else if (data->type == ENUM)
		{
			getEnum();
		}
		else if (data->type == TYPEDEF)
		{
			parseTypedef();
		}
		else if (data->type == VARIABLE)
		{
			parseVariable();
		}

	}

	int a;
	a = 10;
}

void Parser::getLine()
{
	while (data->token.peek(0).isEos())
	{
		data->token.read();
	}

	int i;

	data->line.clear();
	for(i=0;;++i)
	{
		const std::string &str = data->token.peek(i).getString();
		if (str == "{" || str == ";")
		{
			//data->line.push_back(&data->token.peek(i));
			break;
		}

		data->line.push_back(&data->token.peek(i));
	}

	auto it = data->line.begin();
	const std::string &str = (*it)->getString();

	if (str == "typedef")
	{
		data->type = TYPEDEF;
		return;
	}

	auto getType = [this,&it](ParseType tp)
	{
		++it;
		if (it == data->line.end())
		{
			data->type = tp;
			return;
		}

		if ((*it)->getString() == ";")
		{
			addError(std::string("应输入标识符"));
		}

		if ((*it)->isIdentifier() && !(*it)->isKeyWord())
		{
			++it;
			if (it == data->line.end())
			{
				data->type = tp;
			}
			else
			{
				data->type = VARIABLE;
			}
		}
		else
		{
			data->type = tp;
		}
	};

	for (;it != data->line.end(); ++it)
	{
		const std::string &str = (*it)->getString();
		if (str == "struct")
		{
			getType(STRUCT);
			return;
		}

		if (str == "union")
		{
			getType(UNION);
			return;
		}

		if (str == "enum")
		{
			getType(ENUM);
			return;
		}

	}

	data->type = VARIABLE;
}

void Parser::getStruct(bool isTypedef)
{
	bool isStatic = false;
	bool isConst = false;
	bool isAnonymous = false;
	bool isDef = false;
	StructDef strdef;

	while (true)
	{
		const std::string &str = data->token.peek(0).getString();
		if (str == "struct")
		{
			data->token.read();
			break;
		}
		if (str == "static")
		{
			if (isStatic == false)
			{
				isStatic = true;
			}
			else
			{
				addError(std::string("无效的类型组合说明符"));
			}
		}
		else if (str == "const")
		{
			if (isConst == false)
			{
				isConst = true;
			}
			else
			{
				addError(std::string("无效的类型组合说明符"));
			}
		}

		data->token.read();
	}

	if (isTypedef&&isStatic)
	{
		addError(std::string("不能使用多种储存符"));
	}

	std::string name;
	const Token &t = data->token.peek(0);

	if (t.getString() == "{")//是匿名定义
	{
		isDef = true;
		isAnonymous = true;
	}
	else if (!t.isIdentifier() || t.isKeyWord())//是定义，但是标识符不正确
	{
		addError(std::string("应输入正确的标识符"));
	}
	else//是一个合法的标识符，是否是定义取决于下一个字符是否是{
	{
		name = t.getString();
		data->token.read();
		isDef = data->token.peek(0).getString() == "{";
	}

	auto it=data->symbols[0].find(name);

	auto reg = [&]()
	{
		symbol t;
		t.isDefined = false;
		t.isVariable = false;
		t.offSet = -1;
		strdef.size = 0;
		strdef.name = name;
		data->allStructDef.push_back(strdef);

		StructRef tpr = std::make_unique<StructType>();
		tpr->name = name;
		tpr->structDef = data->allStructDef.size() - 1;

		StructRef tprc = std::make_unique<StructType>();
		tprc->name = name;
		tprc->structDef = tpr->structDef;
		tprc->isConst = true;

		t.type = tpr.get();
		t.constType = tprc.get();

		data->allTypes.push_back(std::move(tpr));//类型
		data->allTypes.push_back(std::move(tprc));//常类型
		data->symbols[0].insert(std::pair<std::string, symbol>(name, t));
	};

	if (it == data->symbols[0].end())//没有声明过
	{
		if (!isDef)//是声明则直接声明
		{
			reg();
			data->token.read();
			return;
		}
		if (!isAnonymous)
		{
			reg();
		}
	}
	else if (!it->second.type->isStruct())//有声明不是结构体
	{
		addError(std::string("重定义，不同的类型"));
	}
	else if (it->second.isDefined&&isDef)//有声明且已经定义过
	{
		addError(std::string("结构体重定义"));
	}

	it = data->symbols[0].find(name);

	/*获取结构体定义*/
	requireToken("{");
	
	strdef.name = name;

	while (true)
	{
		if (data->token.peek(0).getString() == "}")
		{
			if (strdef.size == 0)
			{
				addError(std::string("应输入声明"));
			}
			break;
		}

		Types *tp = peekType();

		if (tp->isStatic || tp->isConst)
		{
			addError(std::string("无效的类型说明符"));
		}

		std::vector<variable> vs;
		while (true)
		{
			if (data->token.peek(0).isEos())
			{
				if (vs.empty())
				{
					addError(std::string("应该输入成员变量"));
				}
				data->token.read();
				break;
			}

			variable v = getVariables(tp);
			if (!v.type->canInstance()|| v.type->isStatic || v.type->isConst)
			{
				addError(std::string("不能添加此类型的成员"));
			}

			vs.push_back(v);
		}

		for(int i=0;i<vs.size();++i)
		{
			for (int j = 0; j < strdef.members.size(); ++j)
			{
				if (strdef.members[j] == vs[i].name)
				{
					addError(std::string("成员重定义"));
				}
			}

			strdef.members.push_back(vs[i].name);

			if (strdef.offSets.empty())
			{
				strdef.offSets.push_back(0);
			}
			else
			{
				strdef.offSets.push_back(strdef.offSets[strdef.offSets.size() - 1] + vs[i].type->getSize());
			}

			strdef.size += vs[i].type->getSize();
			strdef.types.push_back(vs[i].type);
		}
	}

	requireToken("}");

	Types *type = nullptr;
	Types *constType = nullptr;

	if (!isAnonymous)//更新
	{
		int index = data->allStructDef.size() - 1;
		data->allStructDef[index] = strdef;
		it->second.isDefined = true;
		type = it->second.type;
		constType = it->second.constType;
	}

	if (data->token.peek(0).getString() == ";")
	{
		data->token.read();
		return;
	}

	if (isAnonymous)
	{
		data->allStructDef.push_back(strdef);
		StructRef tpr = std::make_unique<StructType>();
		tpr->name = name;
		tpr->structDef = data->allStructDef.size() - 1;

		StructRef tprc = std::make_unique<StructType>();
		tprc->name = name;
		tprc->structDef = tpr->structDef;
		tprc->isConst = true;

		type = tpr.get();
		constType =tprc.get();

		data->allTypes.push_back(std::move(tpr));//类型
		data->allTypes.push_back(std::move(tprc));//常类型
	}

	//全局变量的解析
	if (isTypedef)
	{
		getStructTypedef(isStatic, isConst, type, constType, name);
	}
	else
	{
		getStructVariable(isStatic, isConst, type, constType);
	}

	requireToken(";");
}

void Parser::getUnion(bool isTypedef)
{
	bool isStatic = false;
	bool isConst = false;
	bool isAnonymous = false;
	bool isDef = false;
	UnionDef unidef;

	while (true)
	{
		const std::string &str = data->token.peek(0).getString();
		if (str == "union")
		{
			data->token.read();
			break;
		}
		if (str == "static")
		{
			if (isStatic == false)
			{
				isStatic = true;
			}
			else
			{
				addError(std::string("无效的类型组合说明符"));
			}
		}
		else if (str == "const")
		{
			if (isConst == false)
			{
				isConst = true;
			}
			else
			{
				addError(std::string("无效的类型组合说明符"));
			}
		}

		data->token.read();
	}

	if (isTypedef&&isStatic)
	{
		addError(std::string("不能使用多种储存符"));
	}

	std::string name;
	const Token &t = data->token.read();

	if (t.getString() == "{")//是匿名定义
	{
		isDef = true;
		isAnonymous = true;
	}
	else if (!t.isIdentifier() || t.isKeyWord())//是定义，但是标识符不正确
	{
		addError(std::string("应输入正确的标识符"));
	}
	else//是一个合法的标识符，是否是定义取决于下一个字符是否是{
	{
		name = t.getString();
		isDef = data->token.peek(0).getString() == "{";
	}

	auto it = data->symbols[0].find(name);

	auto reg = [&]()
	{
		symbol t;
		t.isDefined = false;
		t.isVariable = false;
		t.offSet = -1;
		unidef.size = 0;
		unidef.name = name;
		data->allUnionDef.push_back(unidef);

		UnionRef tpr = std::make_unique<UnionType>();
		tpr->name = name;
		tpr->unionDef = data->allUnionDef .size() - 1;

		UnionRef tprc = std::make_unique<UnionType>();
		tprc->name = name;
		tprc->unionDef = tpr->unionDef;
		tprc->isConst = true;

		t.type = tpr.get();
		t.constType = tprc.get();

		data->allTypes.push_back(std::move(tpr));//类型
		data->allTypes.push_back(std::move(tprc));//常类型
		data->symbols[0].insert(std::pair<std::string, symbol>(name, t));
	};

	if (it == data->symbols[0].end())//没有声明过
	{
		if (!isDef)//是声明则直接声明
		{
			reg();
			data->token.read();
			return;
		}
		if (!isAnonymous)
		{
			reg();
		}
	}
	else if (!it->second.type->isUnion())//有声明不是结构体
	{
		addError(std::string("重定义，不同的类型"));
	}
	else if (it->second.isDefined&&isDef)//有声明且已经定义过
	{
		addError(std::string("结构体重定义"));
	}

	it = data->symbols[0].find(name);
	unidef.name = name;

	/*获取结构体定义*/
	requireToken("{");

	while (true)
	{
		if (data->token.peek(0).getString() == "}")
		{
			if (unidef.size == 0)
			{
				addError(std::string("应输入声明"));
			}
			break;
		}

		Types *tp = peekType();

		if (tp->isStatic || tp->isConst)
		{
			addError(std::string("无效的类型说明符"));
		}

		std::vector<variable> vs;
		while (true)
		{
			if (data->token.peek(0).isEos())
			{
				if (vs.empty())
				{
					addError(std::string("应该输入成员变量"));
				}
				data->token.read();
				break;
			}

			variable v = getVariables(tp);
			if (!v.type->canInstance() || v.type->isStatic || v.type->isConst)
			{
				addError(std::string("不能添加此类型的成员"));
			}

			vs.push_back(v);
		}

		for (int i = 0; i<vs.size(); ++i)
		{
			for (int j = 0; j < unidef.members.size(); ++j)
			{
				if (unidef.members[j] == vs[i].name)
				{
					addError(std::string("成员重定义"));
				}
			}

			unidef.members.push_back(vs[i].name);

			unidef.size = std::max(vs[i].type->getSize(),unidef.size);
			unidef.types.push_back(vs[i].type);
		}

	}

	requireToken("}");

	Types *type = nullptr;
	Types *constType = nullptr;

	if (!isAnonymous)//更新
	{
		int index = data->allUnionDef.size() - 1;
		data->allUnionDef[index] = unidef;
		it->second.isDefined = true;
		type = it->second.type;
		constType = it->second.constType;
	}

	if (data->token.peek(0).getString() == ";")
	{
		data->token.read();
		return;
	}

	if (isAnonymous)
	{
		data->allUnionDef.push_back(unidef);
		UnionRef tpr = std::make_unique<UnionType>();
		tpr->name = name;
		tpr->unionDef = data->allUnionDef.size() - 1;

		UnionRef tprc = std::make_unique<UnionType>();
		tprc->name = name;
		tprc->unionDef = tpr->unionDef;
		tprc->isConst = true;

		type = tpr.get();
		constType = tprc.get();

		data->allTypes.push_back(std::move(tpr));//类型
		data->allTypes.push_back(std::move(tprc));//常类型
	}


	//全局变量的解析
	if (isTypedef)
	{
		getUnionTypedef(isStatic, isConst, type, constType, name);
	}
	else
	{
		getUnionVariable(isStatic, isConst, type, constType);
	}

	requireToken(";");
}

void Parser::getEnum(bool isTypedef)
{
	bool isStatic = false;
	bool isConst = false;
	bool isAnonymous = false;
	EnumDef enumDef;
	std::string name;

	while (true)
	{
		const std::string &str = data->token.peek(0).getString();
		data->token.read();

		if (str == "enum")
		{
			break;
		}
		if (str == "static")
		{
			if (isStatic == false)
			{
				isStatic = true;
			}
			else
			{
				addError(std::string("无效的类型组合说明符"));
			}
		}
		else if (str == "const")
		{
			if (isConst == false)
			{
				isConst = true;
			}
			else
			{
				addError(std::string("无效的类型组合说明符"));
			}
		}
	}

	auto &token = data->token.peek(0);

	if (token.getString() == "{")
	{
		isAnonymous = true;
	}
	else if (token.isIdentifier() && !token.isKeyWord())
	{
		name = token.getString();
		data->token.read();
	}
	else
	{
		addError(std::string("应输入标识符"));
	}

	enumDef.name = name;
	data->allEnumDef.push_back(enumDef);

	EnumRef type = std::make_unique<EnumType>();
	EnumRef typec = std::make_unique<EnumType>();

	type->enumDef = data->allEnumDef.size()-1;
	typec->enumDef = type->enumDef;
	typec->isConst = true;

	if (!isAnonymous)
	{
		symbol t;
		t.isDefined = true;
		t.isVariable = false;
		t.offSet = -1;
		t.type = type.get();
		t.constType = typec.get();

		auto it=data->symbols[0].find(name);
		if (it != data->symbols[0].end())
		{
			addError(std::string("重定义标识符"));
		}

		data->symbols[0].insert(std::pair<std::string, symbol>(name, t));
	}

	Types *tp = type.get();
	Types *tpc = typec.get();

	data->allTypes.push_back(std::move(type));
	data->allTypes.push_back(std::move(typec));

	requireToken("{");

	int size = 0;
	int lastValue = -1;

	while (true)
	{
		if (data->token.peek(0).getString() == "}")
		{
			if (size == 0)
			{
				addError(std::string("应输入枚举类型"));
			}
			break;
		}

		auto &token = data->token.read();
		if (token.isIdentifier() && !token.isKeyWord())
		{
			const std::string &str = token.getString();
			auto it = data->symbols[0].find(str);
			if (it != data->symbols[0].end())
			{
				addError(std::string("重定义标识符"));
			}

			VariableValue v = getConstIni(data->allTypes[10].get());
			if (!v.buffer)
			{
				v.buffer = new char[sizeof(int)];
				*(int *)v.buffer = lastValue+1;
			}
			else
			{
				lastValue = *(int *)v.buffer;
			}

			symbol t;
			t.isDefined = true;
			t.isVariable = true;
			t.type = tp;
			t.offSet = data->staticRegion.currentOffset;
			data->staticRegion.addValue(v);
			data->symbols[1].insert(std::pair<std::string,symbol>(str,t));

			//Value val;
			//val.isDefined = true;
			//val.offSet = t.offSet;
			//val.type = t.type;
			//data->globalRegion.values.insert(std::pair<std::string,Value>(str,val));

			v.release();
		}
		else
		{
			addError(std::string("应该输入正确的标识符"));
		}

		auto &str = data->token.peek(0).getString();

		if (str =="," )
		{
			data->token.read();
		}
		else
		{
			break;
		}

	}

	requireToken("}");

	if (isTypedef)
	{
		getEnumTypedef(isStatic, isConst, tp, tpc, name);
	}
	else
	{
		getEnumVariable(isStatic, isConst, tp, tpc);
	}

	requireToken(";");
}

void Parser::parseTypedef()
{
	/*检查是不是struct union enum*/
	bool isStruct=false;
	bool isUnion = false;
	bool isEnum = false;

	data->token.read();

	for (int i = 0;; ++i)
	{
		auto &token = data->token.peek(i);
		if (!token.isKeyWord())
		{
			break;
		}

		if (token.getString() == "struct")
		{
			if (data->token.peek(i + 1).getString() == "{" || data->token.peek(i + 2).getString() == "{")
			{
				isStruct = true;
			}
			break;
		}
		else if (token.getString() == "union")
		{
			if (data->token.peek(i + 1).getString() == "{" || data->token.peek(i + 2).getString() == "{")
			{
				isStruct = true;
			}
			break;
		}
		else if (token.getString() == "enum")
		{
			if (data->token.peek(i + 1).getString() == "{" || data->token.peek(i + 2).getString() == "{")
			{
				isStruct = true;
			}
			break;
		}
	}

	if (isStruct)
	{
		getStruct(true);
		return;
	}

	if (isUnion)
	{
		getUnion(true);
		return;
	}

	if (isEnum)
	{
		getEnum(true);
		return;
	}

	Types *tp = peekType();

	std::vector<variable> vs;
	while (true)
	{
		if (data->token.peek(0).isEos())
		{
			if (vs.empty())
			{
				addError(std::string("应输入标识符"));
			}
			data->token.read();
			break;
		}

		variable v = getVariables(tp);
		vs.push_back(v);
	}

	for (int i = 0; i <vs.size(); ++i)
	{
		auto it = data->symbols[0].find(vs[i].name);
		if (it != data->symbols[0].end())
		{
			addError(std::string("重定义的标识符"));
		}

		TypeRef tprc = vs[i].type->copy();
		tprc->isConst = true;

		symbol sbl;
		sbl.offSet = -1;
		sbl.isDefined = true;
		sbl.offSet = -1;
		sbl.isVariable = false;
		sbl.type = vs[i].type;
		sbl.constType = tprc.get();
		data->allTypes.push_back(std::move(tprc));

		data->symbols[0].insert(std::pair<std::string,symbol>(vs[i].name,sbl));
	}

}

void Parser::parseVariable()
{
	bool isExtern = false;

	if (data->token.peek(0).getString() == "extern")
	{
		data->token.read();
		isExtern = true;
	}

	Types *tp = peekType();

	if (tp->isStatic&&isExtern)
	{
		addError(std::string("不能声明多个储存符"));
	}

	while (true)
	{
		variable v = getVariables(tp);
		if (!v.type->isFunction())
		{
			if (isExtern)
			{
				auto it = data->globalRegion.values.find(v.name);
				/*首先检查符号表中是否已经有这个符号*/

				if (it != data->globalRegion.values.end())
				{
					if (it->second.isDefined || !it->second.type->equal(v.type))
					{
						addError(std::string("重定义的标识符"));
					}

					if (data->symbols[0].find(v.name) == data->symbols[0].end())
					{
						symbol t;
						t.type = v.type;
						t.isDefined = it->second.isDefined;
						t.isVariable = true;
						t.offSet = it->second.offSet;
						data->symbols[0].insert(std::pair<std::string, symbol>(v.name, t));
					}

				}
				else
				{
					symbol t;
					t.type = v.type;
					t.isDefined = false;
					t.isVariable = true;
					t.offSet = data->globalRegion.currentOffset;

					VariableValue vv;
					vv.type = t.type;
					vv.buffer = new char[t.type->getSize()];
					memset(vv.buffer, 0, t.type->getSize());

					Value val;
					val.isDefined = false;
					val.offSet = t.offSet;
					val.type = t.type;

					data->globalRegion.addValue(vv);
					data->globalRegion.values.insert(std::pair<std::string, Value>(v.name, val));
					data->symbols[0].insert(std::pair<std::string, symbol>(v.name, t));

					vv.release();
				}
			}
			else
			{
				int index = tp->isStatic ? 1 : 0;
				Region *region;
				if (tp->isStatic)
				{
					region = &data->staticRegion;
				}
				else
				{
					region = &data->globalRegion;
				}

				auto it = region->values.find(v.name);

				/*首先检查符号表中是否已经有这个符号*/
				if (it != region->values.end())
				{
					if (it->second.isDefined || !it->second.type->equal(v.type))
					{
						addError(std::string("重定义的标识符"));
					}
				}
				
				auto it2 = data->symbols[index].find(v.name);
				if (it2 != data->symbols[index].end())
				{
					if (it2->second.isDefined || !it2->second.type->equal(v.type))
					{
						addError(std::string("重定义的标识符"));
					}
				}

				VariableValue vv = getConstIni(tp);
				if (vv.buffer == nullptr)
				{
					vv.buffer = new char[tp->getSize()];
					memset(vv.buffer, 0, tp->getSize());
				}

				if (!tp->isStatic)
				{
					symbol t;
					t.type = v.type;
					t.isDefined = true;
					t.isVariable = true;
					if (it != region->values.end())
					{
						t.offSet = it->second.offSet;
					}
					else
					{
						t.offSet = region->currentOffset;
					}

					Value val;
					val.isDefined = true;
					val.offSet = t.offSet;
					val.type = t.type;

					if (it != region->values.end())
					{
						memcpy(region->data + it->second.offSet, vv.buffer, vv.type->getSize());
					}
					else
					{
						region->addValue(vv);
						region->values.insert(std::pair<std::string, Value>(v.name, val));
					}

					if (it2 == data->symbols[index].end())
					{
						data->symbols[index].insert(std::pair<std::string, symbol>(v.name, t));
					}

				}
				else
				{
					symbol t;
					t.type = v.type;
					t.isDefined = true;
					t.isVariable = true;
					t.offSet = region->currentOffset;
					region->addValue(vv);

					data->symbols[index].insert(std::pair<std::string, symbol>(v.name, t));
				}

				vv.release();
			}
		}
		else
		{
			int index;
			Region *region;

			if (v.type->isStatic)
			{
				index = 1;
				region = &data->staticRegion;
			}
			else
			{
				index = 0;
				region = &data->globalRegion;
			}

			bool isDef = data->token.peek(0).getString() == "{";
			auto it = data->symbols[index].find(v.name);

			if (it != data->symbols[index].end())
			{
				if (it->second.isDefined)
				{
					addError(std::string("重定义的标识符"));
				}
				else if (!it->second.type->equal(v.type))
				{
					addError(std::string("重定义的标识符，不同的类型"));
				}

				if (isDef)
				{
					FunctionDef *fun = *(FunctionDef **)(region->data + it->second.offSet);
					data->currentFunction = fun;
					getFunctionDef();
					if (!v.type->isStatic)
					{
						region->values.find(v.name)->second.isDefined = true;
					}
					return;
				}

			}
			else
			{
				symbol t;
				t.type = v.type;
				t.isDefined = false;
				t.isVariable = true;
				t.offSet = region->currentOffset;

				Value val;
				val.isDefined = false;
				val.offSet = t.offSet;
				val.type = t.type;

				FunctionDefRef foo = std::make_unique<FunctionDef>();
				foo->setType(t.type);
				FunctionDef *fptr = foo.get();
				data->allFunctions.push_back(std::move(foo));

				region->values.insert(std::pair<std::string, Value>(v.name, val));
				data->symbols[index].insert(std::pair<std::string, symbol>(v.name, t));

				VariableValue vv;
				vv.buffer = new char[sizeof(void *)];
				*(FunctionDef **)vv.buffer = fptr;

				vv.type = t.type;
				region->addValue(vv);
				vv.release();

				if (isDef)
				{
					data->currentFunction = fptr;
					getFunctionDef();
					fptr->setType(v.type);
					if (!v.type->isStatic)
					{
						region->values.find(v.name)->second.isDefined = true;
					}
					return;
				}
			}

		}

		if (data->token.peek(0).getString() == ",")
		{
			data->token.read();
		}
		else
		{
			requireToken(";");
			return;
		}

	}
}

void Parser::addError(std::string & info)
{
	throw std::to_string(data->token.peek(0).getLine()) +info;
}

void Parser::requireToken(const std::string & token)
{
	if (data->token.peek(0).getString() != token)
	{
		addError(std::string("缺少") + token);
	}

	data->token.read();
}

void Parser::createBaseType()
{
	BasicRef boolean1 = std::make_unique<BasicType>();
	boolean1->size = 1;
	BasicRef boolean2 = std::make_unique<BasicType>();
	boolean2->isConst = true;
	boolean2->size = 1;

	BasicRef sChar1 = std::make_unique<BasicType>();
	sChar1->size = 1;
	BasicRef sChar2 = std::make_unique<BasicType>();
	sChar2->isConst = true;
	sChar2->size = 1;
	BasicRef usChar1 = std::make_unique<BasicType>();
	usChar1->isSigned = false;
	usChar1->size = 1;
	BasicRef usChar2 = std::make_unique<BasicType>();
	usChar2->isConst = true;
	usChar2->isSigned = false;
	usChar2->size = 1;

	BasicRef sShort1 = std::make_unique<BasicType>();
	sShort1->size = 2;
	BasicRef sShort2 = std::make_unique<BasicType>();
	sShort2->isConst = true;
	sShort2->size = 2;
	BasicRef usShort1= std::make_unique<BasicType>();
	usShort1->isSigned = false;
	usShort1->size = 2;
	BasicRef usShort2 = std::make_unique<BasicType>();
	usShort2->isConst = true;
	usShort2->isSigned = false;
	usChar2->size = 2;

	BasicRef sInt1 = std::make_unique<BasicType>();
	sInt1->size = 4;
	BasicRef sInt2 = std::make_unique<BasicType>();
	sInt2->isConst = true;
	sInt2->size = 4;
	BasicRef usInt1 = std::make_unique<BasicType>();
	sInt2->isSigned = false;
	sInt2->size = 4;
	BasicRef usInt2 = std::make_unique<BasicType>();
	usInt2->isConst = true;
	usInt2->isSigned = false;
	usInt2->size = 4;

	BasicRef sLongLong1 = std::make_unique<BasicType>();
	sLongLong1->size = 4;
	BasicRef sLongLong2= std::make_unique<BasicType>();
	sLongLong2->size = 4;
	sLongLong2->isConst = true;
	BasicRef usLongLong1 = std::make_unique<BasicType>();
	usLongLong1->size = 4;
	usLongLong1->isSigned = false;
	BasicRef usLongLong2 = std::make_unique<BasicType>();
	usLongLong2->size = 4;
	usLongLong2->isConst = true;
	usLongLong2->isSigned = false;

	BasicRef float1 = std::make_unique<BasicType>();
	float1->size = 4;
	float1->isFloat = true;
	BasicRef float2 = std::make_unique<BasicType>();
	float2->size = 4;
	float2->isFloat = true;
	float2->isConst = true;

	BasicRef double1 = std::make_unique<BasicType>();
	double1->size = 8;
	double1->isFloat = true;
	BasicRef double2 = std::make_unique<BasicType>();
	double2->size = 8;
	double2->isFloat = true;
	double2->isConst = true;

	VoidRef vod = std::make_unique<VoidType>();

	data->allTypes.push_back(std::move(boolean1));
	data->allTypes.push_back(std::move(boolean2));
	data->allTypes.push_back(std::move(sChar1));
	data->allTypes.push_back(std::move(sChar2));
	data->allTypes.push_back(std::move(usChar1));
	data->allTypes.push_back(std::move(usChar2));
	data->allTypes.push_back(std::move(sShort1));
	data->allTypes.push_back(std::move(sShort2));
	data->allTypes.push_back(std::move(usShort1));
	data->allTypes.push_back(std::move(usShort2));
	data->allTypes.push_back(std::move(sInt1));
	data->allTypes.push_back(std::move(sInt2));
	data->allTypes.push_back(std::move(usInt1));
	data->allTypes.push_back(std::move(usInt2));
	data->allTypes.push_back(std::move(sLongLong1));
	data->allTypes.push_back(std::move(sLongLong2));
	data->allTypes.push_back(std::move(usLongLong1));
	data->allTypes.push_back(std::move(usLongLong2));
	data->allTypes.push_back(std::move(float1));
	data->allTypes.push_back(std::move(float2));
	data->allTypes.push_back(std::move(double1));
	data->allTypes.push_back(std::move(double2));

	data->allTypes.push_back(std::move(vod));

}

void Parser::getFunctionDef()
{
	data->currentOffset = 0;
	data->currentLevel = 2;
	data->currentFunction->setBlock(getBlock(data->currentFunction));
}

void Parser::getStructVariable(bool isStatic, bool isConst, Types * type, Types * constType)
{
	while (true)
	{
		auto &token = data->token.peek(0);
		const std::string &str = token.getString();

		if (str == ";")
		{
			return;
		}

		data->token.read();

		if (str == "*")
		{
			auto &t = data->token.read();

			if (t.isIdentifier() && !t.isKeyWord())
			{
				std::string vname = t.getString();
				auto it1 = data->symbols[0].find(vname);
				auto it2 = data->symbols[1].find(vname);
				bool ok = true;

				if (isStatic)
				{
					if (it1 != data->symbols[0].end() && it1->second.isVariable == false)//是类型名
					{
						ok = false;
					}
					else if (it2 != data->symbols[1].end())
					{
						ok = false;
					}
				}
				else if (it1 != data->symbols[0].end())
				{
					ok = false;
				}

				if (!ok)
				{
					addError(std::string("重定义的标识符"));
				}

				PointerRef ptr = std::make_unique<PointerType>();
				ptr->targetType = isConst ? constType : type;

				VariableValue v = getConstIni(ptr.get());
				if (v.buffer == nullptr)
				{
					v.buffer = new char[ptr->getSize()];
					memset(v.buffer, 0, ptr->getSize());
				}

				Value val;
				val.isDefined = true;
				val.type = ptr.get();

				if (isStatic)
				{
					val.offSet = data->staticRegion.currentOffset;
					if (data->staticRegion.values.find(vname) != data->staticRegion.values.end())
					{
						addError(std::string("重定义的静态变量"));
					}
					//data->staticRegion.values.insert(std::pair<std::string, Value>(vname, val));
					data->staticRegion.addValue(v);
				}
				else
				{
					val.offSet = data->globalRegion.currentOffset;
					if (data->globalRegion.values.find(vname) != data->globalRegion.values.end())
					{
						addError(std::string("重定义的全局变量"));
					}
					data->globalRegion.values.insert(std::pair<std::string, Value>(vname, val));
					data->globalRegion.addValue(v);
				}

				v.release();

				int index = isStatic ? 1 : 0;

				symbol syb;
				syb.isDefined = true;
				syb.isVariable = true;
				syb.offSet = val.offSet;
				syb.type = ptr.get();
				data->symbols[index].insert(std::pair<std::string, symbol>(vname, syb));//往符号表中插入一个全局（或者静态变量）

				data->allTypes.push_back(std::move(ptr));
				if (data->token.peek(0).getString() != ";")
				{
					requireToken(",");
				}
			}
			else
			{
				addError(std::string("应输入正确的标识符"));
			}
		}
		else if (token.isIdentifier() && !token.isKeyWord())//数组，或者变量
		{
			Types *tp;
			bool isArray = false;
			ArrayRef array;
			int index;

			if (data->token.peek(0).getString() == "[")
			{
				data->token.read();
				auto &indexToken = data->token.read();
				if (indexToken.isIntLiteral())
				{
					index = (int)indexToken.getInteger();
				}
				else
				{
					addError(std::string("应该输入数组大小"));
				}

				if (index <= 0)
				{
					addError(std::string("数组大小必须大于0"));
				}
				requireToken("]");

				array = std::make_unique<ArrayType>();
				array->dataType = isConst ? constType : type;
				array->capacity = index;

				tp = array.get();
				isArray = true;
			}
			else
			{
				tp = isConst ? constType : type;
			}

			VariableValue v = getConstIni(tp);
			if (v.buffer == nullptr)
			{
				v.buffer = new char[tp->getSize()];
				memset(v.buffer, 0, tp->getSize());
			}

			Value val;
			val.isDefined = true;
			val.type = tp;

			if (isStatic)
			{
				val.offSet = data->staticRegion.currentOffset;
				val.offSet = data->staticRegion.currentOffset;
				if (data->staticRegion.values.find(str) != data->staticRegion.values.end())
				{
					addError(std::string("重定义的静态变量"));
				}
				//data->staticRegion.values.insert(std::pair<std::string, Value>(str, val));
				data->staticRegion.addValue(v);
			}
			else
			{
				val.offSet = data->globalRegion.currentOffset;
				if (data->globalRegion.values.find(str) != data->globalRegion.values.end())
				{
					addError(std::string("重定义的全局变量"));
				}
				data->globalRegion.values.insert(std::pair<std::string, Value>(str, val));
				data->globalRegion.addValue(v);
			}

			v.release();

			index = isStatic ? 1 : 0;

			symbol syb;
			syb.isDefined = true;
			syb.isVariable = true;
			syb.offSet = val.offSet;
			syb.type = tp;
			data->symbols[index].insert(std::pair<std::string, symbol>(str, syb));//往符号表中插入一个全局（或者静态变量）

			if (isArray)
			{
				data->allTypes.push_back(std::move(array));
			}

			if (data->token.peek(0).getString() != ";")
			{
				requireToken(",");
			}

		}
		else
		{
			addError(std::string("无效的类型声明"));
		}
	}
}

void Parser::getStructTypedef(bool isStatic, bool isConst, Types * type, Types * constType, std::string & name)
{
	if (isStatic)
	{
		addError(std::string("无效的类型组合说明符"));
	}

	while (true)
	{
		auto &token = data->token.peek(0);
		const std::string &str = token.getString();

		if (str == ";")
		{
			return;
		}

		data->token.read();

		if (str == "*")
		{
			auto &t = data->token.read();
			if (t.isIdentifier() && !t.isKeyWord())
			{
				std::string vname = t.getString();
				auto it1 = data->symbols[0].find(vname);

				if (it1 != data->symbols[0].end())
				{
					addError(std::string("重定义的标识符"));
				}

				PointerRef ptr = std::make_unique<PointerType>();
				ptr->targetType = isConst ? constType : type;
				PointerRef ptrc = std::make_unique<PointerType>();
				ptr->targetType = constType;

				symbol syb;
				syb.isDefined = true;
				syb.isVariable = false;
				syb.offSet = -1;
				syb.type = ptr.get();
				syb.constType = ptrc.get();

				data->symbols[0].insert(std::pair<std::string, symbol>(vname, syb));//往符号表中插入一个类型

				data->allTypes.push_back(std::move(ptr));
				data->allTypes.push_back(std::move(ptrc));
				if (data->token.peek(0).getString() != ";")
				{
					requireToken(",");
				}
			}
			else
			{
				addError(std::string("应输入正确的标识符"));
			}
		}
		else if (token.isIdentifier() && !token.isKeyWord())//数组，或者变量
		{
			Types *tp;
			bool isArray = false;
			ArrayRef array;
			int index;

			if (data->token.peek(0).getString() == "[")
			{
				data->token.read();
				auto &indexToken = data->token.read();
				if (indexToken.isIntLiteral())
				{
					index = (int)indexToken.getInteger();
				}
				else
				{
					addError(std::string("应该输入数组大小"));
				}

				if (index <= 0)
				{
					addError(std::string("数组大小必须大于0"));
				}
				requireToken("]");

				array = std::make_unique<ArrayType>();
				array->dataType = isConst ? constType : type;
				array->capacity = index;

				tp = array.get();
				isArray = true;
			}
			else
			{
				tp = isConst ? constType : type;
			}

			if (!isArray)
			{
				if (str == name)
				{
					continue;
				}

				symbol syb;
				syb.isDefined = true;
				syb.isVariable = false;
				syb.offSet = -1;
				syb.type = tp;
				syb.constType = constType;
				
				data->symbols[0].insert(std::pair<std::string, symbol>(str, syb));//往符号表中插入一个全局（或者静态变量）

				if (data->token.peek(0).getString() != ";")
				{
					requireToken(",");
				}
			}
			else
			{
				symbol syb;
				syb.isDefined = true;
				syb.isVariable = false;
				syb.offSet = -1;
				syb.type = tp;
				ArrayRef arrayc = std::make_unique<ArrayType>();
				arrayc->dataType = constType;
				arrayc->capacity = index;
				syb.constType = arrayc.get();

				data->symbols[0].insert(std::pair<std::string, symbol>(str, syb));//往符号表中插入一个全局（或者静态变量）

				data->allTypes.push_back(std::move(array));
				data->allTypes.push_back(std::move(arrayc));

				if (data->token.peek(0).getString() != ";")
				{
					requireToken(",");
				}
			}

		}
		else
		{
			addError(std::string("无效的类型声明"));
		}
	}
}

void Parser::getUnionVariable(bool isStatic, bool isConst, Types * type, Types * constType)
{
	getStructVariable(isStatic, isConst, type, constType);
}

void Parser::getUnionTypedef(bool isStatic, bool isConst, Types * type, Types * constType, std::string & name)
{
	getStructTypedef(isStatic, isConst, type, constType, name);
}

void Parser::getEnumVariable(bool isStatic, bool isConst, Types * type, Types * constType)
{
	getStructVariable(isStatic, isConst, type, constType);
}

void Parser::getEnumTypedef(bool isStatic, bool isConst, Types * type, Types * constType, std::string & name)
{
	getStructTypedef(isStatic, isConst, type, constType, name);
}

void * Parser::getStackRegion()
{
	return data->stack;
}

void * Parser::getStaticRegion()
{
	return data->staticRegion.data;
}

void * Parser::getGlobalRegion()
{
	return data->globalRegion.data;
}

AStreeRef Parser::getAtomic(AStree * block)
{
	AStreeRef neg;
	if (data->token.peek(0).getString() == "(")
	{
		requireToken("(");
		neg= getExpr(block);
		requireToken(")");
	}
	else if (data->token.peek(0).getString() == "sizeof")
	{
		data->token.read();
		neg = std::make_unique<SizeOfExpr>();
		neg->block = block;
		neg->function = data->currentFunction;

		SizeOfExpr *soe = neg->toSizeOf();
		requireToken("(");
		
		if (data->token.peek(0).isKeyWord() || !findSymbol(data->token.peek(0).getString())->isVariable)
		{
			Types *pre = peekType();
			variable v = getVariables(pre, true);
			if (!v.type->canInstance())
			{
				addError(std::string("不允许使用不完整的类型"));
			}
			soe->size=v.type->getSize();
		}
		else
		{
			soe->size = getExpr(block)->getType()->getSize();
		}

		requireToken(")");
		soe->thisType = data->allTypes[10].get();
	}
	else if(data->token.peek(0).isIntLiteral())
	{
		neg = std::make_unique<IntLiteralExpr>();
		neg->block = block;
		neg->function = data->currentFunction;

		IntLiteralExpr *ile = neg->toIntLiteral();
		ile->value = data->token.read().getInteger();
		ile->thisType = data->allTypes[16].get();
	}
	else if (data->token.peek(0).isRealLiteral())
	{
		neg = std::make_unique<RealLiteralExpr>();
		neg->block = block;
		neg->function = data->currentFunction;

		RealLiteralExpr *ile = neg->toRealLiteral();
		ile->value = data->token.read().getInteger();
		ile->thisType = data->allTypes[16].get();
	}
	else  if (data->token.peek(0).isStringLiteral())
	{
		neg = getStringLiteral(block);
	}
	else if (data->token.peek(0).isIdentifier())
	{
		if (data->token.peek(0).isKeyWord())
		{
			addError(std::string("不能使用关键字作为标识符"));
		}

		symbol *t = nullptr;

		const std::string &str = data->token.read().getString();
		auto it = data->symbols[0].end();

		/*查找局部变量*/
		for (int i = data->currentLevel; i >= 2; --i)
		{
			it = data->symbols[i].find(str);
			if (it != data->symbols[i].end())
			{
				t = &it->second;
				break;
			}
		}

		bool isStatic = false;
		bool isGlobal = false;
		bool isArg = false;

		if (!t)
		{
			it = data->symbols[1].find(str);
			if (it != data->symbols[1].end())
			{
				isStatic = true;
				t = &it->second;
			}
		}

		if (!t)
		{
			it = data->symbols[0].find(str);
			if (it != data->symbols[0].end())
			{
				isGlobal = true;
				t = &it->second;
			}
		}

		if (!isStatic && !isGlobal)
		{
			FunctionType *tp = data->currentFunction->getType()->toFunction();
			
			for (int i = 0; i < tp->args.size(); ++i)
			{
				if (tp->args[i] == str)
				{
					isArg = true;
					break;
				}
			}

		}

		if (!t)
		{
			addError(std::string("使用了未定义的标识符"));
		}

		if (!t->isVariable)
		{
			addError(std::string("不能使用类型名"));
		}

		neg = std::make_unique<IdExpr>();
		neg->block = block;
		neg->function = data->currentFunction;
		IdExpr *ie = neg->toIdExpr();
		ie->thisType = t->type;
		ie->offset = t->offSet;

		if (isStatic)
		{
			ie->reg = StaticVariable;
		}
		else if (isGlobal)
		{
			ie->reg = GlobalVariable;
		}
		else if (isArg)
		{
			ie->reg = Arg;
		}
		else
		{
			ie->reg = Local;
		}

	}

	while (true)
	{
		if (data->token.peek(0).getString() == "++")
		{
			if (!(neg->getType()->isBasic() || neg->getType()->isPointer()) || neg->getType()->isConst || !neg->isLeftValue())
			{
				addError(std::string("自减运算符只能用于可修改的左值"));
			}

			data->token.read();
			AStreeRef poe = std::make_unique<PostIncExpr>();
			poe->block = block;
			poe->function = data->currentFunction;
			PostIncExpr *n = poe->toPostInc();
			n->target = std::move(neg);

			if (n->target->getType()->isPointer())
			{
				if (!n->target->getType()->toPointer()->targetType->canInstance())
				{
					addError(std::string("指针必须指向完整的类型"));
				}
			}

			neg = std::move(poe);
		}
		else if (data->token.peek(0).getString() == "--")
		{
			if (!(neg->getType()->isBasic() || neg->getType()->isPointer()) || neg->getType()->isConst || !neg->isLeftValue())
			{
				addError(std::string("自减运算符只能用于可修改的左值"));
			}

			data->token.read();
			AStreeRef poe = std::make_unique<PostDecExpr>();
			poe->block = block;
			poe->function = data->currentFunction;
			PostDecExpr *n = poe->toPostDec();
			n->target = std::move(neg);

			if (n->target->getType()->isPointer())
			{
				if (!n->target->getType()->toPointer()->targetType->canInstance())
				{
					addError(std::string("指针必须指向完整的类型"));
				}
			}

			neg = std::move(poe);
		}
		else if(data->token.peek(0).getString() == "[")
		{
			if (!neg->getType()->isPointer() || !neg->getType()->isArray())
			{
				addError(std::string("必须具有指针或数组类型"));
			}

			Types *tp;
			if (neg->getType()->isPointer())
			{
				tp=neg->getType()->toPointer()->targetType;
			}
			else
			{
				tp=neg->getType()->toArray()->dataType;
			}

			if (!tp->canInstance())
			{
				addError(std::string("不允许使用不完整的类型"));
			}

			AStreeRef poe = std::make_unique<ArrayAccessExpr>();
			poe->block = block;
			poe->function = data->currentFunction;
			ArrayAccessExpr *n = poe->toArrayAccess();
			n->addr = std::move(neg);

			requireToken("[");
			n->index = getExpr(block);
			if (!n->index->getType()->isBasic() || n->index->getType()->toBasic()->isFloat)
			{
				addError(std::string("数组下标必须具有整数类型"));
			}
			requireToken("]");

			neg = std::move(poe);
		}
		else if (data->token.peek(0).getString() == ".")
		{
			if (!neg->getType()->isStruct()&&!neg->getType()->isUnion())
			{
				addError(std::string(".运算符必须具有结构或联合体类型"));
			}
			requireToken(".");

			if (!data->token.peek(0).isIdentifier() || data->token.peek(0).isKeyWord())
			{
				addError(std::string("应输入正确的标识符"));
			}

			const std::string &name = data->token.read().getString();

			Types *tp;
			int off;

			if (neg->getType()->isStruct())
			{
				auto stru = neg->getType()->toStruct();
				int index = -1;

				auto &strdef = data->allStructDef[stru->structDef];

				for (int i = 0; i <strdef.members.size(); ++i)
				{
					if (strdef.members[i] == name)
					{
						index = i;
						break;
					}
				}

				if (index == -1)
				{
					addError(std::string("没有成员")+name);
				}

				tp = strdef.types[index];
				off = strdef.offSets[index];
			}
			else
			{
				auto uni = neg->getType()->toUnion();
				int index = -1;

				auto &unidef = data->allStructDef[uni->unionDef];

				for (int i = 0; i <unidef.members.size(); ++i)
				{
					if (unidef.members[i] == name)
					{
						index = i;
						break;
					}
				}

				if (index == -1)
				{
					addError(std::string("没有成员") + name);
				}


				tp = unidef.types[index];
				off = 0;
			}

			AStreeRef mem = std::make_unique<MemberAccessExpr>();
			mem->function = data->currentFunction;
			mem->block = block;
			auto memac = mem->toMemberAccess();
			memac->thisType = tp;
			memac->offset = off;
			memac->target = std::move(neg);

			neg = std::move(mem);
		}
		else if (data->token.peek(0).getString() == "->")
		{
			if (!neg->getType()->isPointer())
			{
				addError(std::string("必须具有指针类型"));
			}

			auto target = neg->getType()->toPointer()->targetType;

			if (!target->isStruct() && !target->isUnion())
			{
				addError(std::string(".运算符必须具有结构或联合体类型"));
			}

			requireToken("->");

			if (!data->token.peek(0).isIdentifier() || data->token.peek(0).isKeyWord())
			{
				addError(std::string("应输入正确的标识符"));
			}

			const std::string &name = data->token.read().getString();

			Types *tp;
			int off;

			if (target->isStruct())
			{
				auto stru = target->toStruct();
				int index = -1;

				auto &strdef = data->allStructDef[stru->structDef];

				for (int i = 0; i <strdef.members.size(); ++i)
				{
					if (strdef.members[i] == name)
					{
						index = i;
						break;
					}
				}

				if (index == -1)
				{
					addError(std::string("没有成员") + name);
				}

				tp = strdef.types[index];
				off = strdef.offSets[index];
			}
			else
			{
				auto uni = target->toUnion();
				int index = -1;

				auto &unidef = data->allStructDef[uni->unionDef];

				for (int i = 0; i <unidef.members.size(); ++i)
				{
					if (unidef.members[i] == name)
					{
						index = i;
						break;
					}
				}

				if (index == -1)
				{
					addError(std::string("没有成员") + name);
				}

				tp = unidef.types[index];
				off = 0;
			}

			AStreeRef mem = std::make_unique<MemberAccessPtr>();
			mem->function = data->currentFunction;
			mem->block = block;
			auto memac = mem->toMemberAccessPtr();
			memac->thisType = tp;
			memac->offset = off;
			memac->target = std::move(neg);

			neg = std::move(mem);
		}
		else if (data->token.peek(0).getString() == "(")
		{
			if (!neg->getType()->isFunction()&&!(neg->getType()->isPointer()&& neg->getType()->toPointer()->targetType->isFunction()))
			{
				addError(std::string("必须具有函数或函数指针类型"));
			}

			Types *r;
			FunctionType *fun;

			if (neg->getType()->isFunction())
			{
				fun = neg->getType()->toFunction();
				r =fun->returnType;
			}
			else
			{
				fun = neg->getType()->toPointer()->targetType->toFunction();
				r=fun->returnType;
			}

			AStreeRef mem = std::make_unique<FuncallExpr>();
			mem->function = data->currentFunction;
			mem->block = block;

			auto *foo = mem->toFuncall();
			foo->target = std::move(neg);
			foo->thisType = r;

			requireToken("(");
			bool more = false;

			for (int i = 0;; ++i)
			{
				AStreeRef arg = getExpr(block,false);

				if (fun->argsType[i] == nullptr)
				{
					more = true;
					break;
				}

				if (!fun->argsType[i]->compatible(arg->getType()))
				{
					addError(std::string("应输入正确的参数类型"));
				}

				foo->args.push_back(std::move(arg));

				if (data->token.peek(0).getString() == ",")
				{
					requireToken(",");
				}
				else
				{
					break;
				}
			}

			if (more)
			{
				while(true)
				{
					AStreeRef arg = getExpr(block, false);
					foo->args.push_back(std::move(arg));

					if (data->token.peek(0).getString() == ",")
					{
						requireToken(",");
					}
					else
					{
						break;
					}
				}
			}

			requireToken(")");
		}
		else
		{
			break;
		}
	}

	return neg;
}

AStreeRef Parser::getPrimary(AStree *block)
{
	//正号不起作用
	if (data->token.peek(0).getString() == "+")
	{
		data->token.read();
		return getPrimary(block);
	}

	if (data->token.peek(0).getString() == "-")
	{
		data->token.read();
		AStreeRef neg = std::make_unique<NegativeExpr>();
		neg->block = block;
		neg->function = data->currentFunction;
		NegativeExpr *n = neg->toNegative();
		n->target = getPrimary(block);
		if (!n->target->getType()->isBasic())
		{
			addError(std::string("负号后面必须具有数值类型"));
		}

		return neg;
	}

	if (data->token.peek(0).getString() == "++")
	{
		data->token.read();
		AStreeRef neg = std::make_unique<PreIncExpr>();
		neg->block = block;
		neg->function = data->currentFunction;
		PreIncExpr *n = neg->toPreInc();
		n->target = getPrimary(block);
		if (!(n->target->getType()->isBasic()||n->target->getType()->isPointer())||n->target->getType()->isConst||!n->target->isLeftValue())
		{
			addError(std::string("自增运算符只能用于可修改的左值"));
		}

		if (n->target->getType()->isPointer())
		{
			if (!n->target->getType()->toPointer()->targetType->canInstance())
			{
				addError(std::string("指针必须指向完整的类型"));
			}
		}

		return neg;
	}

	if (data->token.peek(0).getString() == "--")
	{
		data->token.read();
		AStreeRef neg = std::make_unique<PreDecExpr>();
		neg->block = block;
		neg->function = data->currentFunction;
		PreDecExpr *n = neg->toPreDec();
		n->target = getPrimary(block);
		if (!(n->target->getType()->isBasic() || n->target->getType()->isPointer()) || n->target->getType()->isConst || !n->target->isLeftValue())
		{
			addError(std::string("自减运算符只能用于可修改的左值"));
		}

		if (n->target->getType()->isPointer())
		{
			if (!n->target->getType()->toPointer()->targetType->canInstance())
			{
				addError(std::string("指针必须指向完整的类型"));
			}
		}

		return neg;
	}

	if (data->token.peek(0).getString() == "*")
	{
		data->token.read();
		AStreeRef neg = std::make_unique<GetValueExpr>();
		neg->block = block;
		neg->function = data->currentFunction;
		GetValueExpr *n = neg->toGetValue();
		n->target = getPrimary(block);
		if (!n->target->getType()->isPointer()&&!n->target->getType()->isArray())
		{
			addError(std::string("表达式必须具有指针类型"));
		}

		if (n->target->getType()->isPointer())
		{
			if (!n->target->getType()->toPointer()->targetType->canInstance())
			{
				addError(std::string("指针必须指向完整的类型"));
			}
		}

		return neg;
	}

	if (data->token.peek(0).getString() == "&")
	{
		data->token.read();
		AStreeRef neg = std::make_unique<GetAddrExpr>();
		neg->block = block;
		neg->function = data->currentFunction;
		GetAddrExpr *n = neg->toGetAddr();
		n->target = getPrimary(block);
		
		PointerRef ptr = std::make_unique<PointerType>();
		ptr->targetType = n->target->getType();
		n->thistype = ptr.get();
		data->allTypes.push_back(std::move(ptr));

		if (!n->target->isLeftValue())
		{
			addError(std::string("&运算符只能用于左值"));
		}

		return neg;
	}

	if (data->token.peek(0).getString() == "~")
	{
		data->token.read();
		AStreeRef neg = std::make_unique<PreIncExpr>();
		neg->block = block;
		neg->function = data->currentFunction;
		PreIncExpr *n = neg->toPreInc();
		n->target = getPrimary(block);
		if (!n->target->getType()->isBasic()||n->target->getType()->toBasic()->isFloat)
		{
			addError(std::string("~运算符只能用于整数"));
		}

		return neg;
	}

	/*类型转换*/
	if (data->token.peek(0).getString() == "("&&data->token.peek(1).isKeyWord() || !findSymbol(data->token.peek(1).getString())->isVariable)
	{
		requireToken(")");
		Types *pre = peekType();
		variable v = getVariables(pre, true);
		requireToken(")");
		AStreeRef neg = std::make_unique<TypeTranExpr>();
		neg->block = block;
		neg->function = data->currentFunction;
		TypeTranExpr *n = neg->toTypeTran();
		n->targetType = v.type;
		n->target = getPrimary(block);

		return neg;
	}

	return getAtomic(block);
}

AStreeRef Parser::getStatement(AStree * block)
{
	if (data->token.peek(0).getString() == ";")
	{
		AStreeRef emp = std::make_unique<EmptyState>();
		emp->block = block;
		emp->function = data->currentFunction;
		return emp;
	}
	else if (data->token.peek(0).getString() == "return")
	{
		return getReturnState(block);
	}
	else if (data->token.peek(0).getString() == "break")
	{
		return getBreakState(block);
	}
	else if (data->token.peek(0).getString() == "for")
	{
		return getForState(block);
	}
	else if (data->token.peek(0).getString() == "while")
	{
		return getWhileState(block);
	}
	else if (data->token.peek(0).getString() == "if")
	{
		return getIfState(block);
	}
	else if (data->token.peek(0).getString() == "do")
	{
		return getDoWhileState(block);
	}
	else if (data->token.peek(0).getString() == "switch")
	{
		return getSwitchState(block);
	}
	else if (data->token.peek(0).getString() == "{")
	{
		return getBlock(data->currentFunction,block);
	}
	else if(data->token.peek(0).isKeyWord()||!findSymbol(data->token.peek(0).getString())->isVariable)
	{
		return getVariableDefState(block);
	}

	return getExprState(block);
}

AStreeRef Parser::getVariableDefState(AStree * block)
{
	Types *tp = peekType();

	AStreeRef rs = std::make_unique<VariableDefState>();
	rs->function = data->currentFunction;
	rs->block = block;
	VariableDefState *vds = rs->toVariableDefState();

	bool one = false;

	while (true)
	{
		if (data->token.peek(0).isEos())
		{
			if (!one)
			{
				addError(std::string("应输入标识符"));
			}
			break;
		}

		variable v = getVariables(tp);

		if (v.type->isFunction())
		{
			addError(std::string("不允许在作用域内声明/定义函数"));
		}

		if (data->symbols[data->currentLevel].find(v.name) != data->symbols[0].end())
		{
			addError(std::string("重定义的标识符"));
		}

		symbol t;
		if (tp->isStatic)//加入静态区间
		{
			t.offSet = data->staticRegion.currentOffset;
			VariableValue vv;
			vv.type = v.type;
			vv.buffer = new char[v.type->getSize()];
			data->staticRegion.addValue(vv);
			vv.release();
		}
		else
		{
			t.offSet = data->currentOffset;
			data->currentOffset += v.type->getSize();
		}

		t.isDefined = true;
		t.isVariable = true;
		t.isStatic = tp->isStatic;
		t.type = tp;

		data->symbols[data->currentLevel].insert(std::pair<std::string, symbol>(v.name, t));

		AStreeRef id = std::make_unique<IdExpr>();
		id->function = data->currentFunction;
		id->block = block;
		IdExpr *ie = id->toIdExpr();
		ie->offset = t.offSet;

		if (t.isStatic)
		{
			ie->reg = StaticVariable;
		}
		else
		{
			ie->reg = Local;
		}

		vds->id.push_back(std::move(id));
		vds->value.push_back(getIni(t.type, block));

		one = true;

		if (data->token.peek(0).isEos())
		{
			requireToken(",");
		}
	}

	requireToken(";");

	return rs;
}

AStreeRef Parser::getExprState(AStree * block)
{
	AStreeRef expr = getExpr(block);
	requireToken(";");
	return expr;
}

AStreeRef Parser::getExpr(AStree * block,bool comma)
{
	return AStreeRef();
}

AStreeRef Parser::getSwitchState(AStree * block)
{
	int offset = data->currentOffset;
	++data->currentLevel;
	if (data->symbols.size() < data->currentLevel)
	{
		data->symbols.resize(data->currentLevel);
	}
	data->symbols[data->currentLevel].clear();

	AStreeRef rs = std::make_unique<SwitchState>();
	rs->function = data->currentFunction;
	rs->block = block;
	SwitchState *sw = rs->toSwitchState();

	requireToken("switch");
	requireToken("(");
	sw->target = getExpr(block);
	requireToken(")");
	requireToken("{");

	Types *tp = sw->target->getType();

	bool isDefault = false;

	while (data->token.peek(0).getString() != "}")
	{
		bool def = false;

		if (data->token.peek(0).getString()=="default")
		{
			if (isDefault)
			{
				addError(std::string("switch语句中已经出现了default"));
			}
			isDefault = true;
			def = true;
		}

		if (def)
		{
			requireToken("default");
			requireToken(":");

			sw->conditions.push_back(nullptr);
		}
		else
		{
			requireToken("case");
			AStreeRef t = getExpr(sw);
			if (!t->getType()->equal(tp))
			{
				addError(std::string("case后的表达式类型与switch不符"));
			}

			sw->conditions.push_back(std::move(t));
		}

		sw->states.push_back(std::vector<AStreeRef>());

		int index = sw->states.size() - 1;

		while (true)
		{
			const std::string &t = data->token.peek(0).getString();
			if (t == "default" || t == "case" || t == "}")
			{
				break;
			}

			sw->states[index].push_back(getStatement(sw));
		}

	}

	requireToken("}");
	--data->currentLevel;
	data->currentOffset = offset;

	return rs;
}

AStreeRef Parser::getIfState(AStree * block)
{
	requireToken("if");
	AStreeRef rs = std::make_unique<IfState>();
	rs->function = data->currentFunction;
	rs->block = block;
	IfState *ifs = rs->toIfState();
	
	requireToken("(");
	AStreeRef con = getExpr(block);
	Types *tp = con->getType();
	if (tp == nullptr || (!tp->isBasic() && !tp->isPointer()))
	{
		addError(std::string("表达式必须为bool类型"));
	}
	requireToken(")");

	ifs->condition = std::move(con);
	ifs->conTrue = getExpr(block);
	if (data->token.peek(0).getString() == "else")
	{
		data->token.read();
		ifs->conFalse = getExpr(block);
	}

	return rs;
}

AStreeRef Parser::getWhileState(AStree * block)
{
	requireToken("while");
	AStreeRef rs = std::make_unique<WhileState>();
	rs->function = data->currentFunction;
	rs->block = block;
	WhileState *ws = rs->toWhileState();

	requireToken("(");
	AStreeRef con = getExpr(block);
	Types *tp = con->getType();
	if (tp == nullptr || (!tp->isBasic() && !tp->isPointer()))
	{
		addError(std::string("表达式必须为bool类型"));
	}
	requireToken(")");

	ws->condition = std::move(con);
	ws->state = getStatement(block);
	if (ws->state->isBlock())
	{
		ws->state->toBlock()->type = LoopBlock;
	}

	return rs;
}

AStreeRef Parser::getDoWhileState(AStree * block)
{
	requireToken("do");
	AStreeRef rs = std::make_unique<DoWhileState>();
	rs->function = data->currentFunction;
	rs->block = block;
	DoWhileState *ws = rs->toDoWhileState();

	ws->state = getStatement(block);
	if (ws->state->isBlock())
	{
		ws->state->toBlock()->type = LoopBlock;
	}

	requireToken("while");
	requireToken("(");
	AStreeRef con = getExpr(block);
	Types *tp = con->getType();
	if (tp == nullptr || (!tp->isBasic() && !tp->isPointer()))
	{
		addError(std::string("表达式必须为bool类型"));
	}
	requireToken(")");
	requireToken(";");

	ws->condition = std::move(con);
	return rs;
}

AStreeRef Parser::getForState(AStree * block)
{
	int offset = data->currentOffset;
	++data->currentLevel;
	if (data->symbols.size() < data->currentLevel)
	{
		data->symbols.resize(data->currentLevel);
	}
	data->symbols[data->currentLevel].clear();

	AStreeRef rs = std::make_unique<ForState>();
	rs->function = data->currentFunction;
	rs->block = block;
	ForState *f = rs->toForState();

	requireToken("for");
	requireToken("(");
	
	if (data->token.peek(0).isKeyWord() || !findSymbol(data->token.peek(0).getString())->isVariable)
	{
		f->ini = getVariableDefState(block);
	}
	else if (data->token.peek(0).getString() != ";")
	{
		f->ini = getExprState(block);
	}
	else
	{
		requireToken(";");
	}

	if (data->token.peek(0).getString() != ";")
	{
		f->con = getExprState(block);
		Types *tp = f->con->getType();
		if (tp == nullptr || (!tp->isBasic() && !tp->isPointer()))
		{
			addError(std::string("表达式必须为bool类型"));
		}
	}
	else
	{
		requireToken(";");
	}

	if (data->token.peek(0).getString() != ")")
	{
		f->after = getExpr(block);
	}

	requireToken(")");

	f->state = getStatement(block);
	if (f->state->isBlock())
	{
		f->state->toBlock()->type = LoopBlock;
	}

	data->currentOffset = offset;
	--data->currentLevel;
	return AStreeRef();
}

AStreeRef Parser::getBreakState(AStree * block)
{
	requireToken("break");
	requireToken(";");

	AStreeRef rs = std::make_unique<DoWhileState>();
	rs->function = data->currentFunction;
	rs->block = block;

	AStree *loop = nullptr;
	AStree *temp = block;

	while (temp)
	{
		if (temp->type == LoopBlock||temp->type==SwitchBlock)
		{
			loop = temp;
			break;
		}

		temp = temp->block;
	}

	if (loop == nullptr)
	{
		addError(std::string("break语句只能在循环或者switch语句中"));
	}

	return rs;
}

AStreeRef Parser::getReturnState(AStree * block)
{
	requireToken("return");
	FunctionType *tp = data->currentFunction->getType()->toFunction();
	
	AStreeRef rs = std::make_unique<ReturnState>();
	rs->function = data->currentFunction;
	rs->block = block;

	if (tp->returnType->isVoid())
	{	
		return rs;
	}

	ReturnState *r = rs->toReturnState();
	r->expr = getExprState(block);

	return rs;
}

AStreeRef Parser::getStringLiteral(AStree * block)
{
	AStreeRef res=std::make_unique<StringLiteralExpr>();
	res->block = block;
	res->function = data->currentFunction;
	res->toStringLiteral()->value = (char *)data->staticRegion.data + data->staticRegion.currentOffset;

	const std::string &str=data->token.read().getString();
	
	ArrayType arr;//将字符串放入静态区
	arr.dataType = data->allTypes[2].get();
	arr.capacity = str.size() + 1;
	VariableValue v;
	v.buffer = new char[str.size()+1];
	memcpy(v.buffer,str.c_str(),str.size());
	v.buffer[str.size()] = 0;
	v.type = &arr;
	data->staticRegion.addValue(v);
	v.release();

	if (res->toStringLiteral()->thisType == nullptr)
	{
		TypeRef ptr = std::make_unique<PointerType>();//创建一个类型
		ptr->toPointer()->targetType = data->allTypes[2].get();
		res->toStringLiteral()->thisType = ptr.get();
		data->allTypes.push_back(std::move(ptr));
	}

	return res;
}

AStreeRef Parser::getBlock(FunctionDef * fun, AStree * statement)
{
	int offset = data->currentOffset;
	bool ok = true;

	if (statement != nullptr)
	{
		ok = false;
	}
	else if (fun != nullptr)
	{
		FunctionType *type = fun->getType()->toFunction();
		int currentOffset = 0;

		for (int i = 0; i < type->args.size();)
		{
			if (!type->args[i].empty()&&type->argsType[i])
			{
				symbol t;
				t.isVariable = true;
				t.isDefined = true;
				t.offSet = currentOffset;
				t.type = type->argsType[i];
				data->symbols[data->currentLevel].insert(std::pair<std::string,symbol>(type->args[i],t));
			}

			if (type->getSize())
			{
				currentOffset += type->getSize();
			}
		}

	}

	if (ok)
	{
		++data->currentLevel;
		if (data->symbols.size() < data->currentLevel)
		{
			data->symbols.resize(data->currentLevel);
		}
		data->symbols[data->currentLevel].clear();
	}

	requireToken("{");
	AStreeRef block = std::make_unique<Block>();
	Block *blk = block->toBlock();
	blk->function = data->currentFunction;

	while (data->token.peek(0).getString() != "}")
	{
		blk->statements.push_back(getStatement(blk));
	}

	requireToken("}");
	if (ok)
	{
		--data->currentLevel;
	}
	data->currentOffset = offset;

	return block;
}

StructDef * Parser::getStructDef(int index)
{
	return &data->allStructDef[index];
}

UnionDef * Parser::getUnionDef(int index)
{
	return &data->allUnionDef[index];
}

EnumDef * Parser::getEnumDef(int index)
{
	return &data->allEnumDef[index];
}

VariableValue Parser::getConstIni(Types * type)
{
	if (data->token.peek(0).getString() != "=")
	{
		VariableValue t;
		t.type = type;
		return t;
	}

	data->token.read();

	bool isBaracket = false;

	if (type->isArray() || type->isStruct() || type->isUnion())
	{
		isBaracket = true;
	}

	if (isBaracket)
	{
		requireToken("{");
	}

	VariableValue v= getConstIniCore(type);

	if (isBaracket)
	{
		requireToken("}");
	}

	return v;
}

VariableValue Parser::getConstIniCore(Types * type)
{
	if (type->isBasic())
	{

		if (type->toBasic()->isFloat)
		{
			if (!data->token.peek(0).isRealLiteral() && !data->token.peek(0).isIntLiteral())
			{
				addError(std::string("无效的初始化类型"));
			}

			VariableValue v;
			v.buffer = new char[type->getSize()];
			v.type = type;
			double t = std::atof(data->token.read().getString().c_str());
			if (type->getSize() == sizeof(float))
			{
				*(float *)v.buffer = t;
			}
			else
			{
				*(double *)v.buffer = t;
			}
			return v;
		}
		else
		{
			if (!data->token.peek(0).isRealLiteral() && !data->token.peek(0).isIntLiteral())
			{
				addError(std::string("无效的初始化类型"));
			}

			int size = type->getSize();
			VariableValue v;
			v.buffer = new char[size];
			v.type = type;
			long long t = std::atoll(data->token.read().getString().c_str());
			auto basic = type->toBasic();

			if (size == sizeof(char))
			{
				if (basic->isSigned)
				{
					*(char *)v.buffer = t;
				}
				else
				{
					*(unsigned char *)v.buffer = t;
				}
			}
			else if (size== sizeof(short))
			{
				if (basic->isSigned)
				{
					*(short *)v.buffer = t;
				}
				else
				{
					*(unsigned short *)v.buffer = t;
				}

			}
			else if (size == sizeof(int))
			{
				if (basic->isSigned)
				{
					*(int *)v.buffer = t;
				}
				else
				{
					*(unsigned int *)v.buffer = t;
				}
			}
			else if (size == sizeof(long long))
			{
				if (basic->isSigned)
				{
					*(long long *)v.buffer = t;
				}
				else
				{
					*(unsigned long long *)v.buffer = t;
				}
			}

			return v;
		}

	}
	else if (type->isPointer())
	{
		int size = type->getSize();
		VariableValue v;
		v.buffer = new char[size];
		v.type = type;
		auto ptr = type->toPointer();

		if (ptr->targetType->isBasic() && ptr->targetType->getSize()==1&&data->token.peek(0).isStringLiteral())
		{
			const std::string &str=data->token.read().getString();
			VariableValue vv;
			vv.buffer = new char[str.size()+1];
			memcpy(v.buffer, str.c_str(), str.size());
			vv.buffer[str.size()] = 0;

			ArrayRef arr = std::make_unique<ArrayType>();
			arr->capacity = str.size() + 1;
			BasicRef basic = std::make_unique<BasicType>();
			basic->size = 1;
			arr->dataType = basic.get();

			*(char **)v.buffer = data->staticRegion.data + data->staticRegion.currentOffset;
			data->staticRegion.addValue(vv);

			vv.release();
		}
		else
		{
			long long t = std::atoll(data->token.read().getString().c_str());
			*(unsigned int *)v.buffer = t;
		}

		return v;
	}
	else if(type->isArray())
	{
		auto arr = type->toArray();
		VariableValue v;
		v.buffer = new char[type->getSize()];
		v.type = type;
		memset(v.buffer, 0, type->getSize());
		bool isBaracket = false;

		if (arr->dataType->isBasic() && arr->dataType->getSize() == 1 && data->token.peek(0).isStringLiteral())
		{
			const std::string &str = data->token.read().getString();
			memcpy(v.buffer,str.c_str(),str.size());
			return v;
		}

		if (data->token.peek(0).getString() == "{")
		{
			data->token.read();
			isBaracket = true;
		}

		for (int i = 0; i < arr->capacity; ++i)
		{
			VariableValue vv;
			vv = getConstIniCore(arr->dataType);

			memcpy(v.buffer + i*arr->dataType->getSize(), vv.buffer, arr->dataType->getSize());

			if (data->token.peek(0).getString() == "}")
			{
				break;
			}

			requireToken(",");
		}

		if (isBaracket)
		{
			requireToken("}");
		}

		return v;
	}
	else if (type->isStruct())
	{ 
		VariableValue v;
		v.buffer = new char[type->getSize()];
		v.type = type;
		memset(v.buffer, 0, type->getSize());
		bool isBaracket = false;

		if (data->token.peek(0).getString() == "{")
		{
			data->token.read();
			isBaracket = true;
		}

		auto str = type->toStruct();

		auto &strdef = data->allStructDef[str->structDef];

		for (int i = 0; i <strdef.types.size(); ++i)
		{
			VariableValue vv;
			vv = getConstIniCore(strdef.types[i]);

			memcpy(v.buffer +strdef.offSets[i], vv.buffer, strdef.types[i]->getSize());

			if (data->token.peek(0).getString() == "}")
			{
				break;
			}

			requireToken(",");
		}

		if (isBaracket)
		{
			requireToken("}");
		}

		return v;
	}
	else if (type->isUnion())
	{
		auto uni = type->toUnion();
		int index = -1,max=-1;

		auto &unidef = data->allStructDef[uni->unionDef];

		for (int i = 0; i < unidef.types.size(); ++i)
		{
			if (unidef.types[i]->getSize() > max)
			{
				max = unidef.types[i]->getSize();
				index = i;
			}
		}

		return getConstIniCore(unidef.types[index]);
	}

	addError(std::string("错误的初始化方式"));
}

IniRef Parser::getIni(Types * type,AStree *block)
{
	if (data->token.peek(0).getString() != "=")
	{
		IniRef t=std::make_unique<IniList>();
		t->type = type;
		return t;
	}

	data->token.read();

	bool isBaracket = false;
	//可以直接初始化
	if (type->isArray() || type->isStruct() || type->isUnion())
	{
		isBaracket = true;
	}

	if (isBaracket)
	{
		requireToken("{");
	}

	IniRef v = getIniCore(type,block);

	if (isBaracket)
	{
		requireToken("}");
	}

	return v;
}

IniRef Parser::getIniCore(Types * type,AStree *block)
{
	if (type->isBasic())
	{
		IniRef t = std::make_unique<IniList>();
		t->expr = getExpr(block, false);
		if (!t->expr->getType()->compatible(type))
		{
			addError(std::string("不兼容的类型"));
		}
		return t;
	}
	else if (type->isPointer())
	{
		auto ptr = type->toPointer();
		IniRef t = std::make_unique<IniList>();

		if (ptr->targetType->isBasic() && ptr->targetType->getSize() == 1 && data->token.peek(0).isStringLiteral())
		{
			t->expr = getStringLiteral(block);
			return t;
		}

		t->expr = getExpr(block, false);
		if (!t->expr->getType()->compatible(type))
		{
			addError(std::string("不兼容的类型"));
		}

	}
	else if (type->isArray())
	{
		auto arr = type->toArray();
		IniRef t = std::make_unique<IniList>();

		if (arr->dataType->isBasic() && arr->dataType->getSize() == 1 && data->token.peek(0).isStringLiteral())
		{
			t->expr = getStringLiteral(block);
			return t;
		}

		bool isBaracket = false;

		if (data->token.peek(0).getString() == "{")
		{
			isBaracket = true;
		}

		if (isBaracket)
		{
			requireToken("{");
		}

		int offset = 0;

		for (int i = 0; i < arr->capacity; ++i)
		{
			t->nexts.push_back(getIniCore(arr->dataType,block));
			t->offset.push_back(offset);
			offset += arr->dataType->getSize();
			if (data->token.peek(0).getString() == "}")
			{
				break;
			}

			requireToken(",");
		}

		if (isBaracket)
		{
			requireToken("}");
		}
	}
	else if (type->isStruct())
	{
		auto str = type->toStruct();
		IniRef t = std::make_unique<IniList>();
		bool isBaracket = false;

		if (data->token.peek(0).getString() == "{")
		{
			isBaracket = true;
		}

		if (isBaracket)
		{
			requireToken("{");
		}

		auto &strdef = data->allStructDef[str->structDef];
		int offset = 0;
		for (int i = 0; i <strdef.types.size(); ++i)
		{
			t->nexts.push_back(getIniCore(strdef.types[i],block));
			t->offset.push_back(offset);
			offset += strdef.types[i]->getSize();
			if (data->token.peek(0).getString() == "}")
			{
				break;
			}

			requireToken(",");
		}

		if (isBaracket)
		{
			requireToken("}");
		}

		return t;
	}
	else if (type->isUnion())
	{
		auto uni = type->toUnion();
		int index = -1, max = -1;

		auto &unidef = data->allStructDef[uni->unionDef];

		for (int i = 0; i < unidef.types.size(); ++i)
		{
			if (unidef.types[i]->getSize() > max)
			{
				max = unidef.types[i]->getSize();
				index = i;
			}
		}

		return getIniCore(unidef.types[index],block);
	}

	addError(std::string("错误的初始化方式"));

	return IniRef();
}

variable Parser::getVariables(Types *pre,bool typeTran)
{
	data->origin.clear();

	std::list<TypeToken> typeTokens;

	int left = 0;
	int right = 0;
	int index = 0;

	TypeToken tt;
	tt.token = nullptr;
	tt.index = -1;
	typeTokens.push_back(tt);

	while (true)
	{
		auto str = data->token.peek(0).getString();
		if (str == ";"||str=="{")
		{
			break;
		}
		else if (str == ",")
		{
			if (left == right)
			{
				data->token.read();
				break;
			} 
		}
		else if (str == "=")
		{
			break;
		}
		else if (str == "(")
		{
			++left;
		}
		else if (str == ")")
		{
			if (typeTran&&left == right)
			{
				break;
			}
			++right;
		}

		tt.token = &data->token.read();
		tt.index = index;
		++index;

		typeTokens.push_back(tt);
	}

	if (!typeTran&&typeTokens.empty())
	{
		addError(std::string("应输入变量名"));
	}

	data->vName.push_back("");
	data->origin.push_back(typeTokens);
	auto type=parseType(pre, typeTokens,0,0);

	if (typeTran)
	{
		if (data->vName[0] != "")
		{
			addError(std::string("应输入)"));
		}
	}
	else if (data->vName[0] == "")
	{
		addError(std::string("应输入变量名"));
	}

	variable t;
	t.name = data->vName[0];
	t.type = type;

	data->vName.pop_back();
	data->origin.pop_back();

	//t.type->isConst = pre->isConst;
	bool sta = pre->isStatic;
	pre->isStatic = false;
	t.type->isStatic =sta;

	if (!t.type->canInstance())
	{
		addError(std::string("不能定义此类型的变量"));
	}

	return t;
}

Types * Parser::parseType(Types * pre, std::list<TypeToken>& tokens,int name,int level)
{
	auto it = tokens.begin();
	++it;

	if (it == tokens.end())
	{
		return pre;
	}

	auto clearBarack = [&tokens](auto &i1,auto &i2)->void
	{
		while (true)//清理多余的括号
		{
			if (i1 == tokens.begin() || i2 == tokens.end())
			{
				break;
			}
			if (i1->token->getString() == "(" && i2->token->getString() == ")")
			{
				auto n1 = i1;
				--n1;
				auto n2 = i2;
				++n2;

				tokens.erase(i1);
				tokens.erase(i2);
				i1 = n1;
				i2 = n2;
			}
			else
			{
				break;
			}
		}
	};

	auto addIt = [this, &tokens](auto &it)
	{
		if (it == tokens.end())
		{
			addError(std::string("代码不完整，缺少必要的token"));
		}
		++it;
	};

	bool isConst = false;
	auto ic = tokens.end();

	for(;it!=tokens.end();++it)
	{
		const std::string &str = it->token->getString();
		if (str == "*")
		{
			auto in = it;
			addIt(in);

			if (in == tokens.end() || in->token->getString() == ")")
			{
				if (name == 1)
				{
					int id = it->index;

					auto n = data->origin[level].begin();
					while (true)
					{
						if (n->index == id)
						{
							break;
						}
						++n;
					}

					++n;

					while (n->token->getString() == "("||n->token->getString()=="const")
					{
						++n;
					}

					if (n->index != data->index)
					{
						addError(std::string("标识符的位置不正确"));
					}

				}

				if (isConst)//清除const
				{
					auto i1 = ic;
					auto i2 = ic;
					--i1;
					addIt(i2);
					tokens.erase(ic);
					clearBarack(i1, i2);
				}

				auto i1 = it;
				auto i2 = it;
				--i1;
				addIt(i2);

				tokens.erase(it);
				clearBarack(i1, i2);

				PointerRef pt = std::make_unique<PointerType>();
				pt->targetType = parseType(pre, tokens, 2,level);
				if (pt->targetType != pre)
				{
					pt->targetType->isConst = isConst;
				}

				auto result = pt.get();
				data->allTypes.push_back(std::move(pt));

				return result;
			}

			isConst = false;
		}
		else if (it->token->isIdentifier() && !it->token->isKeyWord())
		{
			if (name != 0)
			{
				addError(std::string("不应有标识符"));
			}

			data->vName[level] = str;
			data->index = it->index;

			if (isConst)//清除const
			{
				auto i1 = ic;
				auto i2 = ic;
				--i1;
				addIt(i2);
				tokens.erase(ic);
				clearBarack(i1, i2);
			}

			auto i1 = it;
			auto i2 = it;

			--i1; 	
			addIt(i2);
			tokens.erase(it);
			clearBarack(i1, i2);

			auto p = parseType(pre,tokens,1,level);
			if (p != pre)
			{
				p->isConst = isConst;
			}

			return p;
		}
		else if (it->token->getString() == "const")
		{
			isConst = true;
			ic = it;
		}
		else if (str == "(")
		{
			auto in = it;
			addIt(in);
			auto instr = in->token->getString();
			if (instr == ")" || (in->token->isIdentifier() && name!=0))
			{
				if (name == 1)
				{
					int id = it->index;

					auto n = data->origin[level].begin();
					while (true)
					{
						if (n->index == id)
						{
							break;
						}
						++n;
					}

					--n;

					while (n->token->getString() == ")")
					{
						--n;
					}

					if (n->index != data->index)
					{
						addError(std::string("标识符的位置不正确"));
					}

				}

				FunctionRef f = std::make_unique<FunctionType>();
			
				if (instr == "void")
				{
					auto s = in;
					addIt(in);
					tokens.erase(s);
				}
				else if(instr!=")")
				{

					while (true)
					{
						data->argTokens.clear();
						TypeToken tt;
						tt.token = nullptr;
						tt.index = -1;
						int left = 0;
						int right = 0;
						bool out = false;

						while (true)//读取一个参数
						{
							auto str = in->token->getString();
							if (str == ",")
							{
								if (left == right)
								{
									auto n = in;
									addIt(in);
									tokens.erase(n);
									break;
								}
							}
							else if (str == "=")
							{
								addError(std::string("C语言不允许默认参数"));
							}
							else if (str == "(")
							{
								++left;
							}
							else if (str == ")")
							{
								if (left == right)
								{
									out = true;
									break;
								}
								++right;
							}

							tt.token = in->token;
							data->argTokens.push_back(tt);

							auto n = in;
							addIt(in);
							tokens.erase(n);
						}

						if (data->argTokens.size() == 3)
						{
							bool flag = true;
							for (auto it = data->argTokens.begin(); it != data->argTokens.end(); ++it)
							{
								if (it->token->getString() != ".")
								{
									flag = false;
									break;
								}
							}

							if (!flag)
							{
								addError(std::string("错误的参数类型"));
							}

							f->args.push_back("");
							f->argsType.push_back(nullptr);
							break;
						}


						Types *tp = peekType(true);
						variable t;

						if (data->argTokens.empty())
						{
							t.type = tp;
						}
						else
						{
							int index = 0;
							for (auto it = data->argTokens.begin(); it != data->argTokens.end(); ++it)
							{
								it->index = index;
								++index;
							}
							tt.token = nullptr;
							tt.index = -1;
							data->argTokens.push_front(tt);
							std::list<TypeToken> typeTokens = data->argTokens;
							data->vName.push_back("");
							data->origin.push_back(typeTokens);

							auto thetype = parseType(tp, typeTokens, 0, level + 1);

							t.name = data->vName[level + 1];
							t.type = thetype;

							data->vName.pop_back();
							data->origin.pop_back();
						}

						for (int i = 0; i < f->args.size(); ++i)
						{
							if (f->args[i] != ""&&f->args[i] == t.name)
							{
								addError(std::string("有重名的参数"));
							}
						}

						f->args.push_back(t.name);
						f->argsType.push_back(t.type);

						if (out)
						{
							break;
						}
					}
					
				}

				auto i1 = in;
				auto i2 = in;
				--i1;
				clearBarack(i1, i2);

				f->returnType = parseType(pre, tokens, 2, level);
				if (f->returnType->isFunction() || f->returnType->isArray())
				{
					addError(std::string("返回值类型无效"));
				}

				if (isConst)
				{
					addError(std::string("此处不应有const"));
				}

				auto result = f.get();
				data->allTypes.push_back(std::move(f));

				return result;
			}

		}
		else if (str == "[")
		{
			if (isConst)
			{
				addError(std::string("此处不应有const"));
			}

			if (name == 1)
			{
				int id = it->index;

				auto n = data->origin[level].begin();
				while (true)
				{
					if (n->index == id)
					{
						break;
					}
					++n;
				}

				--n;

				while (n->token->getString() == ")")
				{
					--n;
				}

				if (n->index != data->index)
				{
					addError(std::string("标识符的位置不正确"));
				}

			}

			++it;

			ArrayRef arr = std::make_unique<ArrayType>();
			if (it->token->isIntLiteral())
			{
				arr->capacity = (int)it->token->getInteger();			//读取数组大小
				if (arr->capacity <= 0)
				{
					addError(std::string("数组大小必须大于0"));
				}
				auto ci = it;
				addIt(it);
				tokens.erase(ci);
			}
			else
			{
				addError(std::string("应该输入数组大小"));
			}

			if (it->token->getString() != "]")
			{
				addError(std::string("缺少]"));
			}

			auto i1 = it;
			auto i2 = it;
			--i1;
			addIt(it);
			tokens.erase(i1);
			tokens.erase(i2);
			i1 = it;
			i2 = it;
			clearBarack(i1, i2);

			arr->dataType = parseType(pre, tokens, 2,level);
			auto result = arr.get();
			data->allTypes.push_back(std::move(arr));

			return result;
		}
		else if (str == ")")
		{
			continue;
		}
		else
		{
			addError(std::string("错误的类型声明"));
		}

	}

	addError(std::string("无效的类型组合"));

	return nullptr;
}

symbol * Parser::findSymbol(const std::string & name)
{
	for (int i = data->currentLevel; i >= 0; --i)
	{
		auto it = data->symbols[i].find(name);
		if (it != data->symbols[i].end())
		{
			return &it->second;
		}
	}

	return nullptr;
}

Types * Parser::peekType(bool args)
{
	bool isStatic = false;
	bool isConst = false;

	std::vector<std::string> basic;
	auto it = data->argTokens.begin();

	auto clear = [&it,args,this]()
	{
		if (args)
		{
			for (auto i = data->argTokens.begin(); i != it;)
			{
				auto n = i;
				++i;
				data->argTokens.erase(n);
			}
		}
	};

	while (true)
	{
		const Token *ct;
		if (!args)
		{
			ct = &data->token.read();
		}
		else
		{
			if (it == data->argTokens.end())
			{
				break;
			}
			ct = it->token;
			++it;
		}

		const std::string &str =ct->getString();
		if (str == "static")
		{
			if (isStatic == false)
			{
				isStatic = true;
			}
			else
			{
				addError(std::string("无效的类型组合说明符"));
			}
		}
		else if (str == "const")
		{
			if (isConst == false)
			{
				isConst = true;
			}
			else
			{
				addError(std::string("无效的类型组合说明符"));
			}
		}
		else if (str == "void")
		{
			auto p = data->allTypes[22]->copy();
			p->isStatic = isStatic;
			p->isConst = isConst;
			auto np = p.get();
			data->allTypes.push_back(std::move(p));
			clear();
			return np;
		}
		else if (str == "struct"||str=="union"||str=="enum")
		{
			int tp=3;
			if (str == "struct")
			{
				tp = 1;
			}
			else if (str == "union")
			{
				tp = 2;
			}

			auto &nt = data->token.read();

			if (nt.isIdentifier() && !nt.isKeyWord())
			{
				const std::string &name =nt.getString();
				symbol *syb=findSymbol(name);

				if (!syb)
				{
					addError(std::string("未定义的类型")+name);
				}

				bool ok = true;

				if (tp == 1&&!syb->type->isStruct())
				{
					ok = false;
				}
				else if (tp == 2 && !syb->type->isUnion())
				{
					ok = false;
				}
				else if(!syb->type->isEnum())
				{
					ok = false;
				}

				if (!ok)
				{
					addError(std::string("无效的类型") + name);
				}

				auto pt = isConst ? syb->constType : syb->type;
				auto p = pt->copy();
				p->isStatic = isStatic;
				p->isConst = isConst;
				auto np = p.get();
				data->allTypes.push_back(std::move(p));
				clear();
				return np;
			}
			else
			{
				addError(std::string("应输入正确的标识符"));
			}
		}
		else if (ct->isIdentifier() && !ct->isKeyWord())
		{
			const std::string &name = ct->getString();
			symbol *syb = findSymbol(name);

			if (!syb)
			{
				if (basic.empty())
				{
					addError(std::string("未定义的标识符"));
				}

				if (!args)
				{
					data->token.unRead();
				}
				else
				{
					--it;
				}
				break;
			}

			if (syb->isVariable)
			{
				if (!basic.empty())
				{
					if (!args)
					{
						data->token.unRead();
					}
					else
					{
						--it;
					}
					break;
				}
				addError(std::string("无效的类型组合说明符"));
			}
			
			auto pt = isConst ? syb->constType : syb->type;
			auto p = pt->copy();
			p->isStatic = isStatic;
			p->isConst = isConst;
			auto np = p.get();
			data->allTypes.push_back(std::move(p));
			clear();
			return np;
		}
		else if(ct->isKeyWord())
		{
			basic.push_back(str);
		}
		else
		{
			data->token.unRead();
			break;
		}
	}

	if (basic.empty())
	{
		addError(std::string("应输入类型说明符"));
	}

	int result = getBasicType(basic);

	if (isConst)
	{
		++result;
	}

	clear();

	auto p=data->allTypes[result]->copy();
	p->isStatic = isStatic;
	p->isConst = isConst;
	auto pt = p.get();
	data->allTypes.push_back(std::move(p));

	return pt;
}

int Parser::getBasicType(std::vector<std::string>& basic)
{
	std::sort(basic.begin(), basic.end());

	if (basic[0] == "bool")
	{
		if (basic.size() != 1)
		{
			addError(std::string("无效的类型组合"));
		}

		return 0;
	}
	else if (basic[0] == "char")
	{
		if (basic.size() > 1)
		{
			if (basic.size() != 2)
			{
				addError(std::string("无效的类型组合"));
			}

			if (basic[1] == "signed")
			{
				return 2;
			}
			else if (basic[1] == "unsigned")
			{
				return 4;
			}
			else
			{
				addError(std::string("无效的类型组合"));
			}
		}
		
		return 2;
	}
	else if (basic[0] == "double")
	{
		if (basic.size() != 1)
		{
			addError(std::string("无效的类型组合"));
		}

		return 20;
	}
	else if (basic[0] == "float")
	{
		if (basic.size() != 1)
		{
			addError(std::string("无效的类型组合"));
		}

		return 18;
	}
	else if (basic[0] == "int")
	{
		if (basic.size() == 1)
		{
			return 10;
		}

		int isLong = 0;
		bool isSigned = false;
		bool isUnSigned = false;
		bool isShort = false;

		for (int i = 1; i < basic.size(); ++i)
		{
			if (basic[i] == "long")
			{
				if (isShort)
				{
					addError(std::string("无效的类型组合"));
				}
				++isLong;
			}
			else if (basic[i] == "short")
			{
				if (isShort||isLong)
				{
					addError(std::string("无效的类型组合"));
				}
				isShort = true;
			}
			else if (basic[i] == "signed")
			{
				if (isSigned||isUnSigned)
				{
					addError(std::string("无效的类型组合"));
				}
				isSigned = true;
			}
			else if (basic[i] == "unsigned")
			{
				if (isUnSigned||isUnSigned)
				{
					addError(std::string("无效的类型组合"));
				}
				isUnSigned = true;
			}
			else
			{
				addError(std::string("无效的类型组合"));
			}
		}

		if (isShort)
		{
			return isUnSigned ? 8 : 6;
		}

		if (isLong == 2)
		{
			return isUnSigned ? 16 : 14;
		}

		return isUnSigned ? 12 : 10;
	}
	else if (basic[0] == "long")
	{
		if (basic.size() == 1)
		{
			return 10;
		}

		bool isLong = false;
		bool isSigned = false;
		bool isUnSigned = false;

		for (int i = 1; i < basic.size(); ++i)
		{
			if (basic[i] == "long")
			{
				if ( isLong)
				{
					addError(std::string("无效的类型组合"));
				}
				isLong = true;
			}
			else if (basic[i] == "signed")
			{
				if (isSigned || isUnSigned)
				{
					addError(std::string("无效的类型组合"));
				}
				isSigned = true;
			}
			else if (basic[i] == "unsigned")
			{
				if (isUnSigned || isUnSigned)
				{
					addError(std::string("无效的类型组合"));
				}
				isUnSigned = true;
			}
			else
			{
				addError(std::string("无效的类型组合"));
			}
		}

		if (isLong)
		{
			return isUnSigned ? 16 : 14;
		}

		return isUnSigned ? 12 : 10;
	}
	else if (basic[0] == "short")
	{
		if (basic.size() > 1)
		{
			if (basic.size() != 2)
			{
				addError(std::string("无效的类型组合"));
			}

			if (basic[1] == "signed")
			{
				return 6;
			}
			else if (basic[1] == "unsigned")
			{
				return 8;
			}
			else
			{
				addError(std::string("无效的类型组合"));
			}
		}

		return 6;
	}

	addError(std::string("无效的类型组合"));
	return 0;
}
