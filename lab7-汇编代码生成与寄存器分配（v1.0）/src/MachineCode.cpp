#include "MachineCode.h"
#include<iostream>
#include<Type.h>
extern FILE* yyout;

MachineOperand::MachineOperand(int tp, int val)
{
    this->type = tp;
    if(tp == MachineOperand::IMM)
        this->val = val;
    else 
        this->reg_no = val;
}

MachineOperand::MachineOperand(std::string label)
{
    this->type = MachineOperand::LABEL;
    this->label = label;
}

bool MachineOperand::operator==(const MachineOperand&a) const
{
    if (this->type != a.type)
        return false;
    if (this->type == IMM)
        return this->val == a.val;
    return this->reg_no == a.reg_no;
}

bool MachineOperand::operator<(const MachineOperand&a) const
{
    if(this->type == a.type)
    {
        if(this->type == IMM)
            return this->val < a.val;
        return this->reg_no < a.reg_no;
    }
    return this->type < a.type;

    if (this->type != a.type)
        return false;
    if (this->type == IMM)
        return this->val == a.val;
    return this->reg_no == a.reg_no;
}

void MachineOperand::PrintReg()
{
    switch (reg_no)
    {
    case 11:
        fprintf(yyout, "fp");
        break;
    case 13:
        fprintf(yyout, "sp");
        break;
    case 14:
        fprintf(yyout, "lr");
        break;
    case 15:
        fprintf(yyout, "pc");
        break;
    default:
        fprintf(yyout, "r%d", reg_no);
        break;
    }
}

void MachineOperand::output() 
{
    /* HINT：print operand
    * Example:
    * immediate num 1 -> print #1;
    * register 1 -> print r1;
    * lable addr_a -> print addr_a; */
    switch (this->type)
    {
    case IMM:
        fprintf(yyout, "#%d", this->val);
        break;
    case VREG:
        fprintf(yyout, "v%d", this->reg_no);
        break;
    case REG:
        PrintReg();
        break;
    case LABEL:
        if (this->label.substr(0, 2) == ".L")
        {
            fprintf(yyout, "%s", this->label.c_str());
        }
        else if (this->label.substr(0, 1) == "@")
        {
            fprintf(yyout, "%s", this->label.c_str() + 1);
        }
        else
        {
            fprintf(yyout, "addr_%s%d", this->label.c_str(),parent->getParent()->getParent()->getParent()->getN());
        }
    default:
        break;
    }
}

void MachineInstruction::PrintCond()
{
    // TODO
    switch (cond)
    {
    case LT:
        fprintf(yyout, "lt");
        break;
    case LE:
        fprintf(yyout, "le");
        break;
    case GT:
        fprintf(yyout, "gt");
        break;
    case GE:
        fprintf(yyout, "ge");
        break;
    case EQ:
        fprintf(yyout, "eq");
        break;
    case NE:
        fprintf(yyout, "ne");
        break;
    default:
        break;
    }
}

void MachineInstruction::insertBefore(MachineInstruction* inst) {
    auto& instructions = parent->getInsts();
    auto it = std::find(instructions.begin(), instructions.end(), this);
    instructions.insert(it, inst);
}

void MachineInstruction::insertAfter(MachineInstruction* inst) {
    auto& instructions = parent->getInsts();
    auto it = std::find(instructions.begin(), instructions.end(), this);
    instructions.insert(++it, inst);
}

BinaryMInstruction::BinaryMInstruction(MachineBlock* p, int op, MachineOperand* dst, MachineOperand* src1, MachineOperand* src2, int cond)
{
    this->parent = p;
    this->type = MachineInstruction::BINARY;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    dst->setParent(this);
    src1->setParent(this);
    src2->setParent(this);
}

