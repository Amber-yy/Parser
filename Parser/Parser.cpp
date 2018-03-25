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
	bool isVariable;
	bool isDefined;
	int offSet;
	Types *type;
	Types *constType;//ֻ�е�isVariableΪfalseʱ����ֶβ���Ч
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
			delete data;
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
	std::list<TypeToken> origin;
	std::string vName;
	ParseType type;
	int currentLevel;
	int index;
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
			getUnion();
		}
		else if (data->type == ENUM)
		{
			getEnum();
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
			addError(std::string("Ӧ�����ʶ��"));
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
				addError(std::string("��Ч���������˵����"));
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
				addError(std::string("��Ч���������˵����"));
			}
		}

		data->token.read();
	}

	if (isTypedef&&isStatic)
	{
		addError(std::string("����ʹ�ö��ִ����"));
	}

	std::string name;
	const Token &t = data->token.read();

	if (t.getString() == "{")//����������
	{
		isDef = true;
		isAnonymous = true;
	}
	else if (!t.isIdentifier() || t.isKeyWord())//�Ƕ��壬���Ǳ�ʶ������ȷ
	{
		addError(std::string("Ӧ������ȷ�ı�ʶ��"));
	}
	else//��һ���Ϸ��ı�ʶ�����Ƿ��Ƕ���ȡ������һ���ַ��Ƿ���{
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

		data->allTypes.push_back(std::move(tpr));//����
		data->allTypes.push_back(std::move(tprc));//������
		data->symbols[0].insert(std::pair<std::string, symbol>(name, t));
	};

	if (it == data->symbols[0].end())//û��������
	{
		if (!isDef)//��������ֱ������
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
	else if (!it->second.type->isStruct())//���������ǽṹ��
	{
		addError(std::string("�ض��壬��ͬ������"));
	}
	else if (it->second.isDefined&&isDef)//���������Ѿ������
	{
		addError(std::string("�ṹ���ض���"));
	}

	it = data->symbols[0].find(name);

	/*��ȡ�ṹ�嶨��*/
	requireToken("{");
	
	while (true)
	{
		if (data->token.peek(0).getString() == "}")
		{
			if (strdef.size == 0)
			{
				addError(std::string("Ӧ��������"));
			}
			break;
		}

		Types *tp = peekType();

		if (tp->isStatic || tp->isConst)
		{
			addError(std::string("��Ч������˵����"));
		}

		std::vector<variable> vs;
		while (true)
		{
			variable v = getVariables(tp);
			if (v.type->isFunction())
			{
				addError(std::string("�����г�Ա����"));
			}
			vs.push_back(v);
		}

		for(int i=0;i<vs.size();++i)
		{
			for (int j = 0; j < strdef.members.size(); ++j)
			{
				if (strdef.members[j] == vs[i].name)
				{
					addError(std::string("��Ա�ض���"));
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

	if (!isAnonymous)//����
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

		data->allTypes.push_back(std::move(tpr));//����
		data->allTypes.push_back(std::move(tprc));//������
	}

	//ȫ�ֱ����Ľ���
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
				addError(std::string("��Ч���������˵����"));
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
				addError(std::string("��Ч���������˵����"));
			}
		}

		data->token.read();
	}

	if (isTypedef&&isStatic)
	{
		addError(std::string("����ʹ�ö��ִ����"));
	}

	std::string name;
	const Token &t = data->token.read();

	if (t.getString() == "{")//����������
	{
		isDef = true;
		isAnonymous = true;
	}
	else if (!t.isIdentifier() || t.isKeyWord())//�Ƕ��壬���Ǳ�ʶ������ȷ
	{
		addError(std::string("Ӧ������ȷ�ı�ʶ��"));
	}
	else//��һ���Ϸ��ı�ʶ�����Ƿ��Ƕ���ȡ������һ���ַ��Ƿ���{
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

		data->allTypes.push_back(std::move(tpr));//����
		data->allTypes.push_back(std::move(tprc));//������
		data->symbols[0].insert(std::pair<std::string, symbol>(name, t));
	};

	if (it == data->symbols[0].end())//û��������
	{
		if (!isDef)//��������ֱ������
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
	else if (!it->second.type->isUnion())//���������ǽṹ��
	{
		addError(std::string("�ض��壬��ͬ������"));
	}
	else if (it->second.isDefined&&isDef)//���������Ѿ������
	{
		addError(std::string("�ṹ���ض���"));
	}

	it = data->symbols[0].find(name);

	/*��ȡ�ṹ�嶨��*/
	requireToken("{");

	while (true)
	{
		if (data->token.peek(0).getString() == "}")
		{
			if (unidef.size == 0)
			{
				addError(std::string("Ӧ��������"));
			}
			break;
		}

		Types *tp = peekType();

		if (tp->isStatic || tp->isConst)
		{
			addError(std::string("��Ч������˵����"));
		}

		std::vector<variable> vs;
		while (true)
		{
			variable v = getVariables(tp);
			vs.push_back(v);
		}

		for (int i = 0; i<vs.size(); ++i)
		{
			for (int j = 0; j < unidef.members.size(); ++j)
			{
				if (unidef.members[j] == vs[i].name)
				{
					addError(std::string("��Ա�ض���"));
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

	if (!isAnonymous)//����
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

		data->allTypes.push_back(std::move(tpr));//����
		data->allTypes.push_back(std::move(tprc));//������
	}


	//ȫ�ֱ����Ľ���
	if (isTypedef)
	{
		getUnionTypedef(isStatic, isConst, type, constType, name);
	}
	else
	{
		getUnionVariable(isStatic, isConst, type, constType);
	}

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
				addError(std::string("��Ч���������˵����"));
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
				addError(std::string("��Ч���������˵����"));
			}
		}
	}

	auto token = data->token.peek(0);

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
		addError(std::string("Ӧ�����ʶ��"));
	}

	enumDef.name = name;
	data->allEnumDef.push_back(enumDef);

	EnumRef type = std::make_unique<EnumType>();
	EnumRef typec = std::make_unique<EnumType>();

	type->enumDef = &data->allEnumDef[data->allEnumDef.size()-1];
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
			addError(std::string("�ض����ʶ��"));
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
				addError(std::string("Ӧ����ö������"));
			}
			break;
		}

		auto token = data->token.read();
		if (token.isIdentifier() && !token.isKeyWord())
		{
			const std::string &str = token.getString();
			auto it = data->symbols[0].find(name);
			if (it != data->symbols[0].end())
			{
				addError(std::string("�ض����ʶ��"));
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
			t.offSet = data->globalRegion.currentOffset;
			data->globalRegion.addValue(v);
			data->symbols[0].insert(std::pair<std::string,symbol>(str,t));
			v.release();
		}
		else
		{
			addError(std::string("Ӧ��������ȷ�ı�ʶ��"));
		}
	}

	requireToken("}");






}

