#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include "BasicBlock.h"
#include "SymbolTable.h"

//编译单元
class Unit;

class Function
{
    //仿照Unit中的双向迭代器结构，此时迭代对象为BasicBlock
    typedef std::vector<BasicBlock *>::iterator iterator;
    typedef std::vector<BasicBlock *>::reverse_iterator reverse_iterator;

private:
    //存储基本块的动态数组
    std::vector<BasicBlock *> block_list;
    //符号表项，这个符号表项应该是指函数名
    SymbolEntry *sym_ptr;
    //每个函数的入口块
    BasicBlock *entry;
    //所属的编译单元？
    Unit *parent;

public:
    //构造函数，需要编译单元和符号表项
    Function(Unit *, SymbolEntry *);
    //~Function();

    //插入一个基本块
    void insertBlock(BasicBlock *bb) { block_list.push_back(bb); };
    //获取函数入口块
    BasicBlock *getEntry() { return entry; };
    //删除某个基本块
    void remove(BasicBlock *bb);
    //打印
    void output() const;
    //获取全部的基本块
    std::vector<BasicBlock *> &getBlockList(){return block_list;};
    //双向迭代器
    iterator begin() { return block_list.begin(); };
    iterator end() { return block_list.end(); };
    reverse_iterator rbegin() { return block_list.rbegin(); };
    reverse_iterator rend() { return block_list.rend(); };
    //获取符号表项
    SymbolEntry *getSymPtr() { return sym_ptr; };
};

#endif
