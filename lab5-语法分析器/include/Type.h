#ifndef __TYPE_H__
#define __TYPE_H__
#include <vector>   //参数类型向量
#include <string>   //tostr()函数

class Type
{
private:
    int kind;
protected:
    enum {INT, VOID, FUNC,FLOAT,ARRAY};
public:
    Type(int kind) : kind(kind) {};
    virtual ~Type() {};
    virtual std::string toStr() = 0;
    bool isInt() const {return kind == INT;};
    bool isVoid() const {return kind == VOID;};
    bool isFunc() const {return kind == FUNC;};
    bool isFloat() const {return kind == FLOAT;};
    bool isArray() const {return kind == ARRAY;};
};

class IntType : public Type
{
private:
    int size;
    bool ifconst;
public:
    IntType(int size, bool ifconst = false) : Type(Type::INT), size(size), ifconst(ifconst){};
    std::string toStr();
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

class VoidType : public Type
{
public:
    VoidType() : Type(Type::VOID){};
    std::string toStr();
};

class FunctionType : public Type
{
private:
    Type *returnType;               //函数的返回值类型
    std::vector<Type*> paramsType; //函数的参数类型，参数数量不确定，因此使用动态数组
public:
    FunctionType(Type* returnType, std::vector<Type*> paramsType) : 
    Type(Type::FUNC), returnType(returnType), paramsType(paramsType){};
    std::string toStr();
    //新增
    void setparamsType(std::vector<Type*> paramsType){this->paramsType=paramsType;}
    std::vector<Type*> getparamsType(){return paramsType;}  
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

class TypeSystem        //类型系统，上述类型的类，只允许有一个实例
{
private:
    static IntType commonInt;       //全局
    static IntType commonIntConst;       //全局
    static VoidType commonVoid;     //全局
    static FloatType commonFloat;   //全局
    static FloatType commonFloatConst;   //全局
public:
    static Type *intType;           //是上面私有的地址（cpp说的）
    static Type *intconstType;      //疑惑为什么用Type指针而不是各自的类型类指针
    static Type *voidType;
    static Type *floatType;
    static Type *floatconstType;
};

#endif
