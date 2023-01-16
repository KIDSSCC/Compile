#include "Function.h"
#include "Unit.h"
#include "Type.h"
#include <list>

//负责输出的对象？
extern FILE* yyout;

//构造函数，向编译单元中插入一个函数对象，
Function::Function(Unit *u, SymbolEntry *s)
{
    u->insertFunc(this);
    entry = new BasicBlock(this);
    sym_ptr = s;
    parent = u;
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

void Function::genMachineCode(AsmBuilder* builder) 
{
    /*
    一个函数的汇编代码
    根据当前的汇编代码单元和函数的符号表项，创建机器指令函数单元，并设置为编译单元当前执行的函数
    */
    auto cur_unit = builder->getUnit();
    auto cur_func = new MachineFunction(cur_unit, this->sym_ptr);
    builder->setFunction(cur_func);
    /*
    中间代码基本块与汇编指令基本块的映射map
    */
    std::map<BasicBlock*, MachineBlock*> map;
    /*
    对于中间代码基本块列表中的每一个基本块，生成其汇编代码
    在进行每一个基本块的汇编代码生成时，会将汇编单元中的当前基本块设置为正在翻译的基本块，
    随后将其存储到map映射关系中，
    */
    for(auto block : block_list)
    {
        block->genMachineCode(builder);
        map[block] = builder->getBlock();
    }
    // Add pred and succ for every block
    /*
    根据map关系中的中间代码基本块与汇编代码基本块的映射关系，
    为汇编代码基本块之间补充前驱与后继关系，
    */
    for(auto block : block_list)
    {
        /*
        mblock是汇编代码基本块
        */
        auto mblock = map[block];
        for (auto pred = block->pred_begin(); pred != block->pred_end(); pred++)
            mblock->addPred(map[*pred]);
        for (auto succ = block->succ_begin(); succ != block->succ_end(); succ++)
            mblock->addSucc(map[*succ]);
    }
    /*
    完成如上工作，将当前函数插入到了编译单元所管理的函数列表中。
    */
    cur_unit->InsertFunc(cur_func);

}
