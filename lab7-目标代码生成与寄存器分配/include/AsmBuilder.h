#ifndef __ASMBUILDER_H__
#define __ASMBUILDER_H__

#include "MachineCode.h"


class AsmBuilder    //中间代码向目标代码进行自顶向下的转换时，记录当前正在进行转换操作的对象，以进行函数、基本块及指令的插入。
{
private:
    /*
    机器指令编译单元，当前正在编译的函数,正在编译的基本块，比较指令的操作码
    */
    MachineUnit* mUnit;  // mahicne unit                            向当前单元中插入函数体
    MachineFunction* mFunction; // current machine code function;   向当前函数体中插入基本块
    MachineBlock* mBlock; // current machine code block;            向当前块中插入指令
    int cmpOpcode; // CmpInstruction opcode, for CondInstruction;   为啥单独写出来？？
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