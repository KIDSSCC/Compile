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
extern MachineUnit mUnit;

extern FILE *yyout;
int Node::counter = 0;
IRBuilder* Node::builder = nullptr;

bool ifFuncHasRtnstmt=0;//类型检查返回值
int boolcastvalue=0;



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

void Node::backPatchelse(std::vector<Instruction*> &list, BasicBlock*bb)//改进
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
    if (stmt)
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
            if(truebranch->empty())
            {
                if (((FunctionType*)(se->getType()))->getRetType() ==TypeSystem::intType)
                {
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),truebranch);
                }
                if (((FunctionType*)(se->getType()))->getRetType() ==TypeSystem::voidType)
                {
                    new RetInstruction(nullptr, truebranch);
                }
            }
            if(falsebranch->empty())
            {
                if (((FunctionType*)(se->getType()))->getRetType() ==TypeSystem::intType)
                {
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),falsebranch);
                }
                if (((FunctionType*)(se->getType()))->getRetType() ==TypeSystem::voidType)
                {
                    new RetInstruction(nullptr, falsebranch);
                }
            }


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
   //debug:void无返回语句时也要打印ret void,但还是不对，2-4 068

   /*
   BasicBlock * lastblock=func->getLastBlock();
   if(((FunctionType*)(se->getType()))->getRetType()->isVoid()) {
        if(ifFuncHasRtnstmt==0){
            fprintf(stderr, "void 无返回语句\n");
            new RetInstruction(nullptr, lastblock);
        }       
        else{
            fprintf(stderr, "void 有返回语句\n");
        }
            
    } */
}

void generateCtrlStream(IRBuilder* builder,ExprNode* cond)
{
    BasicBlock *bb = builder->getInsertBB();

    if(cond->compare_inst_forunary)
    {
        //单目运算在这里需要比较语句
        Operand* tem=new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
        new CmpInstruction(CmpInstruction::NE, tem, cond->getOperand(),new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),bb);
        cond->setOperand(tem);
    }

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
    fprintf(yyout, "%*cIntToBoolExpr\n", level, ' ');
    this->expr->output(level + 4);
}

void IntToBool::typeCheck(Type* rtnType )
{

}

IntToFloat::IntToFloat(ExprNode* expr,int iv):ExprNode(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()))
{
    this->expr=expr;
    this->fv=(float)iv;
    dst=new Operand(symbolEntry);
}
FloatToInt::FloatToInt(ExprNode* expr,float fv):ExprNode(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()))
{
    this->expr=expr;
    this->iv=(int)fv;
    dst=new Operand(symbolEntry);
    
}
void IntToFloat::output(int level)
{

    fprintf(yyout, "%*cIntToFloatExpr\t\t after float value:%f\n", level, ' ',fv);
    this->expr->output(level + 4);
}
void FloatToInt::output(int level)
{
    fprintf(yyout, "%*cFloatToIntExpr\t\t after int value:%d\n", level, ' ',iv);
    this->expr->output(level + 4);
}

