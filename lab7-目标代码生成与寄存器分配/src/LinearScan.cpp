#include <algorithm>
#include<iterator>//新加
#include "LinearScan.h"
#include <iostream>
#include "MachineCode.h"
#include "LiveVariableAnalysis.h"

LinearScan::LinearScan(MachineUnit *unit)
{
    this->unit = unit;
    for (int i = 4; i < 11; i++)
        regs.push_back(i);
}

void LinearScan::allocateRegisters()
{
    /*
    在main.cpp中调用allocateRegisters
    遍历每一个函数
    */
    for (auto &f : unit->getFuncs())
    {
        func = f;
        bool success;
        success = false;
        /*
        在所有虚拟寄存器得到映射之前，一直在这里循环
        */
        while (!success)        // repeat until all vregs can be mapped
        {
            computeLiveIntervals();
            success = linearScanRegisterAllocation();
            if (success)        // all vregs can be mapped to real regs
                modifyCode();
            else                // spill vregs that can't be mapped to real regs
                genSpillCode();
        }
    }
}

void LinearScan::makeDuChains()
{
    /*
    创建了线性分析单元，并对当前的函数调用了pass
    */
    LiveVariableAnalysis lva;
    lva.pass(func);
    du_chains.clear();
    int i = 0;
    std::map<MachineOperand, std::set<MachineOperand *>> liveVar;
    for (auto &bb : func->getBlocks())
    {
        liveVar.clear();
        //每一个t是一个operand
        for (auto &t : bb->getLiveOut())
            liveVar[*t].insert(t);
        int no;
        //no应该记载了当前总的指令数
        no = i = bb->getInsts().size() + i;
        //逆序遍历所有指令
        for (auto inst = bb->getInsts().rbegin(); inst != bb->getInsts().rend(); inst++)
        {
            //设置每一条指令的编号
            (*inst)->setNo(no--);
            for (auto &def : (*inst)->getDef())
            {
                if (def->isVReg())
                {
                    auto &uses = liveVar[*def];
                    du_chains[def].insert(uses.begin(), uses.end());
                    auto &kill = lva.getAllUses()[*def];
                    std::set<MachineOperand *> res;
                    set_difference(uses.begin(), uses.end(), kill.begin(), kill.end(), inserter(res, res.end()));
                    liveVar[*def] = res;
                }
            }
            for (auto &use : (*inst)->getUse())
            {
                if (use->isVReg())
                    liveVar[*use].insert(use);
            }
        }
    }
}

void LinearScan::computeLiveIntervals()
{
    makeDuChains();
    intervals.clear();
    for (auto &du_chain : du_chains)
    {
        int t = -1;
        for (auto &use : du_chain.second)
            t = std::max(t, use->getParent()->getNo());
        Interval *interval = new Interval({du_chain.first->getParent()->getNo(), t, false, 0, 0, {du_chain.first}, du_chain.second});
        intervals.push_back(interval);
    }
    bool change;
    change = true;
    while (change)
    {
        change = false;
        std::vector<Interval *> t(intervals.begin(), intervals.end());
        for (size_t i = 0; i < t.size(); i++)
            for (size_t j = i + 1; j < t.size(); j++)
            {
                Interval *w1 = t[i];
                Interval *w2 = t[j];
                if (**w1->defs.begin() == **w2->defs.begin())
                {
                    std::set<MachineOperand *> temp;
                    set_intersection(w1->uses.begin(), w1->uses.end(), w2->uses.begin(), w2->uses.end(), inserter(temp, temp.end()));
                    if (!temp.empty())
                    {
                        change = true;
                        w1->defs.insert(w2->defs.begin(), w2->defs.end());
                        w1->uses.insert(w2->uses.begin(), w2->uses.end());
                        // w1->start = std::min(w1->start, w2->start);
                        // w1->end = std::max(w1->end, w2->end);
                        auto w1Min = std::min(w1->start, w1->end);
                        auto w1Max = std::max(w1->start, w1->end);
                        auto w2Min = std::min(w2->start, w2->end);
                        auto w2Max = std::max(w2->start, w2->end);
                        w1->start = std::min(w1Min, w2Min);
                        w1->end = std::max(w1Max, w2Max);
                        auto it = std::find(intervals.begin(), intervals.end(), w2);
                        if (it != intervals.end())
                            intervals.erase(it);
                    }
                }
            }
    }
    sort(intervals.begin(), intervals.end(), compareStart);
}

