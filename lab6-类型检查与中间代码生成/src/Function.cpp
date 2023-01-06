#include "Function.h"
#include "Unit.h"
#include "Type.h"
#include <list>

//负责输出的对象？
extern FILE* yyout;

//构造函数，向编译单元中插入一个函数对象，
Function::Function(Unit *u, SymbolEntry *s)
{
    sym_ptr = s;
    parent = u;
    entry = new BasicBlock(this);
    u->insertFunc(this);
    

}
/*
Function::~Function()
{
    auto delete_list = block_list;
    for (auto &i : delete_list)
        delete i;
    parent->removeFunc(this);
}*/

// remove the basicblock bb from its block_list.
void Function::remove(BasicBlock *bb)
{
    block_list.erase(std::find(block_list.begin(), block_list.end(), bb));
}

void Function::output() const
{
    
    //获取符号表项的类型并强制转换成FunctionType
    FunctionType* funcType = dynamic_cast<FunctionType*>(sym_ptr->getType());
    //获取函数的返回类型
    Type *retType = funcType->getRetType();
    //获取函数的参数列表
    std::vector<SymbolEntry*> params=funcType->getParams();
    
    fprintf(yyout, "define %s %s(",retType->toStr().c_str(), sym_ptr->toStr().c_str());
    //打印参数
    for(int i=0;i<(int)params.size();i++)
    {
        std::string type=params[i]->getType()->toStr();
        std::string name=params[i]->toStr();
        fprintf(yyout, "%s %s",type.c_str(),name.c_str());
        if(i!=(int)params.size()-1)
        {
            fprintf(yyout, ",");
        }
    }
    fprintf(yyout, ") {\n");

    //一个set容器，一个list容器。
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);
    v.insert(entry);
    while (!q.empty())
    {
        auto bb = q.front();
        q.pop_front();
        bb->output();
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            if (v.find(*succ) == v.end())
            {
                v.insert(*succ);
                q.push_back(*succ);
            }
        }
    }
    fprintf(yyout, "}\n");
}
