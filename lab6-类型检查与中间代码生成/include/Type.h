#ifndef __TYPE_H__
#define __TYPE_H__
#include <vector>
#include <string>
#include <SymbolTable.h>

class Type
{
protected:
    //第一版先不考虑数组和浮点，后续可以在这里补充对应类型
    enum {BOOL,INT, VOID, FUNC, PTR,FLOAT,ARRAY};    //lab6：增PTR
public:
    int kind;
    Type(int kind) : kind(kind) {};
    virtual ~Type() {};
    virtual std::string toStr() = 0;
    //暂时用不到指针，所以这里也也先不判断指针类型
    bool isInt() const {return kind == INT;};
    bool isVoid() const {return kind == VOID;};
    bool isFunc() const {return kind == FUNC;};
    bool isBool() const {return kind==BOOL;};
    bool isFloat() const {return kind == FLOAT;};
    bool isArray() const {return kind == ARRAY;};
    bool isPtr() const { return kind == PTR; };
    int getkind(){return kind;};
};

class IntType : public Type
{
private:
    int size;
    bool ifconst;
public:
    IntType(int size,bool ifconst=false) : Type(Type::INT), size(size),ifconst(ifconst){};
    std::string toStr();
    int getSize(){return size;};
};
class FloatType : public Type
{
private:
    int size;
    bool ifconst;
public:
    FloatType(int size, bool ifconst = false) : Type(Type::FLOAT), size(size), ifconst(ifconst){};
    std::string toStr();
};


class BoolType:public IntType
{
public:
    BoolType():IntType(1){kind=Type::BOOL;};
    std::string toStr();
}
;
class VoidType : public Type
{
public:
    VoidType() : Type(Type::VOID){};
    std::string toStr();
};

class FunctionType : public Type
{
private:
    Type *returnType;
    std::vector<Type*> paramsType;
    std::vector<SymbolEntry*> params;
public:
    FunctionType(Type* returnType, std::vector<Type*> paramsType) : 
    Type(Type::FUNC), returnType(returnType), paramsType(paramsType){};
    Type* getRetType() {return returnType;};
    std::string toStr();
    //新增
    void setparamsType(std::vector<Type*> paramsType){this->paramsType=paramsType;}
    std::vector<Type*> getparamsType(){return paramsType;} 
    void setparams(std::vector<SymbolEntry*> params){this->params=params;}
    std::vector<SymbolEntry*> getParams(){return params;}
};

class ArrayType : public Type
{
private:
    Type * artype;  //数组类型
    int num;        //数组元素个数
public:
    ArrayType( Type * artype,int num) : Type(Type::ARRAY), artype(artype), num(num){};
    std::string toStr();
};

class PointerType : public Type
{
private:
    Type *valueType;
public:
    PointerType(Type* valueType) : Type(Type::PTR) {this->valueType = valueType;};
    std::string toStr();
    Type* getType() const { return valueType; };
};

class TypeSystem
{
private:
    static IntType commonInt;
    static IntType commonIntConst;
    static BoolType commonBool;
    static VoidType commonVoid;
    static FloatType commonFloat;   
    static FloatType commonFloatConst;
public:
    static Type *intType;
    static Type *intconstType;
    static Type *voidType;
    static Type *boolType;
    static Type *floatType;
    static Type *floatconstType;
};

#endif
