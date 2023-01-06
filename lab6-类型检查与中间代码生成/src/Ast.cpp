#include "Ast.h"
#include "SymbolTable.h"
#include "Unit.h"
#include "Instruction.h"
#include "IRBuilder.h"
#include "Operand.h"
#include <string>
#include "Type.h"
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
extern Unit unit;

extern FILE *yyout;
int Node::counter = 0;
IRBuilder* Node::builder = nullptr;

bool ifFuncHasRtnstmt=0;//类型检查返回值
int boolcastvalue=0;

//void generateCtrlStream

Node::Node()
{
    seq = counter++;
    next = nullptr; //加
}
void Node::setNext(Node* node) {   
    //通过next指针将不同的节点串联起来，暂时只用于变量批量声明时，在同一级打印出各个声明语句
    Node* n = this;
    while (n->getNext()) {
        n = n->getNext();
    }
    if (n == this) {
        this->next = node;
    } else {
        n->setNext(node);
    }
}
void Node::backPatch(std::vector<Instruction*> &list, BasicBlock*bb)
{
    //回填函数
    for(auto &inst:list)
    {
        if(inst->isCond())
            dynamic_cast<CondBrInstruction*>(inst)->setTrueBranch(bb);
        else if(inst->isUncond())
            dynamic_cast<UncondBrInstruction*>(inst)->setBranch(bb);
    }
}

void Node::backPatchelse(std::vector<Instruction*> &list, BasicBlock*bb)
{
    //回填函数
    for(auto &inst:list)
    {
        if(inst->isCond())
            dynamic_cast<CondBrInstruction*>(inst)->setFalseBranch(bb);
    }
}
std::vector<Instruction*> Node::merge(std::vector<Instruction*> &list1, std::vector<Instruction*> &list2)
{
    //合并函数
    std::vector<Instruction*> res(list1);
    res.insert(res.end(), list2.begin(), list2.end());
    return res;
}

void Ast::genCode(Unit *unit)
{
    //根节点，中间代码生成的起始
    IRBuilder *builder = new IRBuilder(unit);
    Node::setIRBuilder(builder);
    root->genCode();
}

void FunctionDef::genCode()
{
    //函数的声明
    Unit *unit = builder->getUnit();
    Function *func = new Function(unit, se);
    BasicBlock *entry = func->getEntry();
    // set the insert point to the entry basicblock of this function.
    builder->setInsertBB(entry);
    if(paramDecl!=nullptr)
    {
        paramDecl->genCode();
    }
    stmt->genCode();
    /**
     * Construct control flow graph. You need do set successors and predecessors for each basic block.
     * Todo
    */

   for(std::vector<BasicBlock *>::iterator bb=func->begin();bb!=func->end();bb++)
   {
        Instruction* lastinst=(*bb)->rbegin();
        if (lastinst->isCond()) 
        {
            BasicBlock *truebranch =dynamic_cast<CondBrInstruction*>(lastinst)->getTrueBranch();
            BasicBlock *falsebranch =dynamic_cast<CondBrInstruction*>(lastinst)->getFalseBranch();
            //块与块之间的前驱与后继关系
            (*bb)->addSucc(truebranch);
            (*bb)->addSucc(falsebranch);
            truebranch->addPred(*bb);
            falsebranch->addPred(*bb);
        }
        if (lastinst->isUncond())  
        {
            BasicBlock* branch =dynamic_cast<UncondBrInstruction*>(lastinst)->getBranch();
            //样例27，return语句在分支中，在整个函数最后需要补充额外的返回语句。
            //if语句中在then和else中都设置了return，使得整个if语句的end块中没有指令了
            //也可以改进...应该？
            if (branch->empty()) {
                if (((FunctionType*)(se->getType()))->getRetType() ==TypeSystem::intType)
                {
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),branch);
                }
                if (((FunctionType*)(se->getType()))->getRetType() ==TypeSystem::voidType)
                {
                    new RetInstruction(nullptr, branch);
                }
            }
            (*bb)->addSucc(branch);
            branch->addPred(*bb);
        }
   }   
   
}

void generateCtrlStream(IRBuilder* builder,ExprNode* cond)
{
    BasicBlock *bb = builder->getInsertBB();
    //基本块所属的函数
    Function *func = bb->getParent();
    BasicBlock *block1=new BasicBlock(func);
    BasicBlock *block2=new BasicBlock(func);
    CondBrInstruction* tmp=new CondBrInstruction(block1, block2, cond->getOperand(), bb);
    cond->trueList().push_back(tmp);
    cond->falseList().push_back(tmp);
}

IntToBool::IntToBool(ExprNode* expr):ExprNode(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()))
{
    this->expr=expr;
    dst=new Operand(symbolEntry);
}
//整型转bool
void IntToBool::genCode()
{
    expr->genCode();
    BasicBlock* bb = builder->getInsertBB();
    Function* func = bb->getParent();
    //像二元的关系表达式一样进行分支的设置。
    BasicBlock* block1 = new BasicBlock(func);
    BasicBlock* block2 = new BasicBlock(func);

    //和0的比较
    new CmpInstruction(CmpInstruction::NE, this->dst, this->expr->getOperand(),new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)), bb);
    CondBrInstruction* tmp=new CondBrInstruction(block1, block2, this->dst, bb);
    this->trueList().push_back(tmp);
    this->falseList().push_back(tmp);
}

void IntToBool::output(int level)
{

}

void IntToBool::typeCheck(Type* rtnType )
{

}

