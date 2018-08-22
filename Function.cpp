#include "Function.h"
#include "Stack.h"
#include "State.h"




InstructionValue::InstructionValue():
	_insSet(nullptr)
{

}


Function::Function()
{
}


Function::~Function()
{
}




Closure* Function::generateClosure(State* s)
{
	Closure* cl = new Closure(s);
	cl->setPrototype(this);
	return cl;
}





Closure::Closure(State* s)
	:_state(s), _realParamNum(0),
	_realRetNum(0), _needRetNum(0),
	_prototype(nullptr),
	_parentClosure(nullptr),
	_upTables(nullptr)
{

}

void Closure::initClosure()
{
	clearClosure();
	_nest_tables.push_back(new Table());
}

void Closure::clearClosure()
{
	_nest_tables.clear();
}

void Closure::balanceStack()
{
	int n = _realRetNum - _needRetNum;      //��ʵ����ֵ����-��Ҫ����ֵ����
	if (n > 0)  {                           //f() + 2,f()��ʵ������2������������ֻ��Ҫһ������ջҪ����һ��
		while (n > 0)  {
			_state->getStack()->popValue();
			n--;
		}
	}
	else  {
		while (n < 0)  {                    //f() + 2, f()��ʵ����0��������������Ҫһ������Ҫѹ��ջһ����ò�Ʋ�ѹҲ�У���Ϊ���渳ֵʱ��ƽ�⣩
			_state->getStack()->Push(new Nil());
			n++;
		}
	}
	_realRetNum = 0;
	_needRetNum = 0;
}

void Closure::addBlockTable()
{
	_nest_tables.push_back(new Table());
}

void Closure::removeBlockTable()
{
	Table* top = getTopTable();
	if (top)  {
		delete top;
	}
	_nest_tables.pop_back();
}

Table* Closure::getTopTable()
{
	if (_nest_tables.size() == 0)  {
		return nullptr;
	}
	Table* top = _nest_tables.back();
	return top;
}

void Closure::setParentClosure(Closure* c)
{
	_parentClosure = c;
	if (c)  {
		Table* topTab = c->getTopTable();       //ֻ�追�������ľֲ�����,forѭ������ľֲ��������Ӻ����ǲ��ɼ���
		if (topTab)  {
			_upTables = topTab->clone();
		}
	}
}


int Closure::findInNestTables(Value* key, Value** val)
{
	int num = _nest_tables.size();
	int level = 0;
	Value* temp = nullptr;
	for (int i = num - 1; i >= 0; i--)  {
		temp = _nest_tables[i]->GetValue(key);
		if (temp)  {
			if (val) *val = temp;
			return level + 1;
		}
		level++;
	}

	return -1;
}

int Closure::findUpTables(Value* key, Value** val, Table** tab)
{
	Closure* cl = this;
	while (cl)  {
		int ret = cl->findInNestTables(key, val);
		if (ret != -1)  {  //�ڵ�ǰ�հ����ҵ�
			if (tab) *tab = cl->getTopTable();
			return 1;
		}
		cl = cl->_parentClosure;
	}

	if (_upTables)  {      //������ֵ����
		Value* temp = _upTables->GetValue(key);
		if (temp) { 
			if (val) *val = temp;
			if (tab) *tab = _upTables;
			return 2;
		}
	}

	Table* table = getState()->getGlobalTable();  //�ѵ������
	Value* temp = table->GetValue(key);
	if (temp)  {
		if (val) *val = temp;
		if (tab) *tab = table;
		return 0;
	}
	else  {
		return -1;
	}
	return -1;
}