void Parser::addError(std::string & info)
{
	throw info;
}

void Parser::requireToken(const std::string & token)
{
	if (data->token.peek(0).getString() != token)
	{
		addError(std::string("ȱ��") + token);
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
					if (it1 != data->symbols[0].end() && it1->second.isVariable == false)//��������
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
					addError(std::string("�ض���ı�ʶ��"));
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
					//val.offSet = data->staticRegion.currentOffset;
					//data->staticRegion.values.insert(std::pair<std::string, Value>(vname, val));
					data->staticRegion.addValue(v);
				}
				else
				{
					val.offSet = data->globalRegion.currentOffset;
					if (data->globalRegion.values.find(vname) != data->globalRegion.values.end())
					{
						addError(std::string("�ض����ȫ�ֱ���"));
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
				data->symbols[index].insert(std::pair<std::string, symbol>(vname, syb));//�����ű��в���һ��ȫ�֣����߾�̬������

				data->allTypes.push_back(std::move(ptr));
				if (data->token.peek(0).getString() != ";")
				{
					requireToken(",");
				}
			}
			else
			{
				addError(std::string("Ӧ������ȷ�ı�ʶ��"));
			}
		}
		else if (token.isIdentifier() && !token.isKeyWord())//���飬���߱���
		{
			Types *tp;
			bool isArray = false;
			ArrayRef array;
			int index;

			if (data->token.peek(0).getString() == "[")
			{
				data->token.read();
				auto indexToken = data->token.read();
				if (indexToken.isIntLiteral())
				{
					index = (int)indexToken.getInteger();
				}
				else
				{
					addError(std::string("Ӧ�����������С"));
				}

				if (index <= 0)
				{
					addError(std::string("�����С�������0"));
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
				//val.offSet = data->staticRegion.currentOffset;
				//data->staticRegion.values.insert(std::pair<std::string, Value>(str, val));
				data->staticRegion.addValue(v);
			}
			else
			{
				val.offSet = data->globalRegion.currentOffset;
				if (data->globalRegion.values.find(str) != data->globalRegion.values.end())
				{
					addError(std::string("�ض����ȫ�ֱ���"));
				}
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
			data->symbols[index].insert(std::pair<std::string, symbol>(str, syb));//�����ű��в���һ��ȫ�֣����߾�̬������

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
			addError(std::string("��Ч����������"));
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
					if (it1 != data->symbols[0].end() && it1->second.isVariable == false)//��������
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
					addError(std::string("�ض���ı�ʶ��"));
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

				data->symbols[0].insert(std::pair<std::string, symbol>(vname, syb));//�����ű��в���һ������

				data->allTypes.push_back(std::move(ptr));
				data->allTypes.push_back(std::move(ptrc));
				if (data->token.peek(0).getString() != ";")
				{
					requireToken(",");
				}
			}
			else
			{
				addError(std::string("Ӧ������ȷ�ı�ʶ��"));
			}
		}
		else if (token.isIdentifier() && !token.isKeyWord())//���飬���߱���
		{
			Types *tp;
			bool isArray = false;
			ArrayRef array;
			int index;

			if (data->token.peek(0).getString() == "[")
			{
				data->token.read();
				auto indexToken = data->token.read();
				if (indexToken.isIntLiteral())
				{
					index = (int)indexToken.getInteger();
				}
				else
				{
					addError(std::string("Ӧ�����������С"));
				}

				if (index <= 0)
				{
					addError(std::string("�����С�������0"));
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

			data->symbols[0].insert(std::pair<std::string, symbol>(str, syb));//�����ű��в���һ��ȫ�֣����߾�̬������

			data->allTypes.push_back(std::move(array));
			data->allTypes.push_back(std::move(arrayc));

			if (data->token.peek(0).getString() != ";")
			{
				requireToken(",");
			}

		}
		else
		{
			addError(std::string("��Ч����������"));
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

VariableValue Parser::getConstIni(Types * type)
{
	return VariableValue();
}

variable Parser::getVariables(Types *pre)
{
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
		if (str == ";")
		{
			break;
		}
		if (str == ",")
		{
			if (left == right)
			{
				break;
			}
		}
		if (str == "=")
		{
			break;
		}
		if (str == "(")
		{
			++left;
		}
		if (str == ")")
		{
			++right;
		}

		tt.token = &data->token.read();
		tt.index = index;
		++index;

		typeTokens.push_back(tt);
	}

	data->vName = "";
	data->origin = typeTokens;
	auto type=parseType(pre, typeTokens,0);

	if (data->vName == "")
	{
		addError(std::string("Ӧ���������"));
	}

	if (type->isVoid())
	{
		addError(std::string("Ӧ������ȷ������"));
	}

	variable t;
	t.name = data->vName;
	t.type = type;

	return t;
}

Types * Parser::parseType(Types * pre, std::list<TypeToken>& tokens,int name)
{
	auto it = tokens.begin();
	++it;

	if (it == tokens.end())
	{
		return pre;
	}

	auto clearBarack = [&tokens](auto &i1,auto &i2)->void
	{
		while (true)//������������
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
			addError(std::string("���벻������ȱ�ٱ�Ҫ��token"));
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

					auto n = data->origin.begin();
					while (true)
					{
						if (n->index == id)
						{
							break;
						}
						++n;
					}

					++n;

					while (n->token->getString() == "(")
					{
						++n;
					}

					if (n->index != data->index)
					{
						addError(std::string("��ʶ����λ�ò���ȷ"));
					}

				}

				if (isConst)//���const
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
				pt->targetType = parseType(pre, tokens, 2);
				pt->targetType->isConst = isConst;

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
				addError(std::string("��Ӧ�б�ʶ��"));
			}

			data->vName = str;
			data->index = it->index;

			if (isConst)//���const
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

			auto p = parseType(pre,tokens,1);
			p->isConst = isConst;

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
			if (instr == ")" || (in->token->isIdentifier() && !in->token->isKeyWord()))
			{
				if (name == 1)
				{
					int id = it->index;

					auto n = data->origin.begin();
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
						addError(std::string("��ʶ����λ�ò���ȷ"));
					}

				}

				FunctionRef f = std::make_unique<FunctionType>();
				//��װ��ȡ����

				f->returnType = parseType(pre,tokens,2);
				if (f->returnType->isFunction() || f->returnType->isArray())
				{
					addError(std::string("����ֵ������Ч"));
				}

				if (isConst)
				{
					addError(std::string("�˴���Ӧ��const"));
				}

				auto i1 = in;
				auto i2 = in;
				--i1;
				clearBarack(i1, i2);

				auto result = f.get();
				data->allTypes.push_back(std::move(f));

				return result;
			}

		}
		else if (str == "[")
		{
			if (isConst)
			{
				addError(std::string("�˴���Ӧ��const"));
			}

			if (name == 1)
			{
				int id = it->index;

				auto n = data->origin.begin();
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
					addError(std::string("��ʶ����λ�ò���ȷ"));
				}

			}

			++it;


			ArrayRef arr = std::make_unique<ArrayType>();
			if (it->token->isIntLiteral())
			{
				arr->capacity = (int)it->token->getInteger();			//��ȡ�����С
				if (arr->capacity <= 0)
				{
					addError(std::string("�����С�������0"));
				}
				auto ci = it;
				addIt(it);
				tokens.erase(ci);
			}
			else
			{
				addError(std::string("Ӧ�����������С"));
			}

			if (it->token->getString() != "]")
			{
				addError(std::string("ȱ��]"));
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

			arr->dataType = parseType(pre, tokens, 2);
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
			addError(std::string("�������������"));
		}

	}

	addError(std::string("��Ч���������"));

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
				addError(std::string("��Ч���������˵����"));
			}
			data->token.read();
		}
		else if (str == "const")
		{
			if (isConst == false)
			{
				isConst = true;
			}
			else
			{
				addError(std::string("��Ч���������˵����"));
			}
			data->token.read();
		}
		else if (str == "void")
		{
			data->token.read();
			auto p = data->allTypes[22]->copy();
			p->isStatic = isStatic;
			data->allTypes.push_back(std::move(p));
			return p.get();
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
					addError(std::string("δ���������")+name);
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
					addError(std::string("��Ч������") + name);
				}

				auto pt = isConst ? syb->constType : syb->type;
				auto p = pt->copy();
				p->isStatic = isStatic;
				p->isConst = isConst;
				data->allTypes.push_back(std::move(p));
				return p.get();
			}
			else
			{
				addError(std::string("Ӧ������ȷ�ı�ʶ��"));
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
			auto p = pt->copy();
			p->isStatic = isStatic;
			p->isConst = isConst;
			data->allTypes.push_back(std::move(p));
			return p.get();
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

	auto p=data->allTypes[result]->copy();
	p->isStatic = isStatic;
	data->allTypes.push_back(std::move(p));

	return p.get();
}

