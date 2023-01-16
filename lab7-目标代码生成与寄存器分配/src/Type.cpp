#include "Type.h"
#include <sstream>

IntType TypeSystem::commonInt = IntType(32);
IntType TypeSystem::commonIntConst = IntType(32,true);
BoolType TypeSystem::commonBool = BoolType();
VoidType TypeSystem::commonVoid = VoidType();
FloatType TypeSystem::commonFloat = FloatType(32);   //float也是4字节
FloatType TypeSystem::commonFloatConst = FloatType(32,true);

Type* TypeSystem::intType = &commonInt;
Type* TypeSystem::intconstType = &commonIntConst;
Type* TypeSystem::voidType = &commonVoid;
Type* TypeSystem::boolType = &commonBool;
Type* TypeSystem::floatType = &commonFloat;
Type* TypeSystem::floatconstType = &commonFloatConst;

int Type::getOSize()  
{ 
    if(this->isInt()) return ((IntType *)this)->getSize();
    else if(this->isFloat())  return ((FloatType *)this)->getSize();
    else if(this->isArray())  return ((ArrayType *)this)->getArraySize();
    else if(this->isBool())  return 1;
    else if(this->isVoid())  return 0;
    else if(this->isPtr())  return 4;
    else return 0; //函数类型咋算？
}

std::string IntType::toStr()
{
    std::ostringstream buffer;
    buffer << "i" << size;
    return buffer.str();
}

std::string BoolType::toStr()
{
    std::ostringstream buffer;
    buffer<<"i1";
    return buffer.str();
}

std::string VoidType::toStr()
{
    return "void";
}

std::string FloatType::toStr()
{
    std::ostringstream buffer;
    buffer << "float";//ll文件不是f32！！
    return buffer.str();
}
std::string ArrayType::toStr()  //a[10][9]:[10 x [9 x i32]]
{
    Type* temp = this;//数组每一维度遍历
    int count = 0;//记录[]对数
    std::ostringstream buffer;
    bool flag = false;//数组debug
    while (temp && temp->isArray()) {
        if (((ArrayType*)temp)->getArrayNum() == 0) {//数组debug
            flag = true;
        }
        else{
            buffer << "[" << ((ArrayType*)temp)->getArrayNum() << " x ";//输出“[10 x ”、“[9 x ”,counter=2
            count++; 
        }
         
        temp = ((ArrayType*)temp)->getArrayType();
    }
    if(temp->isInt()) buffer << "i32";//输出“[10 x [9 x i32”
    else buffer << "float";//输出“[10 x [9 x f32”
    
    while (count--)
        buffer << ']';//输出“[10 x [9 x i32]]” 
    if (flag)//数组debug
        buffer << '*';
    return buffer.str();  
}
std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    buffer << returnType->toStr() << "(";
    for(int i=0;i<(int)paramsType.size();i++)
    {
        buffer<< paramsType[i]->toStr();
        if(i!=(int)paramsType.size()-1)
        {
            buffer<<",";
        }
    }
    buffer<< ")";
    return buffer.str();
}

std::string PointerType::toStr()
{
    std::ostringstream buffer;
    buffer << valueType->toStr() << "*";
    return buffer.str();
}
