#include "SymbolTable.h"
#include <iostream>
#include <sstream>
#include "Type.h"//加

SymbolEntry::SymbolEntry(Type *type, int kind) 
{
    this->type = type;
    this->kind = kind;
}

ConstantSymbolEntry::ConstantSymbolEntry(Type *type, int value) : SymbolEntry(type, SymbolEntry::CONSTANT)
{
    if(type->isInt()){
        this->value = value;
    }
}
ConstantSymbolEntry::ConstantSymbolEntry(Type *type, float floatvalue) : SymbolEntry(type, SymbolEntry::CONSTANT)
{
    if(type->isFloat()){
        this->floatvalue = floatvalue;
    }   
}

std::string ConstantSymbolEntry::toStr()
{
    std::ostringstream buffer;
     if(type->isInt()){
        buffer << value;
    }
    else{
        buffer << floatvalue;
    }
    return buffer.str();
}

IdentifierSymbolEntry::IdentifierSymbolEntry(Type *type, std::string name, int scope) : SymbolEntry(type, SymbolEntry::VARIABLE), name(name)
{
    this->scope = scope;
    this->label=-1; //初始化新加
    addr = nullptr;
}

std::string IdentifierSymbolEntry::toStr()
{
    std::ostringstream buffer;
    if(label==-1)
    {
        buffer<<"@" << name;
    }
    else
    {
        buffer << "%t" << label;
    }
    return buffer.str();
}

TemporarySymbolEntry::TemporarySymbolEntry(Type *type, int label) : SymbolEntry(type, SymbolEntry::TEMPORARY)
{
    this->label = label;
}

std::string TemporarySymbolEntry::toStr()
{
    std::ostringstream buffer;
    buffer << "%t" << label;
    return buffer.str();
}

SymbolTable::SymbolTable()
{
    prev = nullptr;
    level = 0;
}

SymbolTable::SymbolTable(SymbolTable *prev)
{
    this->prev = prev;
    this->level = prev->level + 1;
}

/*
    Description: lookup the symbol entry of an identifier in the symbol table
    Parameters: 
        name: identifier name
    Return: pointer to the symbol entry of the identifier

    hint:
    1. The symbol table is a stack. The top of the stack contains symbol entries in the current scope.
    2. Search the entry in the current symbol table at first.
    3. If it's not in the current table, search it in previous ones(along the 'prev' link).
    4. If you find the entry, return it.
    5. If you can't find it in all symbol tables, return nullptr.
*/
SymbolEntry* SymbolTable::lookup(std::string name)
{
    // Todo
    SymbolTable* now=this;
    while(now!=nullptr)
    {
        //通过map容器的find函数找到了对应的表项
        if(now->symbolTable.find(name) != now->symbolTable.end())
        {
            return now->symbolTable[name];
        }
        else
        {
            //当前符号表中没找到，转移阵地
            now=now->prev;
        }
    }
    return nullptr;
}

// install the entry into current symbol table.
void SymbolTable::install(std::string name, SymbolEntry* entry)
{
    symbolTable[name] = entry;
}
//变量重复定义
bool SymbolTable::install1(std::string name, SymbolEntry* entry) {
    //检查是否重定义
    if (this->symbolTable.find(name) != this->symbolTable.end()) {
       
        return false;
    } else {
        symbolTable[name] = entry;
        return true;
    }
}
//函数重复定义（函数重载）缺点：只是根据个数不同就ok，其实即使个数相同，也不一定false，应继续排查每个参数类型以及返回值类型最终给定ok与否
bool SymbolEntry::setNext(SymbolEntry* se) {    //但lab6/04的函数重载，刚好用个数一致与否可以判定
    SymbolEntry* s = this;
    int num =((FunctionType*)(se->getType()))->getparamsType().size();//新声明函数的参数个数
    //fprintf(stderr,"%d,%d\n",num,((FunctionType*)(s->getType()))->getparamsType().size());
    if (num == (int)((FunctionType*)(s->getType()))->getparamsType().size())//旧函数的参数个数
    {
        return false;
    }   
    while (s->getNext()) {
        if (num == (int)((FunctionType*)(s->getType()))->getparamsType().size()){
            return false;
        }
            
        s = s->getNext();
    }//新来的与之前同名函数（内部各异）一一排查
    //排查完没有在个数上一致的（同名函数之间参数个数均不一样），可以插入
    if (s == this) {
        this->next = se;
    } else {
        s->setNext(se);
    }
    return true;
}
bool SymbolTable::install2(std::string name, SymbolEntry* entry) {
    //检查是否重定义
    if (this->symbolTable.find(name) != this->symbolTable.end()) {
        SymbolEntry* se = this->symbolTable[name];
        if (se->getType()->isFunc())
            return se->setNext(entry);
        return false;
    } else {
        symbolTable[name] = entry;
        return true;
    }
}

int SymbolTable::counter = 0;
static SymbolTable t;
SymbolTable *identifiers = &t;
SymbolTable *globals = &t;
