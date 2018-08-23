#include "VM.h"
#include <assert.h>
#include "Value.h"
#include "State.h"
#include "Function.h"
#include "Stack.h"






void PrintType(Value* val)
{
	if (val->Type() == Value::TYPE_STRING)  {
		printf("%s", ((String*)val)->Get().c_str());
	}
	else if (val->Type() == Value::TYPE_NUMBER)  {
		if (((Number*)val)->IsInteger())  {
			printf("%d", ((Number*)val)->GetInteger());
		}
		else  {
			printf("%f", ((Number*)val)->Get());
		}
	}
	else if (val->Type() == Value::TYPE_NIL)  {
		printf("nil");
	}
	else if (val->Type() == Value::TYPE_CLOSURE)  {
		printf("funciont: 0x%08x", val);
	}
	else if (val->Type() == Value::TYPE_TABLE)  {
		printf("table: 0x%08x", val);
	}
}





int Print(State* state, void* num)
{
	std::vector<Value*> vtVals;
	for (int i = (int)num - 1; i >= 0; i--)   {
		Value* val = state->getStack()->popValue();
		vtVals.push_back(val);
	}

	for (int i = (int)num - 1; i >= 0; i--)   {
		Value* val = vtVals[i];
		if (val)  {
			PrintType(val);
		}
		printf("\t");
	}
	printf("\n");
	return 0;
}






VM::VM(State* state)
	: _state(state),
	_stackClosure(new Stack(20)),
	_curInsVal(nullptr)
{
	_stack = _state->getStack();
	registerFunc();
}


VM::~VM()
{
}

void VM::execute()
{
}


void VM::execute_frame()
{

}


void VM::registerFunc()
{
	_state->registerFunc(Print);
}


int VM::runCode(InstructionValue* insSetVal)
{
	insSetVal->setParent(_curInsVal);
	_curInsVal = insSetVal;
	auto& vtIns = insSetVal->getInstructionSet()->toVtInstructions();
	for (auto it = vtIns.begin(); it != vtIns.end(); ++it)  {
		if (insSetVal->getBreaked())  {
			printf("breaked!!!\n");
			return 0;
		}
		Instruction* ins = *it;
		switch (ins->op_code)
		{
		case Instruction::OpCode_AddGlobalTable:
			add_global_table();
			break;
		case Instruction::OpCode_EnterClosure:         //���뺯��ʱ����
			enterClosure();
			break;

		case Instruction::OpCode_QuitClosure:
			quitClosure();
			break;

		case Instruction::OpCode_InitLocalVar:         //�ֲ���������
			initLocalVar(ins);
			break;
		case Instruction::OpCode_GetLocalVar:             
			getLoacalVar(ins);
			break;

		case Instruction::OpCode_SetLocalVar:             
			setLoacalVar(ins);
			break;

		case Instruction::OpCode_GenerateClosure:
			generateClosure(ins);
			break;

		case Instruction::OpCode_PassFunParam:
			passFunParam(ins);
			break;

		case Instruction::OpCode_Call:
			call(ins);
			break;

		case Instruction::OpCode_Ret:
			funcionRet(ins);
			break;

		case Instruction::OpCode_Push:
			pushValue(ins);
			break;

		case Instruction::OpCode_Assign:
			assignOperate(ins);
			break;

		case Instruction::OpCode_Plus:
		case Instruction::OpCode_Minus:
		case Instruction::OpCode_Multiply:
		case Instruction::OpCode_Divide:
			operateNum(ins);
			break;

		case Instruction::OpCode_If:
			ifCompare(ins);
			break;

		case Instruction::OpCode_For:
			forCompare(ins);
			break;

		case Instruction::OpCode_Break:
			//breakFor(ins);
			return -1;
			//break;

		case Instruction::OpCode_Less:
		case Instruction::OpCode_Greater:
		case Instruction::OpCode_LessEqual:
		case Instruction::OpCode_GreaterEqual:
		case Instruction::OpCode_NotEqual:
		case Instruction::OpCode_Equal:
			operateLogic(ins);
			break;

		case Instruction::OpCode_EnterBlock:
			enterBlock(ins);
			break;

		case Instruction::OpCode_QuitBlock:
			quitBlock(ins);
			break;

		case Instruction::OpCode_GenerateBlock:
			generateBlock(ins);
			break;

		case Instruction::OpCode_TableDefine:
			tableDefine(ins);
			break;

		case Instruction::OpCode_TableMemAccess:
			tableAccess(ins);
			break;

		case Instruction::OpCode_TableArrIndex:
			tableArrIndex(ins);
			break;
		
		default:
			break;
		}
	}
	return 0;
}

