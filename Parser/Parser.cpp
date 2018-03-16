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
};

struct Region
{
	char *data;
	int currentOffset;
	std::map<std::string, symbol> globals;
};

struct Parser::Data
{
	Preprocessor pre;
	Tokenizer token;
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
	data->currentLevel = 2;

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
			addError(std::string("代码不完整，缺少必要的Token"));
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
			addError(std::string("错误的声明"));
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

	const Token &t = data->token.peek(0);

	if (!t.isIdentifier() || t.isKeyWord())
	{
		addError(std::string("应输入正确的标识符"));
	}

	std::string name = data->token.peek(0).getString();
	data->token.read();

	bool isDef=data->token.peek(0).getString()=="{";
	auto it=data->symbols[0].find(name);

	auto reg = [&]()
	{
		symbol t;
		t.isDefined = false;
		t.isVariable = false;
		t.offSet = -1;
		strdef.size = -1;
		data->allStructDef.push_back(strdef);
		StructRef tpr = std::make_unique<StructType>();
		tpr->name = name;
		tpr->structDef = &data->allStructDef[data->allStructDef.size() - 1];
		data->allTypes.push_back(std::move(tpr));
		t.type = tpr.get();
		data->symbols[0].insert(std::pair<std::string, symbol>(name, t));
	};

	if (it == data->symbols[0].end())
	{
		reg();
		if (!isDef)
		{
			data->token.read();
			return;
		}
	}
	else if (!it->second.type->isStruct())
	{
		addError(std::string("重定义，不同的类型"));
	}
	else if (it->second.isDefined)
	{
		addError(std::string("结构体重定义"));
	}

	it = data->symbols[0].find(name);

	/*获取结构体定义*/
	requireToken("{");
	
	while (true)
	{
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

		if (data->token.peek(0).getString() == "}")
		{
			break;
		}
	}

	requireToken("}");

	*it->second.type->toStruct()->structDef = strdef;
	it->second.isDefined = true;
	//todo,全局变量的解析


}

void Parser::getUnion(bool isTypedef)
{
	bool isStatic = false;
	bool isConst = false;
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

	const Token &t = data->token.peek(0);

	if (!t.isIdentifier() || t.isKeyWord())
	{
		addError(std::string("应输入正确的标识符"));
	}

	std::string name = data->token.peek(0).getString();
	data->token.read();

	bool isDef = data->token.peek(0).getString() == "{";
	auto it = data->symbols[0].find(name);

	auto reg = [&]()
	{
		symbol t;
		t.isDefined = false;
		t.isVariable = false;
		t.offSet = -1;
		unidef.size = -1;
		data->allUnionDef.push_back(unidef);
		UnionRef tpr = std::make_unique<UnionType>();
		tpr->name = name;
		tpr->unionDef = &data->allUnionDef[data->allUnionDef.size() - 1];
		data->allTypes.push_back(std::move(tpr));
		t.type = tpr.get();
		data->symbols[0].insert(std::pair<std::string, symbol>(name, t));
	};

	if (it == data->symbols[0].end())
	{
		reg();
		if (!isDef)
		{
			data->token.read();
			return;
		}
	}
	else if (!it->second.type->isUnion())
	{
		addError(std::string("重定义，不同的类型"));
	}
	else if (it->second.isDefined)
	{
		addError(std::string("联合体重定义"));
	}

	it = data->symbols[0].find(name);

	/*获取结构体定义*/
	requireToken("{");

	while (true)
	{
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

			unidef.size = std::max(vs.type->getSize(), unidef.size);
			unidef.types.push_back(vs.type);
		}

		if (data->token.peek(0).getString() == "}")
		{
			break;
		}
	}

	requireToken("}");

	*it->second.type->toUnion()->unionDef = unidef;
	it->second.isDefined = true;
	//todo,全局变量的解析

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
			return data->allTypes[0].get();
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
				return syb->type;
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
			return syb->type;
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

	if (isStatic)
	{
		data->allTypes[result]->isStatic = true;
	}

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