int Parser::getBasicType(std::vector<std::string>& basic)
{
	std::sort(basic.begin(), basic.end());

	if (basic[0] == "bool")
	{
		if (basic.size() != 1)
		{
			addError(std::string("��Ч���������"));
		}

		return 0;
	}
	else if (basic[0] == "char")
	{
		if (basic.size() > 1)
		{
			if (basic.size() != 2)
			{
				addError(std::string("��Ч���������"));
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
				addError(std::string("��Ч���������"));
			}
		}
		
		return 2;
	}
	else if (basic[0] == "double")
	{
		if (basic.size() != 1)
		{
			addError(std::string("��Ч���������"));
		}

		return 20;
	}
	else if (basic[0] == "float")
	{
		if (basic.size() != 1)
		{
			addError(std::string("��Ч���������"));
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
					addError(std::string("��Ч���������"));
				}
				++isLong;
			}
			else if (basic[i] == "short")
			{
				if (isShort||isLong)
				{
					addError(std::string("��Ч���������"));
				}
				isShort = true;
			}
			else if (basic[i] == "signed")
			{
				if (isSigned||isUnSigned)
				{
					addError(std::string("��Ч���������"));
				}
				isSigned = true;
			}
			else if (basic[i] == "unsigned")
			{
				if (isUnSigned||isUnSigned)
				{
					addError(std::string("��Ч���������"));
				}
				isUnSigned = true;
			}
			else
			{
				addError(std::string("��Ч���������"));
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
					addError(std::string("��Ч���������"));
				}
				isLong = true;
			}
			else if (basic[i] == "signed")
			{
				if (isSigned || isUnSigned)
				{
					addError(std::string("��Ч���������"));
				}
				isSigned = true;
			}
			else if (basic[i] == "unsigned")
			{
				if (isUnSigned || isUnSigned)
				{
					addError(std::string("��Ч���������"));
				}
				isUnSigned = true;
			}
			else
			{
				addError(std::string("��Ч���������"));
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
				addError(std::string("��Ч���������"));
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
				addError(std::string("��Ч���������"));
			}
		}

		return 6;
	}

	addError(std::string("��Ч���������"));
	return 0;
}