void BinaryExpr::genCode()
{
    //传递继承属性，在这里获取后续生成的指令要插入的基本块bb
    BasicBlock *bb = builder->getInsertBB();
    //基本块所属的函数
    Function *func = bb->getParent();
    if (op == AND)
    {
        //考虑是否进行操作数的类型转换
        Type* tmp1=expr1->getSymPtr()->getType();
        Type* tmp2=expr2->getSymPtr()->getType();
        if(tmp1->isFunc())
        {
            tmp1=dynamic_cast<FunctionType*>(tmp1)->getRetType();
        }
        if(tmp2->isFunc())
        {
            tmp2=dynamic_cast<FunctionType*>(tmp2)->getRetType();
        }
        if(tmp1->isInt())
        {
            expr1=new IntToBool(expr1);
        }
        if(tmp2->isInt())
        {
            expr2=new IntToBool(expr2);
        }
        
        
        /*
        B->B_1 && M B_2
        backpatch(B_1.truelist,M.instr)
        B.true_list=B_2.truelist
        B.falselist=merge(B_1.falselist,B_2.falselist)
        */
        //M
        BasicBlock *trueBB = new BasicBlock(func);  // if the result of lhs is true, jump to the trueBB.
        expr1->genCode();
        if(expr1->whetherRel)
            generateCtrlStream(builder,expr1);
        backPatch(expr1->trueList(), trueBB);
        builder->setInsertBB(trueBB);               // set the insert point to the trueBB so that intructions generated by expr2 will be inserted into it.
        expr2->genCode();
        if(expr2->whetherRel)
            generateCtrlStream(builder,expr2);
        true_list = expr2->trueList();
        false_list = merge(expr1->falseList(), expr2->falseList());
    }
    else if(op == OR)
    {
        // Todo
        Type* tmp1=expr1->getSymPtr()->getType();
        Type* tmp2=expr2->getSymPtr()->getType();
        if(tmp1->isFunc())
        {
            tmp1=dynamic_cast<FunctionType*>(tmp1)->getRetType();
        }
        if(tmp2->isFunc())
        {
            tmp2=dynamic_cast<FunctionType*>(tmp2)->getRetType();
        }
        if(tmp1->isInt())
        {
            expr1=new IntToBool(expr1);
        }
        if(tmp2->isInt())
        {
            expr2=new IntToBool(expr2);
        }
        
        
        /*
        B->B_1 || M B_2
        backpatch(B_1.falselist,M.instr)
        B.truelist=merge(B_1.truelist,B_2.truelist)
        B.falselist=B_2.falselist
        */
       //M
        BasicBlock* falseBB = new BasicBlock(func);
        expr1->genCode();
        if(expr1->whetherRel)
            generateCtrlStream(builder,expr1);
        backPatchelse(expr1->falseList(), falseBB);
        builder->setInsertBB(falseBB);
        expr2->genCode();
        if(expr2->whetherRel)
            generateCtrlStream(builder,expr2);
        true_list = merge(expr1->trueList(), expr2->trueList());
        false_list = expr2->falseList();
    }
    if(op >= LT && op <= NE)
    {
        // Todo
        whetherRel=true;
        expr1->genCode();
        expr2->genCode();
        Operand* result1 = expr1->getOperand();
        Operand* result2 = expr2->getOperand();
        //result1和result2是参与运算的两个运算数
        //考虑两个运算数是不是bool类型，如果是的话需要进行扩展（新建zext指令）因为bool和int的长度不一样
        /*
        B->E_1 rel E_2
        B.truelist=makelist(nextinst)
        B.falselist=makelist(nextinst+1)
        */
        if(expr1->getSymPtr()->getType()->isBool())
        {
                Operand* tem=new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
                new ZextInstruction(tem,result1,bb);
                result1=tem;
        }
        if(expr2->getSymPtr()->getType()->isBool())
        {
                Operand* tem=new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
                new ZextInstruction(tem,result2,bb);
                result2=tem;
        }
        int opcode;
        switch(op)
        {
            case LT:
                opcode=CmpInstruction::L;
                break;
            case GT:
                opcode=CmpInstruction::G;
                break;
            case LE:
                opcode=CmpInstruction::LE;
                break;
            case GE:
                opcode=CmpInstruction::GE;
                break;
            case EQ:
                opcode=CmpInstruction::E;
                break;
            case NE:
                opcode=CmpInstruction::NE;
                break;
        }

        new CmpInstruction(opcode, dst, result1, result2, bb);
        
    }
    if(op >= ADD && op <= MOD)
    {
        expr1->genCode();
        expr2->genCode();
        Operand *src1 = expr1->getOperand();
        Operand *src2 = expr2->getOperand();
        int opcode;
        switch (op)
        {
        case ADD:
            opcode = BinaryInstruction::ADD;
            break;
        case SUB:
            opcode = BinaryInstruction::SUB;
            break;
        case MUL:
            opcode = BinaryInstruction::MUL;
            break;
        case DIV:
            opcode = BinaryInstruction::DIV;
            break;
        case MOD:
            opcode = BinaryInstruction::MOD;
            break;
        }
        //向bb块中插入了这样的一条指令
        new BinaryInstruction(opcode, dst, src1, src2, bb);
    }
}

UnaryExpr::UnaryExpr(SymbolEntry* se, int op, ExprNode* expr): ExprNode(se), op(op), expr(expr)
{
    dst=new Operand(se);
}

