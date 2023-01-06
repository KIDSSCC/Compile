#ifndef __AST_H__
#define __AST_H__

#include <fstream>

class SymbolEntry;

class Node
{
private:
    static int counter;
    int seq;        //seq打印层次结构   每产生一个node，都把计数器+1，并赋给seq
    //新建变量：下一个节点指针
    Node * next;    //通过next指针将不同的节点串联起来，用于变量批量声明时，在同一级打印出各个声明语句；参数列表串起来
public:
    Node();
    int getSeq() const {return seq;};
    virtual void output(int level) = 0;
    //对next的set和get
    void setNext(Node* node);
    Node* getNext() { return next; }
};

//表达式节点，是一个常量数值，变量ID，或者临时变量
class ExprNode : public Node        
{
protected:
    SymbolEntry *symbolEntry;
public:
    ExprNode(SymbolEntry *symbolEntry) : symbolEntry(symbolEntry){};
    //可以加个output
};
//1.常量结点
class Constant : public ExprNode
{
public:
    Constant(SymbolEntry *se) : ExprNode(se){};
    void output(int level);
};
//2.ID结点
class Id : public ExprNode
{
public:
    Id(SymbolEntry *se) : ExprNode(se){};
    void output(int level);
     SymbolEntry* getSymbolEntry(){return symbolEntry;}//新增
};
//3.二元运算结点
class BinaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr1, *expr2;
public:
    enum {ADD, SUB, MUL,DIV,MOD,AND, OR, LT,GT,EQ,NE,LE,GE};  //加
    //这里给了一个符号表项，是用来存储临时变量t1，t2...的
    BinaryExpr(SymbolEntry *se, int op, ExprNode*expr1, ExprNode*expr2) : ExprNode(se), op(op), expr1(expr1), expr2(expr2){};
    void output(int level);
};
//4.一元运算节点
class UnaryExpr : public ExprNode {     //加
 private: 
    int op; 
    ExprNode* expr; 
public: 
    enum { ADD,NOT, SUB };  //ADD
    UnaryExpr(SymbolEntry* se, int op, ExprNode* expr) : ExprNode(se), op(op), expr(expr){}; 
    void output(int level); 
 }; 

//语句节点
class StmtNode : public Node
{};
//1.复合节点？
class CompoundStmt : public StmtNode
{
private:
    StmtNode *stmt;
public:
    CompoundStmt(StmtNode *stmt) : stmt(stmt) {};
    void output(int level);
};
//工具类，一个工具类中可能包含了多个stmt，暂时发现适用于语句之间的联系？
class SeqNode : public StmtNode
{
private:
    StmtNode *stmt1, *stmt2;
public:
    SeqNode(StmtNode *stmt1, StmtNode *stmt2) : stmt1(stmt1), stmt2(stmt2){};
    void output(int level);
};
//3.声明语句
class DeclStmt : public StmtNode
{
private:
    Id *id; //变量表达式结点
public:
    DeclStmt(Id *id) : id(id){};
    void output(int level);
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
};
//5.条件语句
class IfStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
public:
    IfStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
    void output(int level);
};
//6.条件语句
class IfElseStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
    StmtNode *elseStmt;
public:
    IfElseStmt(ExprNode *cond, StmtNode *thenStmt, StmtNode *elseStmt) : cond(cond), thenStmt(thenStmt), elseStmt(elseStmt) {};
    void output(int level);
};
//7.
class WhileStmt : public StmtNode { //新加3
   private:
    ExprNode* cond;
    StmtNode* stmt;

   public:
    WhileStmt(ExprNode* cond, StmtNode* stmt) : cond(cond), stmt(stmt){};
    void output(int level);
};
//8.
class BreakStmt : public StmtNode {
   public:
    BreakStmt(){};
    void output(int level);
};
//9.
class ContinueStmt : public StmtNode {
   public:
    ContinueStmt(){};
    void output(int level);
};
//10.
class ReturnStmt : public StmtNode
{
private:
    ExprNode *retValue; //可以为nullptr表示return;
public:
    ReturnStmt(ExprNode*retValue=nullptr) : retValue(retValue) {};
    void output(int level);
};

//11.函数声明语句
class FunctionDef : public StmtNode
{
private:
    SymbolEntry *se;
    //新增
    StmtNode* paramDecl;
    StmtNode *stmt;
    
public:
    FunctionDef(SymbolEntry *se, StmtNode* paramDecl, StmtNode *stmt) : se(se),paramDecl(paramDecl), stmt(stmt){};
    void output(int level);
};
//12.
class FunctionCall : public ExprNode
{
private:
    Node* paramGiven;
public:
    FunctionCall(SymbolEntry *se,Node* paramGiven) : ExprNode(se),
    paramGiven(paramGiven){};
    void output(int level);
};
//20.
class BlankStmt : public StmtNode { //新加StmtNode
   public:
    BlankStmt(){};
    void output(int level);
};



class ExprStmt : public StmtNode {  //新加StmtNode
   private:
    //符号表项是给函数名的
    ExprNode* se;

   public:
    ExprStmt(ExprNode* se) : se(se){};
    void output(int level);
};



class Ast
{
private:
    Node* root;
public:
    Ast() {root = nullptr;}
    void setRoot(Node*n) {root = n;}
    void output();
};

#endif