bool LinearScan::linearScanRegisterAllocation()
{
    // Todo
    bool whetherSpill = true;
    active.clear();
    regs.clear();
    for (int i = 4; i < 11; i++)
    {
        regs.push_back(i);
    }
    for(long unsigned int i=0;i<intervals.size();i++)
    {
        //对每个活跃区间
        expireOldIntervals(intervals[i]);
        //是否还有可用的物理寄存器
        if(regs.size()==0)
        {
            //寄存器溢出操作
            spillAtInterval(intervals[i]);
            whetherSpill=false;
        }
        else
        {
            //还有可用的物理寄存器，进行分配
            intervals[i]->rreg=regs[0];
            active.push_back(intervals[i]);
            regs.erase(regs.begin());

            for(long unsigned int i=0;i<active.size();i++)
            {
                for (long unsigned int j=active.size()-1;j>i;j--)
                {
                    if(active[j]->end<active[j-1]->end)
                    {
                        Interval* tmp=active[j-1];
                        active[j-1]=active[j];
                        active[j]=tmp;
                    }
                }
            }
        }
    }
    return whetherSpill;
}


void LinearScan::modifyCode()
{
    for (auto &interval : intervals)
    {
        func->addSavedRegs(interval->rreg);
        for (auto def : interval->defs)
            def->setReg(interval->rreg);
        for (auto use : interval->uses)
            use->setReg(interval->rreg);
    }
}

void LinearScan::genSpillCode()
{
    for(auto &interval:intervals)
    {
        if(!interval->spill)
            continue;
        // TODO
        /* HINT:
         * The vreg should be spilled to memory.
         * 1. insert ldr inst before the use of vreg
         * 2. insert str inst after the def of vreg
         */ 
        //生成溢出代码
        //分配空间，获取当前在栈内相对于fp的偏移
        interval->disp = -func->AllocSpace(4);
        auto fp = new MachineOperand(MachineOperand::REG, 11);
        auto off = new MachineOperand(MachineOperand::IMM, interval->disp);
        //对use列表中的操作数插入load指令
        std::set<MachineOperand *>::iterator use;
        for(use=interval->uses.begin();use!=interval->uses.end();use++)
        {
            MachineOperand* dst=new MachineOperand(*(*use));
            MachineInstruction* new_inst;
            //可以添加范围的判断
            MachineOperand*interval_reg = new MachineOperand(MachineOperand::VREG,SymbolTable::getLabel());
            new_inst = new LoadMInstruction((*use)->getParent()->getParent(),interval_reg, off);
            (*use)->getParent()->insertBefore(new_inst);
            auto src=new MachineOperand(*interval_reg);
            new_inst=new LoadMInstruction((*use)->getParent()->getParent(), dst,fp, src);
            (*use)->getParent()->insertBefore(new_inst);
        }
        //对def列表中的操作数插入store指令
        std::set<MachineOperand *>::iterator def;
        for(def=interval->defs.begin();def!=interval->defs.end();def++)
        {
            MachineOperand* dst=new MachineOperand(*(*def));
            MachineInstruction* new_inst_load;
            MachineInstruction* new_inst_store;
            MachineOperand* interval_reg=new MachineOperand(MachineOperand::VREG,SymbolTable::getLabel());
            new_inst_load=new LoadMInstruction((*def)->getParent()->getParent(),interval_reg, off);
            (*def)->getParent()->insertAfter(new_inst_load);
            auto src=new MachineOperand(*interval_reg);
            new_inst_store =new StoreMInstruction((*def)->getParent()->getParent(), dst,fp, src);
            new_inst_load->insertAfter(new_inst_store);
        }
    }
}

void LinearScan::expireOldIntervals(Interval *interval)
{
    // Todo
    //active中按照结束end递增排序，越靠后end越大
    for(long unsigned int i=0;i<active.size();i++)
    {
        if(active[i]->end>=interval->start)
        {
            break;
        }
        else
        {
            //释放当前活跃区间的物理寄存器
            regs.push_back(active[i]->rreg);
            active.erase(active.begin()+i);
        }
    }
}

void LinearScan::spillAtInterval(Interval *interval)
{
    // Todo
    //寄存器溢出操作
    int last=active.size()-1;
    if(interval->end>active[last]->end)
    {
        interval->spill = true;
    }
    else
    {
        active[last]->spill=true;
        interval->rreg = active[last]->rreg;
        active.push_back(interval);

        for(long unsigned int i=0;i<active.size();i++)
            {
                for (long unsigned int j=active.size()-1;j>i;j--)
                {
                    if(active[j]->end<active[j-1]->end)
                    {
                        Interval* tmp=active[j-1];
                        active[j-1]=active[j];
                        active[j]=tmp;
                    }
                }
            }
    }
}


bool LinearScan::compareStart(Interval *a, Interval *b)
{
    return a->start < b->start;
}