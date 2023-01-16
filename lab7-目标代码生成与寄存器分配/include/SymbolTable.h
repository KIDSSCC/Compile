#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include <string>
#include <map>

class Type;
class Operand;

class SymbolEntry
{
private:
    int kind;
    SymbolEntry* next;  //函数重载，同名不同参返的才ok
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
    void setType(Type *type) {this->type = type;};
    virtual std::string toStr() = 0;
    // You can add any function you need here.
    bool setNext(SymbolEntry* se);
    SymbolEntry* getNext() const { return next; };
};


/*  
    Symbol entry for literal constant. Example:

    int a = 1;

    Compiler should create constant symbol entry for literal constant '1'.
*/
class ConstantSymbolEntry : public SymbolEntry
{
private:
    int value;
    float floatvalue;

public:
    ConstantSymbolEntry(Type *type, int value);
    ConstantSymbolEntry(Type *type, float floatvalue);
    ConstantSymbolEntry(Type* type): SymbolEntry(type, SymbolEntry::CONSTANT){};
    virtual ~ConstantSymbolEntry() {};
    int getValue() const {return value;};
    float getFloatValue() const {return floatvalue;};
    void setIntValue(int intvalue) {this->value = intvalue;}; 
    void setFloatValue(float floatvalue){this->floatvalue = floatvalue;};
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
    int label;  //新加
    Operand *addr;  // The address of the identifier.
    // You can add any field you need here.
    int intvalue;
    float floatvalue;
    //数组新加
    int* intarrayvalue;
    float* floatarrayvalue;
    bool quan0;
    //新加
    int AsParamNum;


public:
    IdentifierSymbolEntry(Type *type, std::string name, int scope,int AsParamNum=-1);//新加
    virtual ~IdentifierSymbolEntry() {};
    std::string toStr();
    bool isGlobal() const {return scope == GLOBAL;};
    bool isParam() const {return scope == PARAM;};
    bool isLocal() const {return scope >= LOCAL;};
    int getScope() const {return scope;};

    void setAddr(Operand *addr) {this->addr = addr;};
    Operand* getAddr() {return addr;};

    // You can add any function you need here.
    void setIntValue(int intvalue) {this->intvalue = intvalue;};    //改进：常量只能set一次，否则报错！
    int getIntValue() const { return intvalue; };
    void setFloatValue(float floatvalue){this->floatvalue = floatvalue;};
    float getFloatValue() const { return floatvalue; };
    std::string getName(){return name;};

    void setLabel(int label){this->label=label;}
    int getLabel(){return label;}
    //数组新加
    void setIntArrayValue(int* intarrayvalue) {this->intarrayvalue = intarrayvalue;};    
    int* getIntArrayValue() const { return intarrayvalue; };
    void setFloatArrayValue(float* floatarrayvalue){this->floatarrayvalue = floatarrayvalue;};
    float* getFloatArrayValue() const { return floatarrayvalue; };
    void setquan0() { quan0 = true; };
    bool ifquan0() const { return quan0; };
    //新加
    int getAsParamNum(){return AsParamNum;}
};


/* 
    Symbol entry for temporary variable created by compiler. Example:

    int a;
    a = 1 + 2 + 3;

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
    int stack_offset;   //lab7新加！！
    int label;
public:
    TemporarySymbolEntry(Type *type, int label);
    virtual ~TemporarySymbolEntry() {};
    std::string toStr();
    int getLabel() const {return label;};
    void setOffset(int offset) { this->stack_offset = offset; };
    int getOffset() { return this->stack_offset; };
    // You can add any function you need here.
};

// symbol table managing identifier symbol entries
class SymbolTable
{
private:
    std::map<std::string, SymbolEntry*> symbolTable;
    SymbolTable *prev;
    int level;
    static int counter;
public:
    SymbolTable();
    SymbolTable(SymbolTable *prev);
    void install(std::string name, SymbolEntry* entry);
    bool install1(std::string name, SymbolEntry* entry);//变量名重定义
    bool install2(std::string name, SymbolEntry* entry);//函数重载
    SymbolEntry* lookup(std::string name);
    SymbolTable* getPrev() {return prev;};
    int getLevel() {return level;};
    static int getLabel() {return counter++;};  //哪里会getLabel？创建临时变量时;bb的no设置时
    std::map<std::string, SymbolEntry*> getsymbolTable(){return symbolTable;}
};

extern SymbolTable *identifiers;
extern SymbolTable *globals;

#endif
