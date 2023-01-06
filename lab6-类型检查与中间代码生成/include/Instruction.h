#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include "Operand.h"
#include <vector>
#include <map>

class BasicBlock;

//指令基类
class Instruction
{
public:
//构造和析构，指令的类型，和一个基本块？
    Instruction(unsigned instType, BasicBlock *insert_bb = nullptr);
    virtual ~Instruction();

    //每一条指令应该都属于一个基本块
    BasicBlock *getParent();

    //指令类型判断，是不是条件分支指令或者无条件跳转
    bool isUncond() const {return instType == UNCOND;};
    bool isCond() const {return instType == COND;};
    bool isRet() const {return instType==RET;};

    //设置所属的基本块
    void setParent(BasicBlock *);

    //上一条指令和后一条指令
    void setNext(Instruction *);
    void setPrev(Instruction *);
    Instruction *getNext();
    Instruction *getPrev();
    //打印
    virtual void output() const = 0;
protected:
    //指令类型
    unsigned instType;
    //操作码
    unsigned opcode;
    Instruction *prev;
    Instruction *next;
    BasicBlock *parent;
    std::vector<Operand*> operands;
    //指令的类型：位运算，条件，非条件，return，load，store，cmp。分配一个空间。
    enum {BINARY, COND, UNCOND, RET, LOAD, STORE, CMP, ALLOCA,XOR,FUNCCALL,ZEXT};
};

// meaningless instruction, used as the head node of the instruction list.
//假指令？无用指令。
class DummyInstruction : public Instruction
{
public:
    //
    DummyInstruction() : Instruction(-1, nullptr) {};
    void output() const {};
};

class AllocaInstruction : public Instruction
{
public:
    //内存分配指令？有一个自己的符号表项，一个操作数
    AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb = nullptr);
    ~AllocaInstruction();
    void output() const;
private:
    SymbolEntry *se;
};

class LoadInstruction : public Instruction
{
public:
    //加载指令
    LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb = nullptr);
    ~LoadInstruction();
    void output() const;
};

class StoreInstruction : public Instruction
{
public:
    StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb = nullptr);
    ~StoreInstruction();
    void output() const;
};

class BinaryInstruction : public Instruction
{
public:
    BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~BinaryInstruction();
    void output() const;
    enum {SUB,ADD,MUL,DIV,MOD,AND, OR};
};

class CmpInstruction : public Instruction
{
public:
    CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~CmpInstruction();
    void output() const;
    enum {E, NE, L, GE, G, LE};
};

// unconditional branch
class UncondBrInstruction : public Instruction
{
public:
    UncondBrInstruction(BasicBlock*, BasicBlock *insert_bb = nullptr);
    void output() const;
    void setBranch(BasicBlock *);
    BasicBlock *getBranch();
protected:
    //无条件跳转指令，branch是他要跳转的分支，是个基本块
    BasicBlock *branch;
};

// conditional branch
class CondBrInstruction : public Instruction
{
public:
    CondBrInstruction(BasicBlock*, BasicBlock*, Operand *, BasicBlock *insert_bb = nullptr);
    ~CondBrInstruction();
    void output() const;
    void setTrueBranch(BasicBlock*);
    BasicBlock* getTrueBranch();
    void setFalseBranch(BasicBlock*);
    BasicBlock* getFalseBranch();
protected:
    BasicBlock* true_branch;
    BasicBlock* false_branch;
};

class RetInstruction : public Instruction
{
public:
    RetInstruction(Operand *src, BasicBlock *insert_bb = nullptr);
    ~RetInstruction();
    void output() const;
};

class XorInstruction:public Instruction
{
public:
    XorInstruction(Operand* ,Operand*,BasicBlock *insert_bb = nullptr );
    void output() const;
};

class FuncCallInstruction:public Instruction
{
private:
    SymbolEntry* func;
public:
    FuncCallInstruction(Operand* ,SymbolEntry* ,std::vector<Operand*> ,BasicBlock* );
    void output() const;
};

class ZextInstruction:public Instruction
{
public:
    ZextInstruction(Operand*,Operand*,BasicBlock*);
    void output()const;
};
#endif