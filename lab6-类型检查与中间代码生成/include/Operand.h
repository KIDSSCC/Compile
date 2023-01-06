#ifndef __OPERAND_H__
#define __OPERAND_H__

#include "SymbolTable.h"
#include <vector>
#include<iostream>

class Instruction;
class Function;

// class Operand - The operand of an instruction.
class Operand
{
    //操作数拿了指令的迭代器？
typedef std::vector<Instruction *>::iterator use_iterator;

private:
    //作为一个操作数来说，他是在哪里被定义的
    Instruction *def;                // The instruction where this operand is defined.
    //哪些指令用到了这个操作数
    std::vector<Instruction *> uses; // Intructions that use this operand.
    
    //这个操作数的符号表项
    SymbolEntry *se;                 // The symbol entry of this operand.
public:
    Operand(SymbolEntry*se) :se(se){def = nullptr;};
    void setDef(Instruction *inst) {def = inst;};


    void addUse(Instruction *inst) { uses.push_back(inst);};
    void removeUse(Instruction *inst);
    int usersNum() const {return uses.size();};

    use_iterator use_begin() {return uses.begin();};
    use_iterator use_end() {return uses.end();};
    Type* getType() {return se->getType();};
    std::string toStr() const;
};

#endif