void UnaryExpr::genCode()
{
    
    expr->genCode();
    BasicBlock* bb = builder->getInsertBB();
    //声明需要都放到switch外面
    Operand* src1;
    Operand* src2;
    Operand* src;

    switch(op)
    {
        case SUB:
            src1 =new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
            src2 = expr->getOperand();
            //涉及bool转整型
            if(expr->getSymPtr()->getType()->isBool())
            {
                Operand* tem=new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
                new ZextInstruction(tem,src2,bb);
                src2=tem;
            }
            new BinaryInstruction(BinaryInstruction::SUB, dst, src1, src2, bb);
            
            break;
        case NOT:
            src = expr->getOperand();
            //涉及整型转bool
            if(expr->getSymPtr()->getType()->isInt())
            {
                Operand* tem=new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
                new CmpInstruction(CmpInstruction::NE, tem, src,new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),bb);
                src=tem;
            }
            new XorInstruction(dst, src, bb);
            break;
    }
}

void Constant::genCode()
{
    // we don't need to generate code.
}

void Id::genCode()
{
    //对于整数可以就这样了
    BasicBlock *bb = builder->getInsertBB();
    Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getAddr();
    new LoadInstruction(dst, addr, bb);
}

void IfStmt::genCode()
{
    if(cond->getSymPtr()->getType()->isInt())
    {
        cond=new IntToBool(cond);
    }
    Function *func= builder->getInsertBB()->getParent();
    BasicBlock *then_bb, *end_bb;
    then_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);

    cond->genCode();
    
    if(cond->whetherRel)
    {
        //条件表达式还没有生成控制流
        generateCtrlStream(builder,cond);
        
    }
    backPatch(cond->trueList(), then_bb);
    backPatchelse(cond->falseList(), end_bb);

    builder->setInsertBB(then_bb);

    thenStmt->genCode();
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);

    builder->setInsertBB(end_bb);
}

void IfElseStmt::genCode()
{
    // Todo
    if(cond->getSymPtr()->getType()->isInt())
    {
        cond=new IntToBool(cond);
    }

    Function *func=builder->getInsertBB()->getParent();
    BasicBlock *then_bb, *else_bb,*end_bb;
    then_bb = new BasicBlock(func);
    else_bb=new BasicBlock(func);
    end_bb = new BasicBlock(func);

    cond->genCode();
    if(cond->whetherRel)
    {
        //条件表达式还没有生成控制流
        generateCtrlStream(builder,cond);
        
    }
    backPatch(cond->trueList(), then_bb);
    backPatchelse(cond->falseList(), else_bb);

    builder->setInsertBB(then_bb);
    thenStmt->genCode();
    
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);

    builder->setInsertBB(else_bb);
    elseStmt->genCode();
    else_bb=builder->getInsertBB();
    new UncondBrInstruction(end_bb, else_bb);

    builder->setInsertBB(end_bb);
}

void CompoundStmt::genCode()
{
    // Todo
    if(stmt!=nullptr)
        stmt->genCode();
}

void SeqNode::genCode()
{
    // Todo
    stmt1->genCode();
    stmt2->genCode();
}

void DeclStmt::genCode()
{
    IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>(id->getSymPtr());
    if(se->isGlobal())
    {
        PointerType* tmpType=new PointerType(se->getType());
        SymbolEntry *dstSymbolentry=new IdentifierSymbolEntry(tmpType,se->getName(),se->getScope());
        Operand* addr=new Operand(dstSymbolentry);
        se->setAddr(addr);
        //全局变量不能通过函数打印出来，通过编译单元进行打印
        unit.constId_list.push_back(se);
        
    }
    if(se->isLocal())
    {
        //查找到了当前属于哪一个函数
        Function *func = builder->getInsertBB()->getParent();
        BasicBlock *entry = func->getEntry();
        PointerType* tmpType = new PointerType(se->getType());
        SymbolEntry *dstSymbolentry = new TemporarySymbolEntry(tmpType, SymbolTable::getLabel());
        Operand *addr = new Operand(dstSymbolentry);
        Instruction *alloca = new AllocaInstruction(addr, se);                   // allocate space for local id in function stack.
        entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block.
        se->setAddr(addr);                                          // set the addr operand in symbol entry so that we can use it in subsequent code generation.

        //是否进行了初始化
        if(InitValue!=nullptr)
        {
            BasicBlock* bb = builder->getInsertBB();
            InitValue->genCode();
            Operand* src = InitValue->getOperand();
            new StoreInstruction(addr, src, bb);
        }
    }
    if(se->isParam())
    {
        //查找到了当前属于哪一个函数
        Function *func = builder->getInsertBB()->getParent();
        BasicBlock *entry = func->getEntry();
        PointerType* tmpType = new PointerType(se->getType());
        SymbolEntry *dstSymbolentry = new TemporarySymbolEntry(tmpType, SymbolTable::getLabel());
        Operand *addr = new Operand(dstSymbolentry);
        Instruction *alloca = new AllocaInstruction(addr, se);                   // allocate space for local id in function stack.
        entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block.

        BasicBlock* bb = builder->getInsertBB();
        new StoreInstruction(addr, se->getAddr(), bb);
        se->setAddr(addr);                                          // set the addr operand in symbol entry so that we can use it in subsequent code generation.
        
    }
    if (this->getNext()) {
        this->getNext()->genCode();
    }
}

void WhileStmt::genCode()
{
    if(cond->getSymPtr()->getType()->isInt())
    {
        cond=new IntToBool(cond);
    }
    BasicBlock *bb = builder->getInsertBB();
    Function* func= builder->getInsertBB()->getParent();
    BasicBlock *stmt_bb;
    condBlock=new BasicBlock(func);
    stmt_bb=new BasicBlock(func);
    endBlock=new BasicBlock(func);


    new UncondBrInstruction(condBlock, bb);

    builder->setInsertBB(condBlock);
    cond->genCode();
    if(cond->whetherRel)
    {
        generateCtrlStream(builder,cond);
    }
    backPatch(cond->trueList(), stmt_bb);
    backPatchelse(cond->falseList(), endBlock);

    builder->setInsertBB(stmt_bb);
    stmt->genCode();
    stmt_bb=builder->getInsertBB();
    new UncondBrInstruction(condBlock, stmt_bb);

    builder->setInsertBB(endBlock);
}