void BinaryMInstruction::output() 
{
    // TODO: 
    // Complete other instructions
    switch (this->op)
    {
    case BinaryMInstruction::ADD:
        fprintf(yyout, "\tadd ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::SUB:
        fprintf(yyout, "\tsub ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::MUL:
        fprintf(yyout, "\tmul ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::DIV:
        fprintf(yyout, "\tsdiv ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::AND:
        fprintf(yyout, "\tand ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::OR:
        fprintf(yyout, "\torr ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    default:
        break;
    }
}

LoadMInstruction::LoadMInstruction(MachineBlock* p,
    MachineOperand* dst, MachineOperand* src1, MachineOperand* src2,
    int cond)
{
    this->parent = p;
    this->type = MachineInstruction::LOAD;
    this->op = -1;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    if (src2)
        this->use_list.push_back(src2);
    dst->setParent(this);
    src1->setParent(this);
    if (src2)
        src2->setParent(this);
}

void LoadMInstruction::output()
{
    fprintf(yyout, "\tldr ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");

    // Load immediate num, eg: ldr r1, =8
    if(this->use_list[0]->isImm())
    {
        fprintf(yyout, "=%d\n", this->use_list[0]->getVal());
        return;
    }

    // Load address
    if(this->use_list[0]->isReg()||this->use_list[0]->isVReg())
        fprintf(yyout, "[");

    this->use_list[0]->output();
    if( this->use_list.size() > 1 )
    {
        fprintf(yyout, ", ");
        this->use_list[1]->output();
    }

    if(this->use_list[0]->isReg()||this->use_list[0]->isVReg())
        fprintf(yyout, "]");
    fprintf(yyout, "\n");
}

StoreMInstruction::StoreMInstruction(MachineBlock* p,
    MachineOperand* src1, MachineOperand* src2, MachineOperand* src3, 
    int cond)
{
    // TODO
    
    this->parent = p;
    this->type = MachineInstruction::STORE;
    this->op = -1;
    this->cond = cond;
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    if (src3)
    {
        this->use_list.push_back(src3);
    }
    src1->setParent(this);
    src2->setParent(this);
    if (src3)
    {
        src3->setParent(this);
    }
}

void StoreMInstruction::output()
{
    // TODO
    fprintf(yyout, "\tstr ");
    this->use_list[0]->output();
    fprintf(yyout, ", ");
    //存疑：这里不考虑store常量吗？
    // store address
    if (this->use_list[1]->isReg() || this->use_list[1]->isVReg())
    {
        fprintf(yyout, "[");
    }
    this->use_list[1]->output();
    if (this->use_list.size() > 2) 
    {
        fprintf(yyout, ", ");
        this->use_list[2]->output();
    }
    if (this->use_list[1]->isReg() || this->use_list[1]->isVReg())
    {
        fprintf(yyout, "]");
    }
    fprintf(yyout, "\n");
}

MovMInstruction::MovMInstruction(MachineBlock* p, int op, MachineOperand* dst, MachineOperand* src,int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::MOV;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src);
    dst->setParent(this);
    src->setParent(this);

}

void MovMInstruction::output() 
{
    // TODO
    fprintf(yyout, "\tmov");
    PrintCond();
    fprintf(yyout, " ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[0]->output();
    fprintf(yyout, "\n");

}

BranchMInstruction::BranchMInstruction(MachineBlock* p, int op, MachineOperand* dst, int cond)
{
    // TODO
    this->parent = p;
    this->op = op;
    this->type = MachineInstruction::BRANCH;
    this->use_list.push_back(dst);
    dst->setParent(this);
    this->cond = cond;
}

void BranchMInstruction::output()
{
    // TODO
    switch (op) {
        case B:
            fprintf(yyout, "\tb");
            PrintCond();
            fprintf(yyout, " ");
            this->use_list[0]->output();
            fprintf(yyout, "\n");
            break;
        case BX:
            fprintf(yyout, "\tbx");
            PrintCond();
            fprintf(yyout, " ");
            this->use_list[0]->output();
            fprintf(yyout, "\n");
            break;
        case BL:
            fprintf(yyout, "\tbl");
            PrintCond();
            fprintf(yyout, " ");
            this->use_list[0]->output();
            fprintf(yyout, "\n");
            break;
    }
}

CmpMInstruction::CmpMInstruction(MachineBlock* p, 
    MachineOperand* src1, MachineOperand* src2, 
    int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::CMP;
    this->op = -1;
    this->cond = cond;
    p->setCmpCond(cond);
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    src1->setParent(this);
    src2->setParent(this);
}

void CmpMInstruction::output()
{
    // TODO
    // Jsut for reg alloca test
    // delete it after test
    fprintf(yyout, "\tcmp ");
    this->use_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[1]->output();
    fprintf(yyout, "\n");
}

StackMInstrcuton::StackMInstrcuton(MachineBlock* p, int op, std::vector<MachineOperand*> srcs,MachineOperand* dst,MachineOperand* src ,int cond)
{
    // TODO
    this->parent=p;
    this->type = MachineInstruction::STACK;
    this->op = op;
    this->cond = cond;
    if(srcs.size()!=0)
    {
        for(long unsigned int i=0;i<srcs.size();i++)
        {
            this->use_list.push_back(srcs[i]);
        }
    }
    this->use_list.push_back(dst);
    dst->setParent(this);
    if(src!=nullptr)
    {
        this->use_list.push_back(src);
        src->setParent(this);
    }

}

void StackMInstrcuton::output()
{
    // TODO
    switch (op) {
        case PUSH:
            fprintf(yyout, "\tpush ");
            break;
        case POP:
            fprintf(yyout, "\tpop ");
            break;
    }
    fprintf(yyout, "{");
    this->use_list[0]->output();
    for (long unsigned int i = 1; i < use_list.size(); i++)
    {
        fprintf(yyout, ", ");
        this->use_list[i]->output();
    }
    fprintf(yyout, "}\n");
}

MachineFunction::MachineFunction(MachineUnit* p, SymbolEntry* sym_ptr) 
{ 
    this->parent = p; 
    this->sym_ptr = sym_ptr; 
    this->stack_size = 0;
};

void MachineBlock::output()
{
    /*fprintf(yyout, ".L%d:\n", this->no);
    for(auto iter : inst_list)
        iter->output();
    */
    //bool first = true;
    //int offset = (parent->getSavedRegs().size() + 2) * 4;
    //int num = parent->getParamsNum();
    if(inst_list.size()!=0)
    {
        fprintf(yyout, ".L%d:\n", this->no);
        for(long unsigned int i=0;i<inst_list.size();i++)
        {
            //当前指令inst_list[i];
            if(inst_list[i]->getinstType()==MachineInstruction::BRANCH)
            {
                if(dynamic_cast<BranchMInstruction*>(inst_list[i])->getop()==BranchMInstruction::BX)
                {
                    auto fp = new MachineOperand(MachineOperand::REG, 11);
                    auto lr = new MachineOperand(MachineOperand::REG, 14);
                    auto cur_inst =new StackMInstrcuton(this, StackMInstrcuton::POP,parent->getSavedRegs(), fp, lr);
                    cur_inst->output();
                }
            }
            /*
            if((inst_list[i]->getinstType()==MachineInstruction::STORE)&&(num>4))
            {
                MachineOperand* operand = inst_list[i]->getUse()[0];
                if (operand->isReg() && operand->getReg() == 3)
                {
                    if (first) 
                    {
                        first = false;
                    } 
                    {
                        auto fp = new MachineOperand(MachineOperand::REG, 11);
                        auto r3 = new MachineOperand(MachineOperand::REG, 3);
                        auto off =new MachineOperand(MachineOperand::IMM, offset);
                        auto cur_inst = new LoadMInstruction(this, r3, fp, off);
                        cur_inst->output();
                        offset += 4;
                    }
                }
            }
            if(inst_list[i]->getinstType()==MachineInstruction::BINARY)
            {
                if(dynamic_cast<BinaryMInstruction*>(inst_list[i])->getop()==BinaryMInstruction::ADD)
                {
                    auto dst = inst_list[i]->getDef()[0];
                    auto src1 = inst_list[i]->getUse()[0];
                    if (dst->isReg() && dst->getReg() == 13 && src1->isReg() &&src1->getReg() == 13)
                    {
                        if((inst_list[i+1]->getinstType()==MachineInstruction::BRANCH)&&(dynamic_cast<BranchMInstruction*>(inst_list[i])->getop()==BranchMInstruction::BX))
                        {
                            int size = parent->AllocSpace(0);
                            if (size < -255 || size > 255) 
                            {
                                auto r1 = new MachineOperand(MachineOperand::REG, 1);
                                auto off =new MachineOperand(MachineOperand::IMM, size);
                                (new LoadMInstruction(nullptr, r1, off))->output();
                                inst_list[i]->getUse()[1]->setReg(1);
                            } 
                            else
                            {
                                inst_list[i]->getUse()[1]->setVal(size);
                            }
                        }
                    }
                }
            }*/
            inst_list[i]->output();
        }
    }
}

std::vector<MachineOperand*> MachineFunction::getSavedRegs() {
    std::vector<MachineOperand*> regs;
    for (auto it = saved_regs.begin(); it != saved_regs.end(); it++) {
        auto reg = new MachineOperand(MachineOperand::REG, *it);
        regs.push_back(reg);
    }
    return regs;
}
void MachineFunction::output()
{
    const char *func_name = this->sym_ptr->toStr().c_str() + 1;
    fprintf(yyout, "\t.global %s\n", func_name);
    fprintf(yyout, "\t.type %s , %%function\n", func_name);
    fprintf(yyout, "%s:\n", func_name);
    // TODO
    /* Hint:
    *  1. Save fp
    *  2. fp = sp
    *  3. Save callee saved register
    *  4. Allocate stack space for local variable */
    
    // Traverse all the block in block_list to print assembly code.
    auto fp = new MachineOperand(MachineOperand::REG, 11);
    auto sp = new MachineOperand(MachineOperand::REG, 13);
    auto lr = new MachineOperand(MachineOperand::REG, 14);
    (new StackMInstrcuton(nullptr, StackMInstrcuton::PUSH, getSavedRegs(), fp,lr))->output();
    (new MovMInstruction(nullptr, MovMInstruction::MOV, fp, sp))->output();
    int off = AllocSpace(0);
    auto size = new MachineOperand(MachineOperand::IMM, off);
    if (off < -255 || off > 255) 
    {
        auto r4 = new MachineOperand(MachineOperand::REG, 4);
        (new LoadMInstruction(nullptr, r4, size))->output();
        (new BinaryMInstruction(nullptr, BinaryMInstruction::SUB, sp, sp, r4))->output();
    } 
    else 
    {
        (new BinaryMInstruction(nullptr, BinaryMInstruction::SUB, sp, sp, size))->output();
    }
    int count = 0;
    for(auto iter : block_list)
    {
        iter->output();
        if(count > 160){
            fprintf(yyout, "\tb .F%d\n", parent->getN());
            fprintf(yyout, ".LTORG\n");
            parent->printGlobal();
            fprintf(yyout, ".F%d:\n", parent->getN()-1);
            count = 0;
        }
    }
}

void MachineUnit::PrintGlobalDecl()
{
    // TODO:
    std::vector<int> constIdx;
    std::vector<int> zeroIdx;
    if (!global_id_list.empty())
    {
        fprintf(yyout, "\t.data\n");
    }
    
    for (long unsigned int i = 0; i < global_id_list.size(); i++) 
    {
        
        IdentifierSymbolEntry* se = (IdentifierSymbolEntry*)global_id_list[i];
        if(se->getType()->isInt()&&(dynamic_cast<IntType*>(se->getType())->whetherConst()))
        {
            //const常量
            constIdx.push_back(i);
        }
        else
        {
            fprintf(yyout, "\t.global %s\n", se->toStr().c_str());
            fprintf(yyout, "\t.align 4\n");
            if(se->getType()->isInt())
            {
                fprintf(yyout, "\t.size %s, %d\n", se->toStr().c_str(),dynamic_cast<IntType*>(se->getType())->getSize() / 8);
            }
            fprintf(yyout, "%s:\n", se->toStr().c_str());
            if (!se->getType()->isArray())
            {
                fprintf(yyout, "\t.word %d\n", se->getIntValue());
            }
        }
        /*
        if (se->getConst()) 
        {
            constIdx.push_back(i);
        } else if (se->isAllZero()) {
            zeroIdx.push_back(i);
            
        } else {
            fprintf(yyout, "\t.global %s\n", se->toStr().c_str());
            fprintf(yyout, "\t.align 4\n");
            fprintf(yyout, "\t.size %s, %d\n", se->toStr().c_str(),
                    se->getType()->getSize() / 8);
            fprintf(yyout, "%s:\n", se->toStr().c_str());
            if (!se->getType()->isArray()) {
                fprintf(yyout, "\t.word %d\n", se->getValue());
            } else {
                int n = se->getType()->getSize() / 32;
                int* p = se->getArrayValue();
                for (int i = 0; i < n; i++) {
                    fprintf(yyout, "\t.word %d\n", p[i]);
                }
            }
        }*/
    }
    if(constIdx.size()!=0)
    {
        fprintf(yyout, "\t.section .rodata\n");
        for(long unsigned int i=0;i<constIdx.size();i++)
        {
            IdentifierSymbolEntry* se = (IdentifierSymbolEntry*)global_id_list[constIdx[i]];
            fprintf(yyout, "\t.global %s\n", se->toStr().c_str());
            fprintf(yyout, "\t.align 4\n");
            fprintf(yyout, "\t.size %s, %d\n", se->toStr().c_str(),dynamic_cast<IntType*>(se->getType())->getSize() / 8);
            fprintf(yyout, "%s:\n", se->toStr().c_str());
            if (!se->getType()->isArray()) {
                fprintf(yyout, "\t.word %d\n", se->getIntValue());
            } 
        }
    }
    /*
    if (!constIdx.empty()) {
        fprintf(yyout, "\t.section .rodata\n");
        for (auto i : constIdx) {
            IdentifierSymbolEntry* se = (IdentifierSymbolEntry*)global_list[i];
            fprintf(yyout, "\t.global %s\n", se->toStr().c_str());
            fprintf(yyout, "\t.align 4\n");
            fprintf(yyout, "\t.size %s, %d\n", se->toStr().c_str(),
                    se->getType()->getSize() / 8);
            fprintf(yyout, "%s:\n", se->toStr().c_str());
            if (!se->getType()->isArray()) {
                fprintf(yyout, "\t.word %d\n", se->getValue());
            } else {
                int n = se->getType()->getSize() / 32;
                int* p = se->getArrayValue();
                for (int i = 0; i < n; i++) {
                    fprintf(yyout, "\t.word %d\n", p[i]);
                }
            }
        }
    }
    if (!zeroIdx.empty()) {
        for (auto i : zeroIdx) {
            IdentifierSymbolEntry* se = (IdentifierSymbolEntry*)global_list[i];
            if (se->getType()->isArray()) {
                fprintf(yyout, "\t.comm %s, %d, 4\n", se->toStr().c_str(),
                        se->getType()->getSize() / 8);
            }
        }
    }*/
}
void MachineUnit::printGlobal(){
    for (auto s : global_id_list) {
        IdentifierSymbolEntry* se = (IdentifierSymbolEntry*)s;
        fprintf(yyout, "addr_%s%d:\n", se->toStr().c_str(), n);
        fprintf(yyout, "\t.word %s\n", se->toStr().c_str());
    }
    n++;
}
void MachineUnit::output()
{
    // TODO
    /* Hint:
    * 1. You need to print global variable/const declarition code;
    * 2. Traverse all the function in func_list to print assembly code;
    * 3. Don't forget print bridge label at the end of assembly code!! */
    fprintf(yyout, "\t.arch armv8-a\n");
    fprintf(yyout, "\t.arch_extension crc\n");
    fprintf(yyout, "\t.arm\n");
    PrintGlobalDecl();
    for(auto iter : func_list)
        iter->output();
    printGlobal();
}
