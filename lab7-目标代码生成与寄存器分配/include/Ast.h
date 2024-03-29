#ifndef __AST_H__
#define __AST_H__

#include <fstream>
#include "Operand.h"
#include <stack>
#include "Type.h"

class SymbolEntry;
class Unit;
class Function;
class BasicBlock;
class Instruction;
class IRBuilder;


//节点基类
class Node
{
private:
    static int counter;
    int seq;

protected:
    //真假分支列表？
    std::vector<Instruction*> true_list;
    std::vector<Instruction*> false_list;
    //中间代码构造辅助类
    static IRBuilder *builder;
    //一堆指令和一个基本块,list中都是跳转指令，使其跳转到bb上
    void backPatch(std::vector<Instruction*> &list, BasicBlock*bb);
    void backPatchelse(std::vector<Instruction*> &list, BasicBlock*bb);//自己lab6加

    //合并，应该是要将两个指令的动态数组合并成一个数组
    std::vector<Instruction*> merge(std::vector<Instruction*> &list1, std::vector<Instruction*> &list2);

public:
    Node * next;    //通过next指针将不同的节点串联起来，用于变量批量声明时，在同一级打印出各个声明语句；参数列表串起来；数组每个维度串起来
    //新建变量：下一个节点指针
    Node();
    int getSeq() const {return seq;};
    static void setIRBuilder(IRBuilder*ib) {builder = ib;};
    virtual void output(int level) = 0;
    virtual void typeCheck(Type* rtnType = nullptr) = 0;
    virtual void genCode() = 0;
    std::vector<Instruction*>& trueList() {return true_list;}
    std::vector<Instruction*>& falseList() {return false_list;}

    void setNext(Node* node);
    Node* getNext() { return next; }

};

//表达式节点
class ExprNode : public Node
{
protected:
    SymbolEntry *symbolEntry;
    Operand *dst;   // The result of the subtree is stored into dst.
public:
    bool whetherRel;//新加
    bool compare_inst_forunary;
    bool ifinitexp=false;//数组新加
    ExprNode(SymbolEntry *symbolEntry) : symbolEntry(symbolEntry){compare_inst_forunary=false;};
    Operand* getOperand() {return dst;};
    void setOperand(Operand* newOpe){dst=newOpe;}
    SymbolEntry* getSymPtr() {return symbolEntry;};

    virtual int getValue() {return 0;};
    virtual float getFloatValue(){return 0.0;};
    void output(int level) {};
    void typeCheck(Type* rtnType = nullptr){};
    void genCode(){};
};
//1.常量节点
class Constant : public ExprNode
{
public:
    Constant(SymbolEntry *se) : ExprNode(se){dst = new Operand(se);};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();

    int getValue();
    float getFloatValue();
};
//2.ID节点
class Id : public ExprNode
{
private:
    ExprNode* arindex;
    bool left = false;
public:
    Id(SymbolEntry *se,ExprNode* arindex = nullptr) : ExprNode(se), arindex(arindex) 
    {
        //fprintf(stderr, "Id结点构造函数,%s\n",((IdentifierSymbolEntry *)(this->getSymPtr()))->getName().c_str());
        //SymbolTable::getLabel()是获取临时变量的标号
        //区分数组
        if(se->getType()->isArray()){
            SymbolEntry* temp = new TemporarySymbolEntry(new PointerType(((ArrayType*)(se->getType()))->getArrayType()),SymbolTable::getLabel());
            dst = new Operand(temp);
        }
        else{
            SymbolEntry *temp = new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel()); 
            dst = new Operand(temp);
        }
    };
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();

    //这里的getSymbolEntry函数在基类ExprNode中已经以getSymPtr的形式实现了，
    //SymbolEntry* getSymbolEntry(){return symbolEntry;}//新增

    int getValue();
    float getFloatValue();
    ExprNode* getArrayIndex() { return arindex; };
    Type* getIdType();
    bool isLeft() const { return left; };
    void setLeft() { left = true; }
};
//3.二元运算节点
class BinaryExpr : public ExprNode
{
private:
    //两个表达式加减与或，小于，大于后仍是一个表达式
    int op;
    ExprNode *expr1, *expr2;
public:

    enum {ADD, SUB,MUL,DIV,MOD, AND, OR, LT,GT,LE, GE,EQ,NE};
    //用se初始化了自己的符号表项，
    BinaryExpr(SymbolEntry *se, int op, ExprNode*expr1, ExprNode*expr2);
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();

    int getValue();
    float getFloatValue();
};
//4.一元运算节点
class UnaryExpr : public ExprNode 
{     
private: 
    int op; 
    ExprNode* expr; 
public: 
    enum { ADD,NOT,SUB };  //ADD
    UnaryExpr(SymbolEntry* se, int op, ExprNode* expr); 
    void output(int level); 
    int getOp() const { return op; };
    void typeCheck(Type* rtnType = nullptr);
    void genCode();

    int getValue();
    float getFloatValue();
}; 

class IntToBool:public ExprNode
{
public:
    ExprNode* expr;
    IntToBool(ExprNode* expr);
    void genCode();
    void output(int level);
    void typeCheck(Type* rtnType );
};

class IntToFloat:public ExprNode
{
public:
    ExprNode* expr;
    float fv;
    IntToFloat(ExprNode* expr,int iv);
    void genCode();
    void output(int level);
    void typeCheck(Type* rtnType );
};
class FloatToInt:public ExprNode
{
public:
    ExprNode* expr;
    int iv;
    FloatToInt(ExprNode* expr,float fv);
    void genCode();
    void output(int level);
    void typeCheck(Type* rtnType );
};