void BreakStmt::genCode()
{
    Function* func = builder->getInsertBB()->getParent();
    BasicBlock* bb = builder->getInsertBB();
    new UncondBrInstruction(whileStmt->getendBlock(), bb);
    //当前基本块已经结束，开始为下面的语句准备块
    BasicBlock* next = new BasicBlock(func);
    builder->setInsertBB(next);
}

void ContinueStmt::genCode()
{
    Function* func = builder->getInsertBB()->getParent();
    BasicBlock* bb = builder->getInsertBB();
    new UncondBrInstruction(whileStmt->getcondBlock(), bb);
    //当前基本块已经结束，开始为下面的语句准备块
    BasicBlock* next = new BasicBlock(func);
    builder->setInsertBB(next);
}

void ReturnStmt::genCode()
{
    // Todo
    BasicBlock* bb = builder->getInsertBB();
    Operand* returnValue;
    if (retValue!=nullptr) {
        retValue->genCode();
        returnValue = retValue->getOperand();
    }
    new RetInstruction(returnValue, bb);
}

void AssignStmt::genCode()
{
    BasicBlock *bb = builder->getInsertBB();
    expr->genCode();
    Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(lval->getSymPtr())->getAddr();
    Operand *src = expr->getOperand();
    /***
     * We haven't implemented array yet, the lval can only be ID. So we just store the result of the `expr` to the addr of the id.
     * If you want to implement array, you have to caculate the address first and then store the result into it.
     */
    new StoreInstruction(addr, src, bb);
}

bool whetherSysy(std::string name)
{
    if((name=="putint")||(name=="getint")||(name=="putch"))
    {
        return true;
    }
    return false;
}

FunctionCall::FunctionCall(SymbolEntry *se,Node* paramGiven) : ExprNode(se),paramGiven(paramGiven)
{
    //构造函数，获取到的参数为函数名的符号表项和可能存在的参数列表。
    //函数的返回值，同时也是本次函数调用语句的类型。
    Type* rtnType = dynamic_cast<FunctionType*>(symbolEntry->getType())->getRetType();
    if(rtnType!=TypeSystem::voidType)
    {
        SymbolEntry* result =new TemporarySymbolEntry(rtnType, SymbolTable::getLabel());
        dst=new Operand(result);
    }
    //检查是否是IO相关sysy函数
    std::string name=dynamic_cast<IdentifierSymbolEntry*>(se)->getName();
    if(whetherSysy(name))
    {
        std::vector<SymbolEntry*>::iterator itea=std::find(unit.externFunction.begin(),unit.externFunction.end(),se);
        if(itea==unit.externFunction.end())
        {
            unit.externFunction.push_back(se);
        }
    }
};

void FunctionCall::genCode()
{
    std::vector<Operand*> operands;
    Node* temp = paramGiven;
    while (temp) {
        temp->genCode();
        operands.push_back(dynamic_cast<ExprNode*>(temp)->getOperand());
        temp = temp->getNext();
    }
    BasicBlock* bb = builder->getInsertBB();
    new FuncCallInstruction(dst, symbolEntry, operands, bb);
}

void BlankStmt::genCode()
{
    //nothing to do-----probably...
}

void ExprStmt::genCode()
{
    se->genCode();
}



void Ast::typeCheck(Type* rtnType )
{
    if(root != nullptr)
        root->typeCheck(rtnType);
}

void FunctionDef::typeCheck(Type* rtnType )
{
    // Todo
    //debug吧
    ifFuncHasRtnstmt=0;
    SymbolEntry* se = this->getse();
    Type* myret = ((FunctionType*)(se->getType()))->getRetType();
    StmtNode* stmt = this->stmt;
    /*if (stmt == nullptr) {
        if (myret != TypeSystem::voidType)
            {fprintf(stderr, "fuction %s Block is Null,but rtnType is not void,but %s\n",((IdentifierSymbolEntry *)se)->getName().c_str(),myret->toStr().c_str());
             exit(EXIT_FAILURE);}
    }*/ //102if空语句会有问题？？？

    stmt->typeCheck(myret); //从函数这里不再流传nullptr而是函数的返回值，为了让return知道

    if(!ifFuncHasRtnstmt && myret != TypeSystem::voidType)
       {
            fprintf(stderr, "fuction  %s has no rtnstmt,but it should return type:%s\n",((IdentifierSymbolEntry *)se)->getName().c_str(),myret->toStr().c_str());
            exit(EXIT_FAILURE);
       } 
   
}