void VM::generateClosure(Instruction* ins)
{
	assert(ins->param_a.param.value->Type() == Value::TYPE_FUNCTION);
	Function *func = static_cast<Function *>(ins->param_a.param.value);
	Closure *cl = func->generateClosure(_state) ;
	cl->setParentClosure(getCurrentClosure());
	_stack->Push(cl);
}

void VM::add_global_table()
{

}

void VM::call(Instruction* ins)
{
	int paramNum = ins->param_a.param.counter.counter1;
	Value* callee = _stack->popValue();

	if (callee->Type() == Value::TYPE_NIL)  {
		printf("error, call a nil val");
	}

	if (callee->Type() == Value::TYPE_NATIVE_FUNCTION)  {
		((NativeFunc*)callee)->doCall(_state, (void*)paramNum);
	}
	else if (callee->Type() == Value::TYPE_CLOSURE)  {
		Closure* cl = static_cast<Closure*>(callee);
		_stackClosure->Push(cl);
		cl->setRealParamNum(paramNum);
		cl->setNeedRetNum(ins->param_a.param.counter.counter2);
		runCode(cl->getPrototype());
	}
}

void VM::enterClosure()
{
	getCurrentClosure()->initClosure();
};

void VM::quitClosure()
{
	Closure* cl = getCurrentClosure();
	cl->clearClosure();
	cl->balanceStack();
	_stackClosure->popValue();
}

Closure* VM::getCurrentClosure()
{
	if (_stackClosure->Size() == 0)  {
		return nullptr;
	}
	Closure* cl = (Closure*)_stackClosure->Top()->param.value;
	assert(cl->Type() == Value::TYPE_CLOSURE);
	return cl;
}

void VM::passFunParam(Instruction* ins)
{
	int needParamNum = ins->param_a.param.counter.counter1;
	int realParamNum = getCurrentClosure()->getRealParamNum();
	int n = needParamNum - realParamNum;
	if (n > 0)  {
		while (n > 0)  {
			_stack->Push(new Nil());
			n--;
		}
	}
	
	assignVals(needParamNum, realParamNum, 0);               //���ѹ���ֵ�Ȳ����࣬����ᴦ���
}

void VM::assignVals(int num_key, int num_val, int type)      //�������ô������ʱҲ���������
{
	Table* tab = getCurrentClosure()->getTopTable();
	std::list<Value*> listKeys;
	std::list<Value*> listVals;
	for (int i = 0; i < num_key; i++)  {
		listKeys.push_front(_stack->popValue());
	}
	if (num_key > num_val)  {                   //ֵ����key���п����Ǻ��������a��b =f(),��൯��key�ĸ�������Ϊջ�Ͽ����������ط���ֵ
		num_val = (unsigned int)num_key < _stack->Size() ? num_key : _stack->Size();
	}
	for (int i = 0; i < num_val; i++)    {      //Ҫ��ʣ�µ�ֵȫ������
		Value* val = _stack->popValue();
		listVals.push_front(val);
	}
	num_val = listVals.size();
	if (num_key > num_val)  {     //a,b,c = 1, 2
		for (int i = 0; i < num_key - num_val; i++)  {
			listVals.push_back(new Nil());
		}
	}

	while (!listKeys.empty() && !listVals.empty())  {     //�п����ж���Ĳ���������
		Value* key = listKeys.front();
		Value* val = listVals.front();
		if (type == 1)  {   
			if (getCurrentClosure()->findUpTables(key, nullptr, &tab) == -1)  {      //��ֵʱû���ҵ��ͷ���ȫ�ֱ���
				tab = _state->getGlobalTable();
			}
		}  
		tab->Assign(key, val);
		listKeys.pop_front();
		listVals.pop_front();
	}
}

void VM::initLocalVar(Instruction* ins)
{
	int num_key = ins->param_a.param.counter.counter1;
	int num_val = ins->param_a.param.counter.counter2;
	if (num_key == 1 && num_val == 1)  {
		assignSimple(0);
	}
	else  {
		assignVals(num_key, num_val, 0);
	}
}

void VM::assignOperate(Instruction* ins)
{
	int num_key = ins->param_a.param.counter.counter1;
	int num_val = ins->param_a.param.counter.counter2;
	if (num_key == 1 && num_val == 1)  {
		assignSimple(1);
	}
	else  {
		assignVals(num_key, num_val, 1);
	}
}