void IntToFloat::typeCheck(Type* rtnType )
{

}
void FloatToInt::typeCheck(Type* rtnType )
{

}
void IntToFloat::genCode(){
    expr->genCode();
    BasicBlock* bb = builder->getInsertBB();
    //new i2f inst
    new I2FInstruction(dst, this->expr->getOperand(), bb);
}
void FloatToInt::genCode(){
    expr->genCode();
    BasicBlock* bb = builder->getInsertBB();
    //new f2i inst
    new F2IInstruction(dst, this->expr->getOperand(), bb);
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
        int opcode=-1;//改
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
        int opcode=-1;//改
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
    whetherRel=true;
    compare_inst_forunary=true;
    //debug 2-4 070 if(!dp[i][j]||aa<dp[i][j])]
    if(op==NOT){
        //涉及整型转bool
        if(expr->getSymPtr()->getType()->isArray()){   
            ArrayType * atype=(ArrayType *)(expr -> getSymPtr() -> getType());
            Type * type=atype->getFinalType();
            IntType * i1 =(IntType *)type;
            if(type->isInt() && i1->getSize() == 32 )
            {
                expr=new IntToBool(expr);
            }
            
        }
        if(expr->getSymPtr()->getType()->isFunc()){
            FunctionType * ftype=( FunctionType *)(expr->getSymPtr()->getType());
            Type * rtype=ftype->getRetType();
            IntType * i1 =(IntType *)rtype;
            if(rtype->isInt() && i1->getSize() == 32 ){
                expr=new IntToBool(expr);
            }   
        }
        /*IntType * itype1=(IntType *)(expr->getSymPtr()->getType());
        if(expr->getSymPtr()->getType()->isInt() && itype1->getSize() == 32){
            this->expr=new IntToBool(expr);    
        }*/
    }
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
            if(expr->getSymPtr()->getType()->isFloat()) src1 =new Operand(new ConstantSymbolEntry(TypeSystem::floatType, float(0)));
            else src1 =new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
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
            if(expr->getSymPtr()->getType()->isArray()){
                Operand* tem=new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
                new CmpInstruction(CmpInstruction::NE, tem, src,new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),bb);
                src=tem;
            }
            if(expr->getSymPtr()->getType()->isFunc()){
                Operand* tem=new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
                new CmpInstruction(CmpInstruction::NE, tem, src,new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),bb);
                src=tem;
            }
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
    if(symbolEntry->getType()->isArray()){
        //完善浮点
        if (arindex) {
            Type* type = ((ArrayType*)(this->getSymPtr()->getType()))->getArrayType();
            Type* type1 = this->getSymPtr()->getType();
            Operand* tempSrc = addr;
            Operand* tempDst = dst;
            ExprNode* idx = arindex;
            bool flag = false;
            bool pointer = false;
            bool firstFlag = true;
            while (true) {
                if (((ArrayType*)type1)->getArrayNum() == 0) {//数组debug
                    Operand* dst1 = new Operand(new TemporarySymbolEntry(new PointerType(type), SymbolTable::getLabel()));
                    tempSrc = dst1;
                    new LoadInstruction(dst1, addr, bb);
                    flag = true;
                    firstFlag = false;
                }
                if (!idx) {
                    Operand* dst1 = new Operand(new TemporarySymbolEntry(new PointerType(type), SymbolTable::getLabel()));
                    Operand* idx = new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
                    new GepInstruction(dst1, tempSrc, idx, bb);
                    tempDst = dst1;
                    pointer = true;
                    break;
                }
                idx->genCode();
                auto gep = new GepInstruction(tempDst, tempSrc,idx->getOperand(), bb, flag);
                if (!flag && firstFlag) {
                    gep->setFirst();
                    firstFlag = false;
                }
                if (flag)
                    flag = false;
                if (type == TypeSystem::intType ||type == TypeSystem::intconstType || type == TypeSystem::floatType ||type == TypeSystem::floatconstType )
                    break;
                type = ((ArrayType*)type)->getArrayType();
                type1 = ((ArrayType*)type1)->getArrayType();
                tempSrc = tempDst;
                tempDst = new Operand(new TemporarySymbolEntry( new PointerType(type), SymbolTable::getLabel()));
                idx = (ExprNode*)(idx->getNext());
            }
            dst = tempDst;
            // 如果是右值还需要一条load
            if (!left && !pointer) {
                Type* atype = ((ArrayType*)(this->getSymPtr()->getType()))->getArrayType();
                Operand* dst1;
                if(atype->isFloat()) dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
                else dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
                new LoadInstruction(dst1, dst, bb);
                dst = dst1;
            }
        } else {
            // 这里先这么办 后续有问题再改
            if (((ArrayType*)(this->getSymPtr()->getType()))->getArrayNum() == 0) {//数组debug
                Operand* dst1 = new Operand(new TemporarySymbolEntry(new PointerType( ((ArrayType*)(this->getSymPtr()->getType()))->getArrayType()),SymbolTable::getLabel()));
                new LoadInstruction(dst1, addr, bb);
                dst = dst1;

            } else {
                Operand* idx = new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
                auto gep = new GepInstruction(dst, addr, idx, bb);
                gep->setFirst();
            }
        }
    }
        
    else {
        new LoadInstruction(dst, addr, bb);
    }
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
        mUnit.global_id_list.push_back(se);//新加
        
    }
    if(se->isLocal())
    {
        //查找到了当前属于哪一个函数
        Function *func = builder->getInsertBB()->getParent();
        BasicBlock *entry = func->getEntry();
        PointerType* tmpType = new PointerType(se->getType());
        SymbolEntry *dstSymbolentry = new TemporarySymbolEntry(tmpType, SymbolTable::getLabel());
        Operand *addr = new Operand(dstSymbolentry);
        Instruction *alloca = new AllocaInstruction(addr, se);                 // allocate space for local id in function stack.
        entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block.
        se->setAddr(addr);                                          // set the addr operand in symbol entry so that we can use it in subsequent code generation.

        //是否进行了初始化
        if(InitValue!=nullptr)
        {
            if(InitValue->ifinitexp){//数组初始化
                Operand* init = nullptr;
                BasicBlock* bb = builder->getInsertBB();
                ExprNode* temp = InitValue;
                std::stack<ExprNode*> stk;
                std::vector<int> idx;
                idx.push_back(0);
                while (temp) {
                    if (temp->ifinitexp) 
                    {
                        stk.push(temp);
                        idx.push_back(0);
                        temp = ((Initlist*)temp)->getExp();
                        continue;
                    } 
                    else 
                    {
                        temp->genCode();
                        Type* type =((ArrayType*)(se->getType()))->getArrayType();
                        Operand* tempSrc = addr;
                        Operand* tempDst;
                        Operand* index;
                        bool flag = true;
                        int i = 1;
                        while (true) {
                            tempDst = new Operand(new TemporarySymbolEntry(new PointerType(type), SymbolTable::getLabel()));
                            index = (new Constant(new ConstantSymbolEntry(TypeSystem::intType, idx[i++]))) ->getOperand();
                            auto gep =new GepInstruction(tempDst, tempSrc, index, bb);
                            gep->setInit(init);
                            if (flag) {
                                gep->setFirst();
                                flag = false;
                            }
                            if (type == TypeSystem::intType ||type == TypeSystem::intconstType || type == TypeSystem::floatType ||type == TypeSystem::floatconstType){
                                gep->setLast();
                                init = tempDst;
                                break;
                            }
                            type = ((ArrayType*)type)->getArrayType();
                            tempSrc = tempDst;
                        }
                        new StoreInstruction(tempDst, temp->getOperand(), bb);
                    }
                    while (true) {
                        if (temp->getNext()) {
                            temp = (ExprNode*)(temp->getNext());
                            idx[idx.size() - 1]++;
                            break;
                        } else {
                            temp = stk.top();
                            stk.pop();
                            idx.pop_back();
                            if (stk.empty())
                                break;
                        }
                    }
                    if (stk.empty())
                        break;
                }
            }
            else{
                BasicBlock* bb = builder->getInsertBB();
                InitValue->genCode();
                Operand* src = InitValue->getOperand();
                new StoreInstruction(addr, src, bb);
            } 
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
    Operand* returnValue=nullptr;//return;段错误的debug！！！！一定初始化为nullptr
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
    Operand *addr = nullptr;
    //判断数组
    if(lval->getSymPtr()->getType()->isArray()){
        ((Id*)lval)->setLeft();
        lval->genCode();
        addr = lval->getOperand();
    }
    else{
        addr=dynamic_cast<IdentifierSymbolEntry*>(lval->getSymPtr())->getAddr();
    }
    Operand *src = expr->getOperand();
    //if(src->getEntry()->getType()->isInt())fprintf(stderr,"1532****\n");
    /***
     * We haven't implemented array yet, the lval can only be ID. So we just store the result of the `expr` to the addr of the id.
     * If you want to implement array, you have to caculate the address first and then store the result into it.
     */
    new StoreInstruction(addr, src, bb);
}

bool whetherSysy(std::string name)
{
    if((name=="putint")||(name=="getint")||(name=="putch")||(name=="getch")||(name=="putfloat")||(name=="getfloat")||(name=="putarray")||(name=="getarray")||(name=="putfarray")||(name=="getfarray"))
    {
        return true;
    }
    return false;
}

FunctionCall::FunctionCall(SymbolEntry *se,Node* paramGiven) : ExprNode(se),paramGiven(paramGiven)
{
    //构造函数，获取到的参数为函数名的符号表项和可能存在的参数列表。
    //函数的返回值，同时也是本次函数调用语句的类型。
    dst=nullptr;
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
void Initlist::genCode() {
    // Todo
}
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
    expr1 -> typeCheck(rtnType);
    expr2 -> typeCheck(rtnType);

    // Todo（参考指导书）
    
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
    //3.5 数组取最终类型
    if(expr1 -> getSymPtr() -> getType()->isArray()){
        //fprintf(stderr,"expr1 is functype\n");
        ArrayType * atype=(ArrayType *)(expr1 -> getSymPtr() -> getType());
        //fprintf(stderr,"return type is %s\n",ftype->getRetType()-> toStr().c_str());
        type1=atype->getFinalType();

    }  
    if(expr2 -> getSymPtr() -> getType()->isArray()){
        //fprintf(stderr,"expr1 is functype\n");
        ArrayType * atype=(ArrayType *)(expr2 -> getSymPtr() -> getType());
        //fprintf(stderr,"return type is %s\n",ftype->getRetType()-> toStr().c_str());
        type2=atype->getFinalType();

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
        //数组
        if (expr1->getSymPtr()->getType()->isArray() ){
            ArrayType * atype=( ArrayType *)(expr1->getSymPtr()->getType());
            Type * rtype=atype->getFinalType();
            IntType * i1 =(IntType *)rtype;
            if(rtype->isInt() && i1->getSize() == 32 ){
                type1=TypeSystem::boolType;
            }     
        }
        if (expr2->getSymPtr()->getType()->isArray() ){
            ArrayType * atype=( ArrayType *)(expr2->getSymPtr()->getType());
            Type * rtype=atype->getFinalType();
            IntType * i1 =(IntType *)rtype;
            if(rtype->isInt() && i1->getSize() == 32 ){
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
    //float和int做二元运算，放过
    if(  (type1 == TypeSystem::floatType) && (type2 == TypeSystem::intType)  ){
        type2=TypeSystem::floatType;
    }
    if(  (type2 == TypeSystem::floatType) && (type1 == TypeSystem::intType)  ){
        type1=TypeSystem::floatType;
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
void Initlist::typeCheck(Type* rtnType) 
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
    
    //debug3:
    if(type2->isArray() ){
            type2 = ((ArrayType *)type2)-> getFinalType();   
    } 
    //debug2: return getInt(),要求inttype，但返回funcType，应该取函数类型的返回值类型！
    if(type2->isFunc() ){
            type2 = ((FunctionType *)type2)-> getRetType();   
    } 
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
    //float 但是return 0；放过（2-6的35.sy）
    if(type1->isFloat() && type2->isInt() ){
        type2=TypeSystem::floatType;
        //这里做类型转化，因为check在gencode前，可以影响
        this->retValue= new IntToFloat(retValue,retValue->getValue()); 
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
    
    while(temp1) {
        Type * shicantype;
        if(temp1->getSymPtr()->getType()->isArray()) shicantype= ((ArrayType  *)(temp1->getSymPtr()->getType()))-> getFinalType();
        else if(temp1->getSymPtr()->getType()->isFunc() ){
            shicantype = ((FunctionType *)(temp1->getSymPtr()->getType()))-> getRetType();   
        }   
        else shicantype=temp1->getSymPtr()->getType();
        Type * xingcantype;
        if(xingparamsType[i]->isArray())    xingcantype=((ArrayType  *)(xingparamsType[i]))-> getFinalType();
        else if(xingparamsType[i]->isFunc() ){
            xingcantype= ((FunctionType *)(xingparamsType[i]))-> getRetType();   
        } 
        else xingcantype=xingparamsType[i];
        if(xingcantype->getkind() != shicantype->getkind() ){
            if(xingcantype->isFloat() && shicantype->isInt()){
                //是首参
                if(temp1==(ExprNode*)(this->getparamGiven())){
                    ExprNode* change=new IntToFloat(temp1,temp1->getValue());
                    this->setparamGiven(change);
                    change->setNext(temp1->getNext());
                } 
                //不是首参
                else{
                    //1.寻找上一个prev
                    ExprNode* prev ;
                    ExprNode* head = (ExprNode*)(this->getparamGiven());
                    while (head) {
                        if(head->getNext() == temp1){
                            prev=head;
                            break;
                        }
                        head = (ExprNode*)(head->getNext());
                    }
                    //2.更换结点,用change替换temp1
                    ExprNode* change=new IntToFloat(temp1,temp1->getValue());//这里怎么赋值给真正的参数结点？
                    change->setNext(temp1->getNext());
                    prev->next=change;//别用setNext()!!!
                }
            }
            else if(xingcantype->isInt() && shicantype->isFloat()){
                //是首参
                if(temp1==(ExprNode*)(this->getparamGiven())){
                    ExprNode* change=new FloatToInt(temp1,temp1->getFloatValue());
                    this->setparamGiven(change);
                    change->setNext(temp1->getNext());
                } 
                //不是首参
                else{
                    //1.寻找上一个prev
                    ExprNode* prev ;
                    ExprNode* head = (ExprNode*)(this->getparamGiven());
                    while (head) {
                        if(head->getNext() == temp1){
                            prev=head;
                            break;
                        }
                        head = (ExprNode*)(head->getNext());
                    }
                    //2.更换结点,用change替换temp1
                    ExprNode* change=new FloatToInt(temp1,temp1->getFloatValue());//这里怎么赋值给真正的参数结点？
                    change->setNext(temp1->getNext());
                    prev->next=change;//别用setNext()!!!
                }
            }
            else {
                    flag=false;
                    index=i;
                    tempindex=temp1;
                    break;   
            }
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
        case MUL:
            op_str = "mul";
            break;
        case DIV:
            op_str = "div";
            break;
        case MOD:
            op_str = "mod";
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
    std::string type = symbolEntry->getType()->toStr();
    fprintf(yyout, "%*cBinaryExpr\top: %s\ttype:%s\n", level, ' ', op_str.c_str(),type.c_str());
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
    fprintf(stderr, "%*cIntegerLiteral\tvalue: %s\ttype: %s\n", level, ' ',
            value.c_str(), type.c_str());
}
Type* Id::getIdType()   //这里没用，可以删了？？
{
    SymbolEntry* se = this->getSymPtr();
    if (!se)
        return TypeSystem::voidType;
    Type* type = se->getType();
    if (!arindex)
    {
        return type;
    }
    else //数组情况
    {
        ArrayType* temp1 = (ArrayType*)type;
        ExprNode* temp2 = arindex;
        while (!temp1->getArrayType()->isInt()) 
        {
            if (!temp2) 
            {
                return temp1;
            }
            temp2 = (ExprNode*)(temp2->getNext());
            temp1 = (ArrayType*)(temp1->getArrayType());
        }
    }
    return TypeSystem::intType;//float咋办？？
}
void Id::output(int level)
{
    
    std::string name, type;
    int scope;
    name = symbolEntry->toStr();
    type = symbolEntry->getType()->toStr();
    scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
    if(this->getSymPtr()->getType()->isFloat()) fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\tvalue:%f\n", level, ' ',name.c_str(), scope, type.c_str(),this->getFloatValue());
    else fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\tvalue:%d\n", level, ' ',name.c_str(), scope, type.c_str(),this->getValue());
    fprintf(stderr, "%*cId\tname: %s\tscope: %d\ttype: %s,\n", level, ' ',
            name.c_str(), scope, type.c_str());
    if(this->getSymPtr()->getType()->isFloat())  fprintf(stderr,"here!033,%f\n",((ConstantSymbolEntry *)(this->getSymPtr()))->getFloatValue());
    
    
    /*if (arindex) {//数组新加
        ExprNode* temp = arindex;
        int i = 0;
        while (temp) {
            if(temp->ifinitexp) fprintf(stderr, "是initexp！！\n");
            temp->output(level + 4 + 4 * i++);
            temp = (ExprNode*)(temp->getNext());
        }
    }*/
    //fprintf(stderr, "id的output\n");
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
    fprintf(stderr, "%*cCompoundStmt\n", level, ' ');
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
    fprintf(stderr, "%*cDeclStmt\n", level, ' ');
    id->output(level + 4);
    //加！！
    if(InitValue!=nullptr){
        //fprintf(stderr, "decl的InitValue!=nullptr\n");
        InitValue->output(level + 4);
    }
    if (this->getNext()) {
        this->getNext()->output(level);
    }
    //fprintf(stderr, "decl的output\n");
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
    fprintf(stderr, "%*cReturnStmt\n", level, ' ');
    if(retValue!=nullptr){
        retValue->output(level + 4);
    }
       
    else {
        fprintf(stderr, "%*cvoidLiteral\ttype: %s\n", level+4, ' ',
            "void");
        
        fprintf(yyout, "%*cvoidLiteral\ttype: %s\n", level+4, ' ',
            "void");
        }
}
void Initlist::output(int level) {
    //fprintf(stderr, "initlist的output\n");
    std::string str;
    if (symbolEntry->getType())
        str = symbolEntry->getType()->toStr();
    fprintf(yyout, "%*cInitListExpr\ttype: %s\n", level, ' ',str.c_str());
    fprintf(stderr, "%*cInitListExpr\ttype: %s\n", level, ' ',str.c_str());
    Node* temp = exp;
    while (temp) {
        temp->output(level + 4);
        temp = temp->getNext();
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
    fprintf(stderr, "%*cFunctionDefine function name: %s, type: %s\n", level, ' ', 
            name.c_str(), type.c_str());
    stmt->output(level + 4);
}

void FunctionCall::output(int level)
{

    if (symbolEntry) {
        std::string name = symbolEntry->toStr();
        std::string type = symbolEntry->getType()->toStr();
        int scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
        fprintf(yyout, "%*cFunctionCall\tfunction name: %s\tscope: %d\ttype: %s\n",
                level, ' ', name.c_str(), scope, type.c_str());
        Node* temp = paramGiven;
        while (temp) {
            temp->output(level + 4);
            temp = temp->getNext();
        }
    }
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
float Constant::getFloatValue()
{
    return dynamic_cast<ConstantSymbolEntry*>(symbolEntry)->getFloatValue();
}

int Id::getValue()
{
    return dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getIntValue();
}
float Id::getFloatValue()
{
    return dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getFloatValue();
}

int BinaryExpr::getValue()
{
    int src1=expr1->getValue();
    int src2=expr2->getValue();
    int result=0;//改
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
            if(expr2->getValue())
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
float BinaryExpr::getFloatValue()
{
    float src1=expr1->getFloatValue();
    float src2=expr2->getFloatValue();
    float result;
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
            if(src2==0) {
                result=-1.0;//非法，可能值没够，还不能算
            }
            else result=src1/src2;
            break;
        case MOD:
            result=-1.0;//非法
            break;
        case AND:
            result=(src1&&src2);
            break;
        case OR:
            result=(src1||src2);
            break;
        case LT:
            result=(src1<src2);
            break;
        case GT:
            result=(src1>src2);
            break;
        case LE:
            result=(src1<=src2);
            break;
        case GE:
            result=(src1>=src2);
            break;
        case EQ:
            result=(src1==src2);
            break;
        case NE:
            result=(src1!=src2);//好奇怪
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

float UnaryExpr::getFloatValue()
{
    float src=expr->getFloatValue();
    float result;
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

float FunctionCall::getFloatValue()
{
    return 0.0;//完善？？
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
            this->cond=new IntToBool(cond);
            fprintf(stderr,"IfStmt Cond(isFuncCall) Cast to Bool!\n");
        }     
    }
    //debugover
    IntType * itype=(IntType *)(cond->getSymPtr()->getType());
    if(cond->getSymPtr()->getType()->isInt() && itype->getSize() == 32)
    {
        this->cond=new IntToBool(cond);
        fprintf(stderr,"IfStmt Cond Cast to Bool!\n");
    }
    //level2-4 68
    if(cond->getSymPtr()->getType()->isArray())
    {
        auto array=dynamic_cast<ArrayType*>(cond->getSymPtr()->getType());
        if(array->getFinalType()->isInt())
        {
            this->cond=new IntToBool(cond);
            fprintf(stderr,"IfStmt Cond Cast to Bool!\n");
        }
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
            this->cond=new IntToBool(cond);
        }     
    }
    //debugover 
    IntType * itype=(IntType *)(cond->getSymPtr()->getType());
    if(cond->getSymPtr()->getType()->isInt()&& itype->getSize() == 32)
    {
        this->cond=new IntToBool(cond);
        fprintf(stderr,"IfElseStmt Cond Cast to Bool!\n");
    }
    if(cond->getSymPtr()->getType()->isArray())
    {
        auto array=dynamic_cast<ArrayType*>(cond->getSymPtr()->getType());
        if(array->getFinalType()->isInt())
        {
            this->cond=new IntToBool(cond);
            fprintf(stderr,"IfStmt Cond Cast to Bool!\n");
        }
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
            this->cond=new IntToBool(cond);
        }     
    }
    //debugover 
    IntType * itype=(IntType *)(cond->getSymPtr()->getType());
    if(cond->getSymPtr()->getType()->isInt() && itype->getSize() == 32)
    {
        this->cond=new IntToBool(cond);
    }
    if(cond->getSymPtr()->getType()->isArray())
    {
        auto array=dynamic_cast<ArrayType*>(cond->getSymPtr()->getType());
        if(array->getFinalType()->isInt())
        {
            this->cond=new IntToBool(cond);
        }
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
        if (expr1->getSymPtr()->getType()->isFunc() )
        {
            FunctionType * ftype=( FunctionType *)(expr1->getSymPtr()->getType());
            Type * rtype=ftype->getRetType();
            IntType * i1 =(IntType *)rtype;
            if(rtype->isInt() && i1->getSize() == 32 ){
                this->expr1=new IntToBool(expr1);
            }     
        }
        if (expr2->getSymPtr()->getType()->isFunc() ){
            FunctionType * ftype=( FunctionType *)(expr2->getSymPtr()->getType());
            Type * rtype=ftype->getRetType();
            IntType * i2 =(IntType *)rtype;
            if(rtype->isInt() && i2->getSize() == 32 )
            {
                this->expr2=new IntToBool(expr2);
                fprintf(stderr,"BinaryExpr2(isFuncCall) Cast to Bool!\n");
            }     
        }
        //debug:if (a[1] && b[2]) 
        if (expr1->getSymPtr()->getType()->isArray() ){
            ArrayType * atype=( ArrayType *)(expr1->getSymPtr()->getType());
            Type * ftype=atype->getFinalType();
            IntType * i1 =(IntType *)ftype;
            if(ftype->isInt() && i1->getSize() == 32 ){
                this->expr1=new IntToBool(expr1);
                fprintf(stderr,"BinaryExpr1(isArray) Cast to Bool!\n");
               
            }     
        }
        if (expr2->getSymPtr()->getType()->isArray() ){
            
            ArrayType * atype=( ArrayType *)(expr2->getSymPtr()->getType());
            Type * ftype=atype->getFinalType();
            
            IntType * i1 =(IntType *)ftype;
            
            if(ftype->isInt() && i1->getSize() == 32 ){
                
                this->expr2=new IntToBool(expr2);
                fprintf(stderr,"BinaryExpr2(isArray) Cast to Bool!\n");
            }     
        }

        //debugover
        if (expr1->getSymPtr()->getType()->isInt() && itype1->getSize() == 32) {
            this->expr1=new IntToBool(expr1);
            fprintf(stderr,"BinaryExpr1 Cast to Bool!\n");
            //fprintf(stderr,"BinaryExpr1 Cast to Bool!type:%s\n",expr1->getSymPtr()->getType()->toStr().c_str());
        }
        if (expr2->getSymPtr()->getType()->isInt() && itype2->getSize() == 32) {
            this->expr2=new IntToBool(expr2);
            fprintf(stderr,"BinaryExpr2 Cast to Bool!\n"); 
            //fprintf(stderr,"BinaryExpr2 Cast to Bool!type:%s\n",expr2->getSymPtr()->getType()->toStr().c_str());
                
        }  
    }
    if(op == BinaryExpr::ADD || op == BinaryExpr::SUB || op == BinaryExpr::MUL || op == BinaryExpr::DIV || (op >= LT && op <= NE)){//暂时这样，之后有bug再补充
        Type * lefttype=expr1->getSymPtr()->getType();
        Type * righttype=expr2->getSymPtr()->getType();
        if (expr1->getSymPtr()->getType()->isFunc() ){
            FunctionType * ftype=( FunctionType *)(expr1->getSymPtr()->getType());
            lefttype=ftype->getRetType(); 
        }
        if (expr2->getSymPtr()->getType()->isFunc() ){
            FunctionType * ftype=( FunctionType *)(expr2->getSymPtr()->getType());
            righttype=ftype->getRetType(); 
        }
        if (expr1->getSymPtr()->getType()->isArray() ){
            ArrayType * atype=( ArrayType *)(expr1->getSymPtr()->getType());
            lefttype=atype->getFinalType();
              
        }
        if (expr2->getSymPtr()->getType()->isArray() ){
            ArrayType * atype=( ArrayType *)(expr2->getSymPtr()->getType());
           righttype=atype->getFinalType();
   
        }
        if (lefttype->isFloat() && righttype->isInt()) {
            this->expr2=new IntToFloat(expr2,expr2->getValue()) ;
            fprintf(stderr,"BinaryExpr2 Cast : int to float!\n"); 
        }
        if (lefttype->isInt() && righttype->isFloat()){
            this->expr1=new IntToFloat(expr1,expr1->getValue()) ;
            fprintf(stderr,"BinaryExpr1 Cast : int to float!\n"); 
        }
    }
    
    
}