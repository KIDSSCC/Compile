#ifndef __BASIC_BLOCK_H__
#define __BASIC_BLOCK_H__
#include <vector>
#include <set>
#include "Instruction.h"

class Function;

class BasicBlock
{
    //一个基本块的迭代器
    typedef std::vector<BasicBlock *>::iterator bb_iterator;

private:
    //两个动态数组，分别是当前基本块的前驱列表和后继列表。应该是代表着哪些块可以跳转到这里，和这个块可以跳转到哪些地方。
    std::vector<BasicBlock *> pred, succ;
    //第一条指令
    Instruction *head;
    //所属的函数
    Function *parent;
    //一个序号？
    int no;

public:
    //构造和析构
    BasicBlock(Function *);
    ~BasicBlock();

    //在前面插入一条指令，在后面插入一条指令
    void insertFront(Instruction *);
    void insertBack(Instruction *);
    void insertBefore(Instruction *, Instruction *);
    void remove(Instruction *);
    bool empty() const { return head->getNext() == head;}
    void output() const;
    bool succEmpty() const { return succ.empty(); };
    bool predEmpty() const { return pred.empty(); };
    void addSucc(BasicBlock *);
    void removeSucc(BasicBlock *);
    void addPred(BasicBlock *);
    void removePred(BasicBlock *);
    int getNo() { return no; };


    Function *getParent() { return parent; };
    //获取其中的指令
    Instruction* begin() { return head->getNext();};
    Instruction* end() { return head;};
    Instruction* rbegin() { return head->getPrev();};
    Instruction* rend() { return head;};
    //获取其前驱与后继的基本块列表
    bb_iterator succ_begin() { return succ.begin(); };
    bb_iterator succ_end() { return succ.end(); };
    bb_iterator pred_begin() { return pred.begin(); };
    bb_iterator pred_end() { return pred.end(); };
    
    int getNumOfPred() const { return pred.size(); };
    int getNumOfSucc() const { return succ.size(); };
};

#endif