void VM::assignSimple(int type)
{
	Table* tab = getCurrentClosure()->getTopTable();
	Value* key = _stack->popValue();
	Value* val = _stack->popValue();
	if (type == 1 && getCurrentClosure()->findUpTables(key, nullptr, &tab) == -1)  {      //��ֵʱû���ҵ��ͷ���ȫ�ֱ���
		tab = _state->getGlobalTable();
	}
	tab->Assign(key, val);
}

void VM::get_table(Instruction* ins)
{
	Value* key = ins->param_a.param.name;
	Table* table = getCurrentClosure()->getTopTable();
	Value* val = table->GetValue(key);
	if (val)  {
		_stack->Push(val);
	}
	
}

void VM::setLoacalVar(Instruction* ins)
{
	_stack->Push(ins->param_a.param.name);
}

void VM::getLoacalVar(Instruction* ins)
{
	Value* key = ins->param_a.param.name;
	Value* val = nullptr;
	if (getCurrentClosure()->findUpTables(key, &val, nullptr) != -1)  {
		_stack->Push(val);
	}
	else  {
		val = new Nil();
		_stack->Push(val);
	}
}

void VM::pushValue(Instruction* ins)
{
	if (ins->param_a.type == InstructionParam::InstructionParamType_Name)  {
		_stack->Push(ins->param_a.param.name);
	}  
	else if (ins->param_a.type == InstructionParam::InstructionParamType_Value)  {
		_stack->Push(ins->param_a.param.value);
	}
}

void VM::operateNum(Instruction* ins)
{
	Value* num1 = _stack->popValue();
	Value* num2 = _stack->popValue();
	
	if (num1->Type() == Value::TYPE_NIL ||
		num2->Type() == Value::TYPE_NIL)  {
		printf("error, operate on a nil value\n");
	}

	double num = 0;
	if (ins->op_code == Instruction::OpCode_Plus)  {
		num = ((Number*)num2)->Get() + ((Number*)num1)->Get();
	}
	else if (ins->op_code == Instruction::OpCode_Minus)  {
		num = ((Number*)num2)->Get() - ((Number*)num1)->Get();
	}
	else if (ins->op_code == Instruction::OpCode_Multiply)  {
		num = ((Number*)num2)->Get() * ((Number*)num1)->Get();
	}
	else if (ins->op_code == Instruction::OpCode_Divide)  {
		num = ((Number*)num2)->Get() / ((Number*)num1)->Get();
	}
	
	Value* ret = new Number(num);
	_stack->Push(ret);
}

void VM::operateLogic(Instruction* ins)
{
	Number* right = (Number*)_stack->popValue();
	Number* left = (Number*)_stack->popValue();
	BoolValue* retLogic = new BoolValue();
	double num1 = left->Get();
	double num2 = right->Get();
	if (ins->op_code == Instruction::OpCode_Less)  {
		retLogic->setLogicVal(num1 < num2);
	}
	else if (ins->op_code == Instruction::OpCode_Greater)  {
		retLogic->setLogicVal(num1 > num2);
	}
	else if (ins->op_code == Instruction::OpCode_LessEqual)  {
		retLogic->setLogicVal(num1 <= num2);
	}
	else if (ins->op_code == Instruction::OpCode_GreaterEqual)  {
		retLogic->setLogicVal(num1 >= num2);
	}
	else if (ins->op_code == Instruction::OpCode_Equal)  {
		retLogic->setLogicVal(num1 == num2);
	}
	else if (ins->op_code == Instruction::OpCode_NotEqual)  {
		retLogic->setLogicVal(num1 != num2);
	}
	_stack->Push(retLogic);
}

void VM::funcionRet(Instruction* ins)
{
 	int num = ins->param_a.param.counter.counter1;
	Closure* cl = getCurrentClosure();
	cl->setRealRetNum(num);
}

void VM::ifCompare(Instruction* ins)
{
	Value* logic = _stack->popValue();
	InstructionValue* leftBlock = (InstructionValue*)_stack->popValue();

	InstructionValue* rightBlock = nullptr;
	if (ins->param_a.param.counter.counter1 > 0)  {
		rightBlock = (InstructionValue*)_stack->popValue();
	}
	bool runLeft = true;                //����nil��false����ȫΪtrue
	if (logic->Type() == Value::TYPE_NIL)  {
		runLeft = false;
	}
	else {
		if (logic->Type() == Value::TYPE_BOOL)  {
			runLeft = ((BoolValue*)logic)->getLogicVal();
		}
	}
	if (runLeft)  {
		runBlockCode(leftBlock);
	}
	else  {
		if (rightBlock)  {
			runBlockCode(rightBlock);
		}
	}
}

