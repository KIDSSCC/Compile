#ifndef __UNIT_H__
#define __UNIT_H__

#include <vector>
#include "Function.h"
#include "AsmBuilder.h"

class Unit
{
    //声明了两个Function*类型的动态数组，更名为iterator和reverse_iterator，用作双向迭代的链表。
    typedef std::vector<Function *>::iterator iterator;
    typedef std::vector<Function *>::reverse_iterator reverse_iterator;

private:
    //一个私有的Function*类型的动态数组func_list。
    std::vector<Function *> func_list;

public:

    std::vector<IdentifierSymbolEntry*>constId_list;
    std::vector<SymbolEntry*> externFunction;

    //默认存在构造函数
    Unit() = default;
    ~Unit() ;

    //插入和移除
    void insertFunc(Function *);
    void removeFunc(Function *);
    //输出
    void output() const;

    //正向迭代器和反向迭代器
    iterator begin() { return func_list.begin(); };
    iterator end() { return func_list.end(); };
    reverse_iterator rbegin() { return func_list.rbegin(); };
    reverse_iterator rend() { return func_list.rend(); };
    void genMachineCode(MachineUnit* munit);
};

#endif