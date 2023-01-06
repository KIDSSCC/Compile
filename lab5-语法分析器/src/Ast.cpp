#include "Ast.h"
#include "SymbolTable.h"
#include <string>
#include "Type.h"

extern FILE *yyout;
int Node::counter = 0;

Node::Node()
{
    seq = counter++;
    next = nullptr;
}

void Node::setNext(Node* node) {   //通过next指针将不同的节点串联起来，暂时只用于变量批量声明时，在同一级打印出各个声明语句
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

void Ast::output()
{
    fprintf(yyout, "program\n");
    if(root != nullptr)
        root->output(4);        //level=0；0+4 root是node类，打印什么要看具体是啥类型结点
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
        case AND:
            op_str = "and";
            break;
        case OR:
            op_str = "or";
            break;
        case LT:
            op_str = "lt";
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

void Constant::output(int level)
{
    std::string type, value;
    type = symbolEntry->getType()->toStr();
    value = symbolEntry->toStr();
    if(type=="int")
    {
        fprintf(yyout, "%*cIntegerLiteral\tvalue: %s\ttype: %s\n", level, ' ',
            value.c_str(), type.c_str());
    }
    if(type=="float")
    {
        fprintf(yyout, "%*cfloatLiteral\tvalue: %s\ttype: %s\n", level, ' ',
            value.c_str(), type.c_str());
    }
    
}

void Id::output(int level)
{
    std::string name, type;
    ExprNode* intValue;
    ExprNode* floatValue;
    int scope;
    name = symbolEntry->toStr();
    type = symbolEntry->getType()->toStr();
    
    //dynamic_cast 类型转换，将宽泛的symbolEntry转换为针对性的IdentifierSymbolEntry
    scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
    fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\n", level, ' ',
            name.c_str(), scope, type.c_str());
    if((type=="const int")||(type=="int"))
    {
        /*
            错误写法：intValue=dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getIntValue();
                    intValue->output(level + 4);   
        */
        
        intValue=dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getIntValue();
        if(intValue)                        //进不去？？
            intValue->output(level + 4);
    }
    else if((type=="const float")||(type=="float"))
    {
        floatValue=dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getFloatValue();
        if(floatValue){
            floatValue->output(level + 4);
        }
    }
}

void CompoundStmt::output(int level)
{
    fprintf(yyout, "%*cCompoundStmt\n", level, ' ');
    if(stmt!=nullptr)
        stmt->output(level + 4);
}

void SeqNode::output(int level)
{
    fprintf(yyout, "%*cSequence\n", level, ' ');
    stmt1->output(level + 4);
    stmt2->output(level + 4);
}

void DeclStmt::output(int level)
{
    fprintf(yyout, "%*cDeclStmt\n", level, ' ');
    id->output(level + 4);
    //打印自己，同时看看还有没有其他的声明，int a,b,c这种的
    if (this->getNext()) {
        this->getNext()->output(level);
    }
}

void AssignStmt::output(int level)
{   //赋值语句的打印，在本行打印出AssignStmt，下两行分别表明左值和右表达式
    fprintf(yyout, "%*cAssignStmt\n", level, ' ');
    lval->output(level + 4);
    expr->output(level + 4);
}

void IfStmt::output(int level)
{
    fprintf(yyout, "%*cIfStmt\n", level, ' ');
    cond->output(level + 4);
    if(thenStmt){               //tip
        thenStmt->output(level + 4);
    }
    
}

void IfElseStmt::output(int level)
{
    fprintf(yyout, "%*cIfElseStmt\n", level, ' ');
    cond->output(level + 4);
     if(thenStmt){               //tip
        thenStmt->output(level + 4);
    }
    if(elseStmt){                //tip
        elseStmt->output(level + 4);
    }
    
}

void WhileStmt::output(int level) {     //新加3
    fprintf(yyout, "%*cWhileStmt\n", level, ' ');
    cond->output(level + 4);
    if(stmt){
        stmt->output(level + 4);
    }
}
void BreakStmt::output(int level) {
    fprintf(yyout, "%*cBreakStmt\n", level, ' ');
}

void ContinueStmt::output(int level) {
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
       



void FunctionDef::output(int level)
{
    std::string name, type;
    name = se->toStr();
    type = se->getType()->toStr();//getType拿到了一个Type*的指针，实质上是FuncType，toStr得返回值类型
    //获取参数的类型
    std::vector<Type*> paramsType= dynamic_cast<FunctionType*>(se->getType())->getparamsType();
    std::string paramTypeString="";
    for(long unsigned int i=0;i<paramsType.size();i++)
    {
        paramTypeString=paramTypeString+paramsType[i]->toStr()+" ";
    }
    fprintf(yyout, "%*cFunctionDefine function name: %s, type: %s%s)\n", level, ' ', 
            name.c_str(), type.c_str(),paramTypeString.c_str());        //闭合括号
    stmt->output(level + 4);
}


void FunctionCall::output(int level)
{
    std::string name;
    name = symbolEntry->toStr();
    fprintf(yyout, "%*cFunctionCall function name: %s\n", level, ' ',
    name.c_str());
    if(paramGiven)
    {
        Node* tmp=paramGiven;
        tmp->output(level+4);
        while(tmp->getNext())
        {
            tmp->getNext()->output(level+4);
            tmp=tmp->getNext();
        }
    }
}

void BlankStmt::output(int level) {     //新加
    fprintf(yyout, "%*cBlankStmt\n", level, ' ');
}



void ExprStmt::output(int level)
{
    fprintf(yyout, "%*cExprStmt\n", level, ' ');

    se->output(level+4);
}