void BinaryExpr::typeCheck(Type* rtnType )
{   

    //先检查两个子表达式的类型
    //fprintf(stderr,"here2!\n");
    expr1 -> typeCheck(rtnType);
    expr2 -> typeCheck(rtnType);

    // Todo（参考指导书）
    //fprintf(stderr,"here!\n");
    
    //需要检查两个操作数的类型是否匹配
    Type *type1 = expr1->getSymPtr()->getType();
    Type *type2 = expr2 -> getSymPtr() -> getType();
    //3.函数调用根据返回值
    if(expr1 -> getSymPtr() -> getType()->isFunc()){
        //fprintf(stderr,"expr1 is functype\n");
        FunctionType * ftype=(FunctionType *)(expr1 -> getSymPtr() -> getType());
        //fprintf(stderr,"return type is %s\n",ftype->getRetType()-> toStr().c_str());
        type1=ftype->getRetType();
        //fprintf(stderr,"return type is %s\n",type1-> toStr().c_str());
    }   
    if(expr2 -> getSymPtr() -> getType()->isFunc()){
        //fprintf(stderr,"expr2 is functype\n");
        FunctionType * ftype=(FunctionType *)(expr2 -> getSymPtr() -> getType());
        //fprintf(stderr,"return type is %s\n",ftype->getRetType()-> toStr().c_str());
        type2=ftype->getRetType();
        //fprintf(stderr,"return type is %s\n",type2-> toStr().c_str());
    }
    //0.void不能参与二元运算
    if (type1->isVoid() || type2->isVoid()) {
        fprintf(stderr,"void can't be a BinaryExpr operand\n");
        exit(EXIT_FAILURE);
    }
    //1.int2bool,放过就行，转化在构函已做
    if (op == BinaryExpr::AND || op == BinaryExpr::OR) { 
        //debug:if (set_a(0) && set_b(1)) 
        if (expr1->getSymPtr()->getType()->isFunc() ){
            FunctionType * ftype=( FunctionType *)(expr1->getSymPtr()->getType());
            Type * rtype=ftype->getRetType();
            IntType * i1 =(IntType *)rtype;
            if(rtype->isInt() && i1->getSize() == 32 ){
                type1=TypeSystem::boolType;
            }     
        }
        if (expr2->getSymPtr()->getType()->isFunc() ){
            FunctionType * ftype=( FunctionType *)(expr2->getSymPtr()->getType());
            Type * rtype=ftype->getRetType();
            IntType * i2 =(IntType *)rtype;
            if(rtype->isInt() && i2->getSize() == 32 ){
                type2=TypeSystem::boolType;
            }     
        }
        IntType * itype1=(IntType *)(expr1->getSymPtr()->getType());
        IntType * itype2=(IntType *)(expr2->getSymPtr()->getType());
        if (expr1->getSymPtr()->getType()->isInt() && itype1->getSize() == 32) {
            type1=TypeSystem::boolType;
        }
        if (expr2->getSymPtr()->getType()->isInt() && itype2->getSize() == 32) {
            type2=TypeSystem::boolType; 
        }  
    }
    //2.bool2int if( a<1 != 2)
    if(op >= BinaryExpr::LT && op <=BinaryExpr:: NE){

        //debug:if (set_a(0) < set_b(1)) 
        if (expr1->getSymPtr()->getType()->isFunc() ){
            FunctionType * ftype=( FunctionType *)(expr1->getSymPtr()->getType());
            Type * rtype=ftype->getRetType();
            if(rtype->isBool()){
                type1=TypeSystem::intType;
            }     
        }
        if (expr2->getSymPtr()->getType()->isFunc() ){
            FunctionType * ftype=( FunctionType *)(expr2->getSymPtr()->getType());
            Type * rtype=ftype->getRetType();
            if(rtype->isBool() ){
                type2=TypeSystem::intType;
            }     
        }
        if (expr1->getSymPtr()->getType()->isBool()) {
            type1=TypeSystem::intType;
        }
        if (expr2->getSymPtr()->getType()->isBool()) {
            type2=TypeSystem::intType; 
        }     
    }
    
        
    //4.const int a =10,return a+5; a是TypeSystem::intconstType，5是TypeSystem::intType,将const进行修正
    if(type1 == TypeSystem::intconstType){
        type1=TypeSystem::intType;
    }
    if(type2 == TypeSystem::intconstType){
        type2=TypeSystem::intType;
    }
    if(type1 == TypeSystem::floatconstType){
        type1=TypeSystem::floatType;
    }
    if(type2 == TypeSystem::floatconstType){
        type2=TypeSystem::floatType;
    }
    //over

    if(type1 != type2){
        fprintf(stderr, "type %s and %s mismatch in line %05d\n",type1 -> toStr().c_str(), type2 -> toStr().c_str(),__LINE__);
        exit(EXIT_FAILURE);
    }
    //symbolEntry->setType(type1);//改变属性值
   
}

void UnaryExpr::typeCheck(Type* rtnType)
{
       //fprintf(stderr,"here3!\n");
    //先检查表达式的类型
    expr-> typeCheck(rtnType);
    // Todo 
    //void不能参与一元运算
    if (expr->getSymPtr()->getType()->isVoid()) {
        fprintf(stderr,"void can't be an UnaryExpr operand\n");
        exit(EXIT_FAILURE);
    }

}

void Constant::typeCheck(Type* rtnType )
{
    // Todo
}

void Id::typeCheck(Type* rtnType )
{
    // Todo
}

void IfStmt::typeCheck(Type* rtnType )
{
    // Todo
    cond->typeCheck(rtnType);
    if (thenStmt)
        thenStmt->typeCheck(rtnType);
}

void IfElseStmt::typeCheck(Type* rtnType)
{
    // Todo
    //fprintf(stderr,"here7!\n");
    cond->typeCheck(rtnType);
    if (thenStmt)
        thenStmt->typeCheck(rtnType);
    if (elseStmt)
        elseStmt->typeCheck(rtnType);
}

void WhileStmt::typeCheck(Type* rtnType )
{
   //fprintf(stderr,"here8!\n");
    cond->typeCheck(rtnType);
    if(stmt)
        stmt->typeCheck(rtnType);
}

void BreakStmt::typeCheck(Type* rtnType)
{

}

