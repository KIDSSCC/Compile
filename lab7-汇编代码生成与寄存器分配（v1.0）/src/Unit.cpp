#include "Unit.h"
#include "Type.h"
#include<iostream>
extern FILE* yyout;

void Unit::insertFunc(Function *f)
{
    func_list.push_back(f);
}

void Unit::removeFunc(Function *func)
{
    func_list.erase(std::find(func_list.begin(), func_list.end(), func));
}

void Unit::output() const
{
    

    
        /* code */
    for(int i=0;i<(int)constId_list.size();i++)
    {
        std::string name=constId_list[i]->toStr();
        auto type=constId_list[i]->getType()->toStr().c_str();
        auto value=constId_list[i]->getIntValue();
        fprintf(yyout, "%s = global %s %d, align 4\n", name.c_str(),type,value);
    }

    for(int i=0;i<(int)func_list.size();i++)
    {
        func_list[i]->output();
    }

    /*
    for (auto &func : func_list)
        func->output();
    */
    for(int i=0;i<(int)externFunction.size();i++)
    {
        FunctionType* type = (FunctionType*)(externFunction[i]->getType());
        std::string str = type->toStr();
        std::string name = str.substr(0, str.find('('));
        std::string param = str.substr(str.find('('));
        fprintf(yyout, "declare %s %s%s\n", type->getRetType()->toStr().c_str(),externFunction[i]->toStr().c_str(), param.c_str());
    }
    
}

void Unit::genMachineCode(MachineUnit* munit) 
{
    /*
    对于每个函数，生成其汇编代码
    */
    AsmBuilder* builder = new AsmBuilder();
    builder->setUnit(munit);
    for (auto &func : func_list)
        func->genMachineCode(builder);
}

Unit::~Unit()
{
    for(auto &func:func_list)
        delete func;
}


