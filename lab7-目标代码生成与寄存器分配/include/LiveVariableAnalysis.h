#ifndef __LIVE_VARIABLE_ANALYSIS_H__
#define __LIVE_VARIABLE_ANALYSIS_H__

#include <set>
#include <map>
//活跃变量分析，用于寄存器分配过程
class MachineFunction;
class MachineUnit;
class MachineOperand;
class MachineBlock;
class LiveVariableAnalysis
{
private:
    /*
    两个map集合，一个操作数对一个操作数set集合
    一个基本块对一个def和use操作数set集合
    */
    std::map<MachineOperand, std::set<MachineOperand *>> all_uses;
    std::map<MachineBlock *, std::set<MachineOperand *>> def, use;
    //对一个函数来说，记录下了这个函数所有指令use_list中的操作数
    void computeUsePos(MachineFunction *);
    //关于set容器，查一下，
    void computeDefUse(MachineFunction *);
    //对函数基本块中的livein和liveout集合的操作，livein集合来自该块使用的操作数，liveout承接自后继块
    void iterate(MachineFunction *);

public:
    //对于一个编译单元处理其所有函数
    void pass(MachineUnit *unit);
    //处理一个函数
    void pass(MachineFunction *func);
    std::map<MachineOperand, std::set<MachineOperand *>> &getAllUses() { return all_uses; };
};

#endif