void VM::forCompare(Instruction* ins)
{
	Value* valStart = _stack->popValue();
	Table* top = getCurrentClosure()->getTopTable();
	Number* numStart = (Number*)top->GetValue(valStart);
	int iStart = ((Number*)top->GetValue(valStart))->GetInteger();
	int iEnd = ((Number*)_stack->popValue())->GetInteger();
	int iStep = 1;
	if (ins->param_a.param.counter.counter1 > 0)  {
		iStep = ((Number*)_stack->popValue())->GetInteger();
	}
	InstructionValue* block = (InstructionValue*)_stack->popValue();
	block->setFor(true);

	auto xInc = [](int i, int end) {return i <= end; };
	auto xDec = [](int i, int end) {return i >= end; };
	typedef bool(*Cmp)(int, int);
	Cmp xCmp = xInc;
	if (iStep < 0) {
		xCmp = xDec;
	}
	for (int i = iStart; xCmp(i, iEnd); i += iStep)  {
		numStart->SetNumber(i);
		top->Assign(valStart, numStart);
		runBlockCode(block);
		if (block->getBreaked())  {
			break;
		}
	}
}

void VM::breakFor(Instruction* ins)
{

}

void VM::enterBlock(Instruction* ins)
{
	getCurrentClosure()->addBlockTable();
}

void VM::quitBlock(Instruction* ins)
{
	getCurrentClosure()->removeBlockTable();
}

void VM::runBlockCode(Value* val)
{
	if (val)  {
		assert(val->Type() == Value::TYPE_INSTRUCTVAL);
		getCurrentClosure()->addBlockTable();
		int ret = runCode((InstructionValue*)val);
		getCurrentClosure()->removeBlockTable();
		if (ret == -1)  {
			((InstructionValue*)val)->setBreaked(true);
			InstructionValue* p = (InstructionValue*)val;
			while (p)  {
				p = p->getParent();
				if (p)  {
					p->setBreaked(true);
					if (p->getFor())  {
						break;
					}
				}
			}
			if (!p)  {    //�ҵ�ͷ�˶�û���ҵ�ѭ��
				printf("run error, break in no loop block!!!\n");
			}
		}
	}
}

void VM::generateBlock(Instruction* ins)
{
	Value* val = ins->param_a.param.value;
	_stack->Push(val);
}

void VM::tableDefine(Instruction* ins)
{
	Table* tab = new Table();
	for (int i = 0; i < ins->param_a.param.counter.counter1; i++)  {
		Value* key = _stack->popValue();
		Value* val = _stack->popValue();
		tab->Assign(key, val);
	}
	_stack->Push(tab);
}

void VM::tableArrIndex(Instruction* ins)
{
	int index = ins->param_a.param.array_index;
	_stack->Push(new Number(index));
}

void VM::tableAccess(Instruction* ins)
{
	Value* tabName = _stack->popValue();
	Value* member = _stack->popValue();
	Value* tab = nullptr;
	Value* val = nullptr;
	std::string stFiled = ((String*)(ins->param_a.param.value))->Get();

	if (tabName->Type() == Value::TYPE_TABLE)  {    //a.b.c ��ôa.b����table
		val = ((Table*)tabName)->GetValue(member);
	}
	else if (tabName->Type() == Value::TYPE_NUMBER)  {
		printf("attempt to index a number val (filed \'%s\') \n", stFiled.c_str());
	}
	else  {
		if (getCurrentClosure()->findUpTables(tabName, &tab, nullptr) != -1)  {
			if (tab->Type() != Value::TYPE_TABLE)  {
				printf("%s is not a table\n", ((String*)tabName)->Get().c_str());
			}
			val = ((Table*)tab)->GetValue(member);
		}
		else  {
			if (tabName->Type() == Value::TYPE_NUMBER)  {
				printf("attempt to index a number val (filed \'%s\') \n", stFiled.c_str());
			}
			else if (tabName->Type() == Value::TYPE_NIL)  {
				printf("attempt to index a nil val (filed \'%s\') \n", stFiled.c_str());
			}
		}
	}
	if (!val)  {       //a.b �Ҳ���a��b����Ϊ��
		val = new Nil();
	}
	
	_stack->Push(val);
}