/**
 * linear scan register allocation
 */

#ifndef _LINEARSCAN_H__
#define _LINEARSCAN_H__
#include <set>
#include <map>
#include <vector>
#include <list>

class MachineUnit;
class MachineOperand;
class MachineFunction;

//为线性扫描寄存器分配算法相关类
class LinearScan
{
private:
    struct Interval
    {
        int start;
        int end;
        bool spill; // whether this vreg should be spilled to memory
        int disp;   // displacement in stack
        int rreg;   // the real register mapped from virtual register if the vreg is not spilled to memory
        std::set<MachineOperand *> defs;
        std::set<MachineOperand *> uses;
    };
    /*
    有所属的编译单元，
    函数：存疑
    寄存器的动态数组：初始压入了4到10
    */
    MachineUnit *unit;
    MachineFunction *func;
    std::vector<int> regs;
    std::map<MachineOperand *, std::set<MachineOperand *>> du_chains;
    std::vector<Interval*> intervals;
    std::vector<Interval*> active;
    /*
    compareStart：比较a的start是不是小于b的start
    expireOldIntervals：todo
    spillAtInterval：todo
    makeDuChains：存疑
    computeLiveIntervals：存疑
    linearScanRegisterAllocation：todo
    modifyCode：存疑
    genSpillCode：todo
    */
    static bool compareStart(Interval*a, Interval*b);
    static bool compareEnd(Interval* a, Interval* b);
    void expireOldIntervals(Interval *interval);
    void spillAtInterval(Interval *interval);
    void makeDuChains();
    void computeLiveIntervals();
    bool linearScanRegisterAllocation();
    void modifyCode();
    void genSpillCode();
public:
    LinearScan(MachineUnit *unit);
    void allocateRegisters();
};

#endif