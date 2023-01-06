#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include <string>
#include <map>

//class Type;
#include "Type.h"
#include <Ast.h>

class SymbolEntry
{
private:
    int kind;
protected:
    enum {CONSTANT, VARIABLE, TEMPORARY};
    Type *type;

public:
    SymbolEntry(Type *type, int kind);
    virtual ~SymbolEntry() {};
    bool isConstant() const {return kind == CONSTANT;};
    bool isTemporary() const {return kind == TEMPORARY;};
    bool isVariable() const {return kind == VARIABLE;};
    Type* getType() {return type;};
    virtual std::string toStr() = 0;
    // You can add any function you need here.
};


/*  
    Symbol entry for literal constant. Example:

    int a = 1;

    Compiler should create constant symbol entry for literal constant '1'.
*/
class ConstantSymbolEntry : public SymbolEntry
{
private:
    int intvalue;
    float floatvalue;
    //bool ifint;   不用了，直接用type

public:
    ConstantSymbolEntry(Type *type, int intvalue,float floatvalue);
    virtual ~ConstantSymbolEntry() {};
    int getIntValue() const {return intvalue;};
    float getFloatValue() const {return floatvalue;};
    std::string toStr();
    // You can add any function you need here.
};


/* 
    Symbol entry for identifier. Example:

    int a;
    int b;
    void f(int c)
    {
        int d;
        {
            int e;
        }
    }

    Compiler should create identifier symbol entries for variables a, b, c, d and e:

    | variable | scope    |
    | a        | GLOBAL   |
    | b        | GLOBAL   |
    | c        | PARAM    |
    | d        | LOCAL    |
    | e        | LOCAL +1 |
*/
class IdentifierSymbolEntry : public SymbolEntry
{
private:
    enum {GLOBAL, PARAM, LOCAL};
    std::string name;
    int scope;
    // You can add any field you need here.
    ExprNode* intvalue;
    ExprNode* floatvalue;

public:
    IdentifierSymbolEntry(Type *type, std::string name, int scope);
    virtual ~IdentifierSymbolEntry() {};
    std::string toStr();
    int getScope() const {return scope;};
    // You can add any function you need here.
    void setIntValue(ExprNode* intvalue) {this->intvalue = intvalue;};    //改进：常量只能set一次，否则报错！
    ExprNode* getIntValue() const { return intvalue; };
    void setFloatValue(ExprNode* floatvalue){this->floatvalue = floatvalue;};
    ExprNode* getFloatValue() const { return floatvalue; };
};


/* 
    Symbol entry for temporary variable created by compiler. Example:

    int a;
    a = 1 + 2 + 3;          //没有三值表达式，需要转换为二值表达式

    The compiler would generate intermediate code like:

    t1 = 1 + 2
    t2 = t1 + 3
    a = t2

    So compiler should create temporary symbol entries for t1 and t2:

    | temporary variable | label |
    | t1                 | 1     |
    | t2                 | 2     |
*/
class TemporarySymbolEntry : public SymbolEntry
{
private:
    int label;          //t1,t2,t3的1,2,3
public:
    TemporarySymbolEntry(Type *type, int label);    //临时变量的类型与标签
    virtual ~TemporarySymbolEntry() {};
    std::string toStr();
    // You can add any function you need here.
};

// symbol table managing identifier symbol entries
class SymbolTable
{
private:
    std::map<std::string, SymbolEntry*> symbolTable;    //某一作用域下的所有变量的符号表
    SymbolTable *prev;                                  //不同作用域下的符号表相连接
    int level;                                          //不同作用域等级
    static int counter;                                 //给临时变量的label计数
public:
    SymbolTable();
    SymbolTable(SymbolTable *prev);
    void install(std::string name, SymbolEntry* entry);
    SymbolEntry* lookup(std::string name);
    SymbolTable* getPrev() {return prev;};
    int getLevel() {return level;}; 
    static int getLabel() {return counter++;};  //哪里会getLabel？创建临时变量时
};

extern SymbolTable *identifiers;    //IdentifierSymbolEntry的scope用identifiers->getLevel来获得
extern SymbolTable *globals;

#endif
