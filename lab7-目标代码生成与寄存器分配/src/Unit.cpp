#include "Unit.h"
#include "Type.h"//lab6新加3行
#include<iostream>
#include <stack>
extern FILE* yyout;
//把一个function*插入到动态数组后面。
void Unit::insertFunc(Function *f)
{
    func_list.push_back(f);
}

//把一个function*从动态数组中删掉。
void Unit::removeFunc(Function *func)
{
    func_list.erase(std::find(func_list.begin(), func_list.end(), func));
}

//输出函数，调用了func_list每个节点的打印函数
void Unit::output() const
{
    

    try
    {
        /* code */
    for(int i=0;i<(int)constId_list.size();i++)
    {
        if(constId_list[i]->getType()->isArray()){//新加数组，完善浮点
            fprintf(yyout, "%s = global ", constId_list[i]->toStr().c_str());
            ArrayType* type = (ArrayType*)(constId_list[i]->getType());
            if(type->getFinalType()->isFloat()) {
                float* val = ((IdentifierSymbolEntry*)constId_list[i])->getFloatArrayValue();
                int i = 0;
                if (((IdentifierSymbolEntry*)constId_list[i])->ifquan0()) {//改过来！！！((IdentifierSymbolEntry*)se)->isAllZero()
                    fprintf(yyout, "%s zeroinitializer", type->toStr().c_str());
                } 
                else {
                    std::stack<ArrayType*> stk;
                    std::stack<int> stk1;
                    stk.push(type);
                    stk1.push(0);
                    ArrayType* temp;
                    while (!stk.empty()) {
                        temp = stk.top();
                        if (temp->getArrayType()->isFloat()) {
                            fprintf(yyout, "%s [", temp->toStr().c_str());
                            for (int j = 0; j < temp->getArrayNum(); j++) {
                                if (j != 0)
                                    fprintf(yyout, ", ");
                                fprintf(yyout, "float %f", val[i++]);
                            }
                            fprintf(yyout, "]");
                            stk1.pop();
                            stk.pop();
                            if(stk.empty())
                                break;
                            stk1.top()++;
                            continue;
                        }
                        if (stk1.top() != temp->getArrayNum()) {
                            stk.push((ArrayType*)(temp->getArrayType()));
                            if (stk1.top() == 0)
                                fprintf(yyout, "%s [", temp->toStr().c_str());
                            if (stk1.top() != 0)
                                fprintf(yyout, ", ");
                            stk1.push(0);
                        } else {
                            fprintf(yyout, "]");
                            stk.pop();
                            stk1.pop();
                            if (stk.empty())
                                break;
                            stk1.top()++;
                            continue;
                        }
                    }
                }  
            }
            else{//IsInt()
                int* val = ((IdentifierSymbolEntry*)constId_list[i])->getIntArrayValue();
                int i = 0;
                if (((IdentifierSymbolEntry*)constId_list[i])->ifquan0()) {//改过来！！！((IdentifierSymbolEntry*)se)->isAllZero()
                    fprintf(yyout, "%s zeroinitializer", type->toStr().c_str());
                } 
                else {
                    std::stack<ArrayType*> stk;
                    std::stack<int> stk1;
                    stk.push(type);
                    stk1.push(0);
                    ArrayType* temp;
                    while (!stk.empty()) {
                        temp = stk.top();
                        if (temp->getArrayType()->isInt()) {
                            fprintf(yyout, "%s [", temp->toStr().c_str());
                            for (int j = 0; j < temp->getArrayNum(); j++) {
                                if (j != 0)
                                    fprintf(yyout, ", ");
                                fprintf(yyout, "i32 %d", val[i++]);
                            }
                            fprintf(yyout, "]");
                            stk1.pop();
                            stk.pop();
                            if(stk.empty())
                                break;
                            stk1.top()++;
                            continue;
                        }
                        if (stk1.top() != temp->getArrayNum()) {
                            stk.push((ArrayType*)(temp->getArrayType()));
                            if (stk1.top() == 0)
                                fprintf(yyout, "%s [", temp->toStr().c_str());
                            if (stk1.top() != 0)
                                fprintf(yyout, ", ");
                            stk1.push(0);
                        } else {
                            fprintf(yyout, "]");
                            stk.pop();
                            stk1.pop();
                            if (stk.empty())
                                break;
                            stk1.top()++;
                            continue;
                        }
                    }
                }
            }
            
            fprintf(yyout, ", align 4\n");

        }
        else{
            std::string name=constId_list[i]->toStr();
            auto type=constId_list[i]->getType()->toStr().c_str();
            if(constId_list[i]->getType()->isFloat()) { 
                double fvalue=constId_list[i]->getFloatValue();//float不行！！
                uint64_t val = reinterpret_cast<uint64_t&>(fvalue);//debug
                fprintf(yyout, "%s = global %s 0x%lX, align 4\n", name.c_str(),type,val);//debug
            }
            else{
                auto value=constId_list[i]->getIntValue();
                fprintf(yyout, "%s = global %s %d, align 4\n", name.c_str(),type,value);
            }
            
        }
        
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
    catch(const std::bad_alloc& e)
    {
        std::cerr << e.what() << '\n';
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

//析构
Unit::~Unit()
{
    for(auto &func:func_list)
        delete func;
}
