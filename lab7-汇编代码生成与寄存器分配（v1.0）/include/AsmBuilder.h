#ifndef __ASMBUILDER_H__
#define __ASMBUILDER_H__

#include "MachineCode.h"


class AsmBuilder
{
private:
    /*
    机器指令编译单元，当前正在编译的函数,正在编译的基本块，比较指令的操作码
    */
    MachineUnit* mUnit;  // mahicne unit
    MachineFunction* mFunction; // current machine code function;
    MachineBlock* mBlock; // current machine code block;
    int cmpOpcode; // CmpInstruction opcode, for CondInstruction;
public:
    /*
    以上四个变量的get与set
    */
    void setUnit(MachineUnit* unit) { this->mUnit = unit; };
    void setFunction(MachineFunction* func) { this->mFunction = func; };
    void setBlock(MachineBlock* block) { this->mBlock = block; };
    void setCmpOpcode(int opcode) { this->cmpOpcode = opcode; };
    MachineUnit* getUnit() { return this->mUnit; };
    MachineFunction* getFunction() { return this->mFunction; };
    MachineBlock* getBlock() { return this->mBlock; };
    int getCmpOpcode() { return this->cmpOpcode; };
};


#endif