void ContinueStmt::typeCheck(Type* rtnType )
{

}

void CompoundStmt::typeCheck(Type* rtnType )
{
    // Todo
    if(stmt)
        stmt->typeCheck(rtnType);
}

void SeqNode::typeCheck(Type* rtnType )
{
    // Todo
    //fprintf(stderr,"here12!\n");
    if(stmt1)
        stmt1->typeCheck(rtnType);
    if(stmt2)
        stmt2->typeCheck(rtnType);
}

void DeclStmt::typeCheck(Type* rtnType )
{
    // Todo
}

void ReturnStmt::typeCheck(Type* rtnType )
{
     ifFuncHasRtnstmt=1;
    // Todo
    //fprintf(stderr,"here14!\n");
    if(retValue)
        retValue->typeCheck(rtnType);//别漏！
    //函数期待的返回值类型：rtnType
    //return语句返回的类型：retValue
    if(rtnType==nullptr){
        fprintf(stderr, "Function ReturnType doesn't exsit!\n");
        exit(EXIT_FAILURE);
    }

    Type* type1=rtnType;
    Type* type2;
    if(retValue==nullptr){
         type2=TypeSystem::voidType;
    }
    else  type2=retValue->getSymPtr()->getType();
    //debug1:const int a =10,return a; a是TypeSystem::intconstType，但函数要求是TypeSystem::intType,将const进行修正
    if(type1 == TypeSystem::intconstType){
        type1=TypeSystem::intType;
    }
    if(type2 == TypeSystem::intconstType){
        type2=TypeSystem::intType;
    }
    if(type1 == TypeSystem::floatconstType){
        type1=TypeSystem::floatType;
    }
    if(type2 == TypeSystem::floatconstType){
        type2=TypeSystem::floatType;
    }
    //debug2: return getInt(),要求inttype，但返回funcType，应该取函数类型的返回值类型！
    if(type2->isFunc() ){
            type2 = ((FunctionType *)type2)-> getRetType();   
    } 
    //debugover
    if(type1 != type2){
        fprintf(stderr, "shouldReturn:%s, butReturn:%s,type dismatch\n", type1->toStr().c_str (),type2->toStr().c_str ());
        exit(EXIT_FAILURE);
    }
}

void AssignStmt::typeCheck(Type* rtnType )
{
    // Todo
    //先检查表达式的类型
    //fprintf(stderr,"here15!\n");
    expr-> typeCheck(rtnType);
}

void FunctionCall::typeCheck(Type* rtnType)
{   //暂不考虑函数重载
    //fprintf(stderr,"here16!\n");
    //拿到形参种类和数目
    IdentifierSymbolEntry * identry=(IdentifierSymbolEntry *)(this->getSymPtr());
    FunctionType * ftype=(FunctionType *)(identry->getType());
    std::vector<Type*> xingparamsType = ftype->getparamsType();
    int xingparamsNum=xingparamsType.size();
    //拿到实参的种类和数目
    int shiparamsNum = 0;
    ExprNode* temp = (ExprNode*)(this->getparamGiven());
    ExprNode* temp1 = (ExprNode*)(this->getparamGiven());
    ExprNode* tempindex;
    int i=0,index;
    bool flag=true;
    while (temp) {
        shiparamsNum++;
        temp = (ExprNode*)(temp->getNext());
    }
    
    while (temp1) {
        if(xingparamsType[i]->getkind() != temp1->getSymPtr()->getType()->getkind() ){
            flag=false;
            index=i;
            tempindex=temp1;
            break;
        }
        i++;
        temp1 = (ExprNode*)(temp1->getNext());
    }
    
    //数目不相等
    if(xingparamsNum!=shiparamsNum){
        fprintf(stderr, "funcname:%s, xingparamsNum:%d,shiparamsNum:%d,num dismatch\n", identry->getName().c_str (),xingparamsNum,shiparamsNum);
        exit(EXIT_FAILURE);
    }
    //种类不匹配
    if(!flag){
        fprintf(stderr, "funcname:%s,index:%d, xingparamstype:%s,shiparamstype:%s,type dismatch\n", identry->getName().c_str (),index,
                xingparamsType[index]->toStr().c_str (),tempindex->getSymPtr()->getType()->toStr().c_str ());
        exit(EXIT_FAILURE);
    }

}

void BlankStmt::typeCheck(Type* rtnType )
{

}

void ExprStmt::typeCheck(Type* rtnType)
{

}

void BinaryExpr::output(int level)
{
    std::string op_str;
    switch(op)
    {
        case ADD:
            op_str = "add";
            break;
        case SUB:
            op_str = "sub";
            break;
        case AND:
            op_str = "and";
            break;
        case OR:
            op_str = "or";
            break;
        case LT:
            op_str = "less";
            break;
        case LE:
            op_str = "le";
            break;
        case GT:
            op_str = "gt";
            break;
        case GE:
            op_str = "ge";
            break;
        case EQ:
            op_str = "eq";
            break;
        case NE:
            op_str = "ne";
            break;
    }
    fprintf(yyout, "%*cBinaryExpr\top: %s\n", level, ' ', op_str.c_str());
    expr1->output(level + 4);
    expr2->output(level + 4);
}

void Ast::output()
{
    fprintf(yyout, "program\n");
    if(root != nullptr)
        root->output(4);
}

void Constant::output(int level)
{
    std::string type, value;
    type = symbolEntry->getType()->toStr();
    value = symbolEntry->toStr();
    fprintf(yyout, "%*cIntegerLiteral\tvalue: %s\ttype: %s\n", level, ' ',
            value.c_str(), type.c_str());
}

