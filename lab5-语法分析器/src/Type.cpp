#include "Type.h"
#include <sstream>      //ostringstream

//给类型系统的成员变量们赋个值
IntType TypeSystem::commonInt = IntType(4);     //new一个公共全局int类型，大小为4
IntType TypeSystem::commonIntConst = IntType(4,true);
VoidType TypeSystem::commonVoid = VoidType();
FloatType TypeSystem::commonFloat = FloatType(4);   //float也是4字节
FloatType TypeSystem::commonFloatConst = FloatType(4,true);

Type* TypeSystem::intType = &commonInt;
Type* TypeSystem::intconstType = &commonIntConst;
Type* TypeSystem::voidType = &commonVoid;
Type* TypeSystem::floatType = &commonFloat;
Type* TypeSystem::floatconstType = &commonFloatConst;

std::string IntType::toStr()
{
    if (ifconst)
        return "const int";
    return "int";
}

std::string VoidType::toStr()
{
    return "void";
}

std::string FloatType::toStr()
{
    if (ifconst)
        return "const float";
    return "float";
}

std::string ArrayType::toStr()
{
    return "array";                         //完善
}

std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    buffer << returnType->toStr() << " (";  //完善，打印参数
    return buffer.str();
}
