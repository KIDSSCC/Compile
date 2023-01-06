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

std::string FloatType::toStr()
{
    std::ostringstream buffer;
    buffer << "f" << size;
    return buffer.str();
}

std::string ArrayType::toStr()
{
    return "array";                         //完善
}



std::string VoidType::toStr()
{
    return "void";
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