void Id::output(int level)
{
    std::string name, type;
    int scope;
    name = symbolEntry->toStr();
    type = symbolEntry->getType()->toStr();
    scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
    fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\n", level, ' ',
            name.c_str(), scope, type.c_str());
}

void UnaryExpr::output(int level) { //加
    std::string op_str;
    switch (op) {
        case ADD:
            op_str = "plus";
            break;
        case NOT:
            op_str = "not";
            break;
        case SUB:
            op_str = "minus";
            break;
    }
    fprintf(yyout, "%*cUnaryExpr\top: %s\n", level, ' ', op_str.c_str());
    expr->output(level + 4);
}

void CompoundStmt::output(int level)
{
    fprintf(yyout, "%*cCompoundStmt\n", level, ' ');
    if(stmt!=nullptr)
        stmt->output(level + 4);
}

void SeqNode::output(int level)
{
    stmt1->output(level);
    stmt2->output(level);
}

void DeclStmt::output(int level)
{
    fprintf(yyout, "%*cDeclStmt\n", level, ' ');
    id->output(level + 4);
    if (this->getNext()) {
        this->getNext()->output(level);
    }
}

void IfStmt::output(int level)
{
    fprintf(yyout, "%*cIfStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
}

void IfElseStmt::output(int level)
{
    fprintf(yyout, "%*cIfElseStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
    elseStmt->output(level + 4);
}

void WhileStmt::output(int level)
{
    fprintf(yyout, "%*cWhileStmt\n", level, ' ');
    cond->output(level + 4);
    if(stmt){
        stmt->output(level + 4);
    }
}

void BreakStmt::output(int level)
{
    fprintf(yyout, "%*cBreakStmt\n", level, ' ');
}

void ContinueStmt::output(int level)
{
    fprintf(yyout, "%*cContinueStmt\n", level, ' ');
}

void ReturnStmt::output(int level)
{
    fprintf(yyout, "%*cReturnStmt\n", level, ' ');
    if(retValue!=nullptr){
        retValue->output(level + 4);
    }
       
    else {
         fprintf(yyout, "%*cvoidLiteral\ttype: %s\n", level+4, ' ',
            "void");
        }
}

void AssignStmt::output(int level)
{
    fprintf(yyout, "%*cAssignStmt\n", level, ' ');
    lval->output(level + 4);
    expr->output(level + 4);
}

void FunctionDef::output(int level)
{
    std::string name, type;
    name = se->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "%*cFunctionDefine function name: %s, type: %s\n", level, ' ', 
            name.c_str(), type.c_str());
    stmt->output(level + 4);
}

void FunctionCall::output(int level)
{

}

void BlankStmt::output(int level)
{

}

void ExprStmt::output(int level)
{

}

int Constant::getValue()
{
    return dynamic_cast<ConstantSymbolEntry*>(symbolEntry)->getValue();
}

int Id::getValue()
{
    return dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getIntValue();
}

int BinaryExpr::getValue()
{
    int src1=expr1->getValue();
    int src2=expr2->getValue();
    int result;
    switch(op)
    {
        case ADD:
            result=src1+src2;
            break;
        case SUB:
            result=src1-src2;
            break;
        case MUL:
            result=src1*src2;
            break;
        case DIV:
            result=src1/src2;
            break;
        case MOD:
            result=src1%src2;
            break;
        case AND:
            result=(int)(src1&&src2);
            break;
        case OR:
            result=(int)(src1||src2);
            break;
        case LT:
            result=(int)(src1<src2);
            break;
        case GT:
            result=(int)(src1>src2);
            break;
        case LE:
            result=(int)(src1<=src2);
            break;
        case GE:
            result=(int)(src1>=src2);
            break;
        case EQ:
            result=(int)(src1==src2);
            break;
        case NE:
            result=(int)(src1!=src2);
            break;
    }
    return result;
}

int UnaryExpr::getValue()
{
    int src=expr->getValue();
    int result;
    switch (op)
    {
    case ADD:
        result=src;
        break;
    case NOT:
        if(src==0)
            result=1;
        else
            result=0;
        break;
    case SUB:
        result=(-1)*src;
        break;
    }
    return result;
}

int FunctionCall::getValue()
{
    return 0;
}


IfStmt::IfStmt(ExprNode *cond, StmtNode *thenStmt){
    this->cond=cond;
    this->thenStmt=thenStmt;
    
    //debug:if (set_a(0) )
    if (cond->getSymPtr()->getType()->isFunc() ){
        FunctionType * ftype=( FunctionType *)(cond->getSymPtr()->getType());
        Type * rtype=ftype->getRetType();
        IntType * i1 =(IntType *)rtype;
        if(rtype->isInt() && i1->getSize() == 32 ){
            cond=new IntToBool(cond);
            fprintf(stderr,"IfStmt Cond(isFuncCall) Cast to Bool!\n");
            //fprintf(stderr,"IfStmt Cond(isFuncCall) Cast to Bool!type:%s\n",cond->getSymPtr()->getType()->toStr().c_str());
        }     
    }
    //debugover
    IntType * itype=(IntType *)(cond->getSymPtr()->getType());
    if(cond->getSymPtr()->getType()->isInt() && itype->getSize() == 32)
    {
        cond=new IntToBool(cond);
        fprintf(stderr,"IfStmt Cond Cast to Bool!\n");
        //fprintf(stderr,"IfStmt Cond Cast to Bool!type:%s\n",cond->getSymPtr()->getType()->toStr().c_str());
    }
    
}
IfElseStmt::IfElseStmt(ExprNode *cond, StmtNode *thenStmt, StmtNode *elseStmt){
    this->cond=cond;
    this->thenStmt=thenStmt;
    this->elseStmt=elseStmt;
    
    //debug:if (set_a(0) )
    if (cond->getSymPtr()->getType()->isFunc() ){
        FunctionType * ftype=( FunctionType *)(cond->getSymPtr()->getType());
        Type * rtype=ftype->getRetType();
        IntType * i1 =(IntType *)rtype;
        if(rtype->isInt() && i1->getSize() == 32 ){
            cond=new IntToBool(cond);
            fprintf(stderr,"IfElseStmt Cond(isFuncCall) Cast to Bool!\n");
        }     
    }
    //debugover 
    IntType * itype=(IntType *)(cond->getSymPtr()->getType());
    if(cond->getSymPtr()->getType()->isInt()&& itype->getSize() == 32)
    {
        cond=new IntToBool(cond);
        fprintf(stderr,"IfElseStmt Cond Cast to Bool!\n");
    }
    
}
WhileStmt::WhileStmt(ExprNode* cond, StmtNode* stmt){
    this->cond=cond;
    this->stmt=stmt;

    
    //debug:if (set_a(0) )
    if (cond->getSymPtr()->getType()->isFunc() ){
        FunctionType * ftype=( FunctionType *)(cond->getSymPtr()->getType());
        Type * rtype=ftype->getRetType();
        IntType * i1 =(IntType *)rtype;
        if(rtype->isInt() && i1->getSize() == 32 ){
            cond=new IntToBool(cond);
            fprintf(stderr,"WhileStmt Cond(isFuncCall) Cast to Bool!\n");
        }     
    }
    //debugover 
    IntType * itype=(IntType *)(cond->getSymPtr()->getType());
    if(cond->getSymPtr()->getType()->isInt() && itype->getSize() == 32)
    {
        cond=new IntToBool(cond);
        fprintf(stderr,"WhileStmt Cond Cast to Bool!\n");
    }
    
    
}
BinaryExpr::BinaryExpr(SymbolEntry *se, int op, ExprNode*expr1, ExprNode*expr2 ): ExprNode(se){
    
    this->op=op;
    this->expr1=expr1;
    this->expr2=expr2;
    dst = new Operand(se);
    whetherRel=false;

    //void不能参与二元运算
    Type *type1 = expr1->getSymPtr()->getType();
    Type *type2 = expr2 -> getSymPtr() -> getType();
    if(expr1 -> getSymPtr() -> getType()->isFunc()){
        FunctionType * ftype=(FunctionType *)(expr1 -> getSymPtr() -> getType());
        type1=ftype->getRetType();
        if(type1->isVoid()){
            fprintf(stderr,"void can't be a BinaryExpr operand1\n");
            exit(EXIT_FAILURE);
        }
    }   
    if(expr2 -> getSymPtr() -> getType()->isFunc()){
        FunctionType * ftype=(FunctionType *)(expr2 -> getSymPtr() -> getType());
        type2=ftype->getRetType();
        if(type2->isVoid()){
            fprintf(stderr,"void can't be a BinaryExpr operand2\n");
            exit(EXIT_FAILURE);
        }
    }

    
    if (op == BinaryExpr::AND || op == BinaryExpr::OR) {
        IntType * itype1=(IntType *)(expr1->getSymPtr()->getType());
        IntType * itype2=(IntType *)(expr2->getSymPtr()->getType());
        //debug:if (set_a(0) && set_b(1)) 
        if (expr1->getSymPtr()->getType()->isFunc() ){
            FunctionType * ftype=( FunctionType *)(expr1->getSymPtr()->getType());
            Type * rtype=ftype->getRetType();
            IntType * i1 =(IntType *)rtype;
            if(rtype->isInt() && i1->getSize() == 32 ){
                expr1=new IntToBool(expr1);
                fprintf(stderr,"BinaryExpr1(isFuncCall) Cast to Bool!\n");
                //fprintf(stderr,"BinaryExpr1(isFuncCall) Cast to Bool!type:%s\n",expr1->getSymPtr()->getType()->toStr().c_str());
            }     
        }
        if (expr2->getSymPtr()->getType()->isFunc() ){
            FunctionType * ftype=( FunctionType *)(expr2->getSymPtr()->getType());
            Type * rtype=ftype->getRetType();
            IntType * i2 =(IntType *)rtype;
            if(rtype->isInt() && i2->getSize() == 32 ){
                expr2=new IntToBool(expr2);
                fprintf(stderr,"BinaryExpr2(isFuncCall) Cast to Bool!\n");
                //fprintf(stderr,"BinaryExpr2(isFuncCall) Cast to Bool!type:%s\n",expr2->getSymPtr()->getType()->toStr().c_str());
            }     
        }
        //debugover
        if (expr1->getSymPtr()->getType()->isInt() && itype1->getSize() == 32) {
            expr1=new IntToBool(expr1);
            fprintf(stderr,"BinaryExpr1 Cast to Bool!\n");
            //fprintf(stderr,"BinaryExpr1 Cast to Bool!type:%s\n",expr1->getSymPtr()->getType()->toStr().c_str());
        }
        if (expr2->getSymPtr()->getType()->isInt() && itype2->getSize() == 32) {
            expr2=new IntToBool(expr2);
            fprintf(stderr,"BinaryExpr2 Cast to Bool!\n"); 
            //fprintf(stderr,"BinaryExpr2 Cast to Bool!type:%s\n",expr2->getSymPtr()->getType()->toStr().c_str());
                
        }  
    }
    
    
}