//初值表达式语句
class Initlist : public ExprNode {
   private:
    ExprNode* exp;
    int num;

   public:
    Initlist(SymbolEntry* se, ExprNode* exp = nullptr) : ExprNode(se), exp(exp) {num = 0; ifinitexp=true;};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
    //
    ExprNode* getExp() const { return exp; };
    void AddExp(ExprNode* exp){
        if (this->exp == nullptr) {
            if(num == 0)
                num++;
        this->exp = exp;
    } else {
        num++;
        this->exp->setNext(exp);
    }
    };
    bool ifempty() { return num == 0; };
    bool ifFull(){ return num == ((ArrayType*)(this->symbolEntry->getType()))->getArrayNum();};
    void fill(){
        Type* type = ((ArrayType*)(this->symbolEntry->getType()))->getArrayType();
        if (type->isArray()) {
            while (!ifFull())
                this->AddExp(new Initlist(new ConstantSymbolEntry(type)));
            ExprNode* temp = exp;
            while (temp) {
                ((Initlist*)temp)->fill();
                temp = (ExprNode*)(temp->getNext());
            }
        }
        if (type->isInt()) {
            while (!ifFull())
                this->AddExp(new Constant(new ConstantSymbolEntry(type, 0)));
            return;
        }
    };
};

//语句节点
class StmtNode : public Node
{};

//1.复合语句
class CompoundStmt : public StmtNode
{
private:
    StmtNode *stmt;
public:
    CompoundStmt(StmtNode *stmt=nullptr) : stmt(stmt) {};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
};

//2.序列语句
class SeqNode : public StmtNode
{
private:
    StmtNode *stmt1, *stmt2;
public:
    SeqNode(StmtNode *stmt1, StmtNode *stmt2) : stmt1(stmt1), stmt2(stmt2){};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
};

//3.声明语句
class DeclStmt : public StmtNode
{
private:
    Id *id;
    ExprNode* InitValue;
public:
    DeclStmt(Id *id,ExprNode*InitValue=nullptr ) : id(id){
        if(InitValue!=nullptr){
            this->InitValue=InitValue;
            if(this->InitValue->ifinitexp)  {
                ((Initlist* )(this->InitValue))->fill();
            }

        }    
    };
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();

    Id* getID(){return id;} //新增
};

//4.赋值语句
class AssignStmt : public StmtNode
{
private:
    ExprNode *lval;
    ExprNode *expr;
public:
    AssignStmt(ExprNode *lval, ExprNode *expr) : lval(lval), expr(expr) {};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
};
//5.条件语句无分支
class IfStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
public:
    IfStmt(ExprNode *cond, StmtNode *thenStmt);
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
};

//6.条件语句有分支
class IfElseStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
    StmtNode *elseStmt;
public:
    IfElseStmt(ExprNode *cond, StmtNode *thenStmt, StmtNode *elseStmt);
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
};

//7.循环语句
class WhileStmt : public StmtNode
{ 
private:
    ExprNode* cond;
    StmtNode* stmt;

    
public:
    BasicBlock* endBlock;
    BasicBlock* condBlock;
    WhileStmt(ExprNode* cond, StmtNode* stmt);
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();

    BasicBlock* getendBlock(){return endBlock;};
    BasicBlock* getcondBlock(){return condBlock;};
};

class loopctl:public StmtNode
{
public:
    WhileStmt *whileStmt;
    void setWhileStmt(WhileStmt *whileStmt){this->whileStmt=whileStmt;}

};

//8.break语句
class BreakStmt : public loopctl 
{
public:
    
    BreakStmt(){};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
};
//9.continue语句
class ContinueStmt : public loopctl 
{
public:
    ContinueStmt(){};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
};
//10.返回语句
class ReturnStmt : public StmtNode
{
private:
    ExprNode *retValue; //可以为nullptr表示return;
    Type * retType;//类型检查新加
public:
    ReturnStmt(ExprNode*retValue=nullptr) : retValue(retValue) {
        //fprintf(stderr, "返回语句结点\n");
         if(retValue==nullptr) {
            retType=TypeSystem::voidType;
        }
        else retType=retValue->getSymPtr()->getType();
    };
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
};

//11.函数声明语句
class FunctionDef : public StmtNode
{
private:
    SymbolEntry *se;
    StmtNode* paramDecl;
    StmtNode *stmt;
public:
    FunctionDef(SymbolEntry *se, StmtNode* paramDecl, StmtNode *stmt) : se(se),paramDecl(paramDecl), stmt(stmt){};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
    SymbolEntry * getse(){return se;};
};

//12.函数调用语句
class FunctionCall : public ExprNode
{
private:
    Node* paramGiven;
public:
    FunctionCall(SymbolEntry *se,Node* paramGiven);
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();

    int getValue();
    float getFloatValue();
    Node* getparamGiven(){return paramGiven;};
    void setparamGiven(Node* n){paramGiven=n;}//参数的i2f，需要重置
};

//13.空语句
class BlankStmt : public StmtNode 
{ //新加StmtNode
public:
    BlankStmt(){};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();

};

//14.表达式语句
class ExprStmt : public StmtNode 
{  //新加StmtNode
private:
    ExprNode* se;

public:
    ExprStmt(ExprNode* se) : se(se){};
    void output(int level);
    void typeCheck(Type* rtnType = nullptr);
    void genCode();
};


class Ast
{
private:
    Node* root;
public:
    Ast() {root = nullptr;}
    void setRoot(Node*n) {root = n;}
    void output();
    void typeCheck(Type* rtnType = nullptr);
    void genCode(Unit *unit);
};

#endif
