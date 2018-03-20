#include "Parser.h"
#include "Preprocessor.h"
#include "Tokenizer.h"
#include "Types.h"
#include "Custom.h"

#include <memory>
#include <list>
#include <map>
#include <algorithm>

enum ParseType
{
	TYPEDEF,
	STRUCT,
	UNION,
	ENUM,
	VARIABLE,
};

struct variables
{
	std::vector<std::string> names;
	Types * type;
};

struct symbol
{
	bool isVariable;
	bool isDefined;
	int offSet;
	Types *type;
	Types *constType;//只有当isVariable为false时这个字段才有效
};

struct VariableValue
{
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
	void addValue(VariableValue &v)
	{

	}
	char *data;
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
	std::vector<TypeRef> allTypes;
	std::list<Token *> line;
	ParseType type;
	int currentLevel;
};

Parser::Parser()
{
	data = new Data;
	createBaseType();
}

Parser::~Parser()
{
	delete data;
}

void Parser::parse(const std::string &file)
{
	std::string code=data->pre.doFile(file);
	data->token.parse(code);
	data->currentLevel = 0;

	while (data->token.hasMore())
	{
		getLine();
		if (data->type == STRUCT)
		{
			getStruct();
		}
		else if (data->type == UNION)
		{

		}
		else if (data->type == ENUM)
		{

		}
		else if (data->type == VARIABLE)
		{

		}

	}



}

void Parser::getLine()
{
	int i;

	data->line.clear();
	for(i=0;;++i)
	{
		const std::string &str = data->token.peek(i).getString();
		if (str == "{" || str == ";")
		{
			data->line.push_back(&data->token.peek(i));
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

	auto it=data->symbols[0].find(name);

	auto reg = [&]()
	{
		symbol t;
		t.isDefined = false;
		t.isVariable = false;
		t.offSet = -1;
		strdef.size = -1;
		strdef.name = name;
		data->allStructDef.push_back(strdef);

		StructRef tpr = std::make_unique<StructType>();
		tpr->name = name;
		tpr->structDef = &data->allStructDef[data->allStructDef.size() - 1];

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

		variables vs = getVariables(false);

		if (vs.type->isStatic || vs.type->isConst)
		{
			addError(std::string("无效的类型说明符"));
		}

		for(int i=0;i<vs.names.size();++i)
		{
			for (int j = 0; j < strdef.members.size(); ++j)
			{
				if (strdef.members[j] == vs.names[i])
				{
					addError(std::string("成员重定义"));
				}
			}

			strdef.members.push_back(vs.names[i]);

			if (strdef.offSets.empty())
			{
				strdef.offSets.push_back(0);
			}
			else
			{
				strdef.offSets.push_back(strdef.offSets[strdef.offSets.size() - 1] + vs.type->getSize());
			}

			strdef.size += vs.type->getSize();
			strdef.types.push_back(vs.type);
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
		tpr->structDef = &data->allStructDef[data->allStructDef.size() - 1];

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
		unidef.size = -1;
		unidef.name = name;
		data->allUnionDef.push_back(unidef);

		UnionRef tpr = std::make_unique<UnionType>();
		tpr->name = name;
		tpr->unionDef = &data->allUnionDef[data->allUnionDef .size() - 1];

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

		variables vs = getVariables(false);

		if (vs.type->isStatic || vs.type->isConst)
		{
			addError(std::string("无效的类型说明符"));
		}

		for (int i = 0; i<vs.names.size(); ++i)
		{
			for (int j = 0; j < unidef.members.size(); ++j)
			{
				if (unidef.members[j] == vs.names[i])
				{
					addError(std::string("成员重定义"));
				}
			}

			unidef.members.push_back(vs.names[i]);

			unidef.size = std::max(unidef.size,vs.type->getSize());
			unidef.types.push_back(vs.type);
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
		tpr->unionDef = &data->allUnionDef[data->allUnionDef.size() - 1];

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

}

void Parser::addError(std::string & info)
{
	throw info;
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

void Parser::getStructVariable(bool isStatic, bool isConst, Types * type, Types * constType)
{
	while (true)
	{
		auto token = data->token.read();
		const std::string &str = token.getString();

		if (str == ";")
		{
			return;
		}

		if (str == "*")
		{
			auto t = data->token.read();
			if (token.isIdentifier() && !token.isKeyWord())
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

				if (isStatic)
				{
					val.isDefined = true;
					val.offSet = data->staticRegion.currentOffset;
					val.type = ptr.get();
					data->staticRegion.values.insert(std::pair<std::string, Value>(vname, val));
					data->staticRegion.addValue(v);
				}
				else
				{
					val.isDefined = true;
					val.offSet = data->globalRegion.currentOffset;
					val.type = ptr.get();
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

			if (data->token.peek(0).getString() == "[")
			{
				data->token.read();
				int index = getArrayIndexConst();
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

			if (isStatic)
			{
				val.isDefined = true;
				val.offSet = data->staticRegion.currentOffset;
				val.type = tp;
				data->staticRegion.values.insert(std::pair<std::string, Value>(str, val));
				data->staticRegion.addValue(v);
			}
			else
			{
				val.isDefined = true;
				val.offSet = data->globalRegion.currentOffset;
				val.type = tp;
				data->globalRegion.values.insert(std::pair<std::string, Value>(str, val));
				data->globalRegion.addValue(v);
			}

			v.release();

			int index = isStatic ? 1 : 0;

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
	while (true)
	{
		auto token = data->token.read();
		const std::string &str = token.getString();

		if (str == ";")
		{
			return;
		}

		if (str == "*")
		{
			auto t = data->token.read();
			if (token.isIdentifier() && !token.isKeyWord())
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
				index = getArrayIndexConst();
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

			if (!isArray&&str == name)
			{
				continue;
			}

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

VariableValue Parser::getConstIni(Types * type)
{
	return VariableValue();
}

int Parser::getArrayIndexConst()
{
	return 0;
}

variables Parser::getVariables(bool ini)
{
	return variables();
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

Types * Parser::peekType()
{
	bool isStatic = false;
	bool isConst = false;

	std::vector<std::string> basic;

	while (true)
	{
		const std::string &str = data->token.peek(0).getString();
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
			data->token.read();
			return data->allTypes[22].get();
		}
		else if (str == "struct"||str=="union"||str=="enum")
		{
			data->token.read();
			int tp=3;
			if (str == "struct")
			{
				tp = 1;
			}
			else if (str == "union")
			{
				tp = 2;
			}

			if (data->token.peek(0).isIdentifier() && !data->token.peek(0).isKeyWord())
			{
				const std::string &name = data->token.read().getString();
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

				data->token.read();
				auto pt= isConst ? syb->constType : syb->type;
				pt->isStatic = isStatic;
				return pt;
			}
			else
			{
				addError(std::string("应输入正确的标识符"));
			}
		}
		else if (data->token.peek(0).isIdentifier() && !data->token.peek(0).isKeyWord())
		{
			const std::string &name = data->token.read().getString();
			symbol *syb = findSymbol(name);

			if (!syb)
			{
				break;
			}

			data->token.read();
			
			auto pt = isConst ? syb->constType : syb->type;
			pt->isStatic = isStatic;
			return pt;
		}
		else if(data->token.peek(0).isKeyWord())
		{
			basic.push_back(str);
			data->token.read();
		}
		else
		{
			break;
		}
	}

	int result = getBasicType(basic);

	if (isConst)
	{
		++result;
	}

	data->allTypes[result]->isStatic = isStatic;

	return data->allTypes[result].get();
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
