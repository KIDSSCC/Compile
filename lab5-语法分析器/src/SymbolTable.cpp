#include "SymbolTable.h"
#include <iostream>
#include <sstream>

//#include "Type.h"

SymbolEntry::SymbolEntry(Type *type, int kind) 
{
    this->type = type;
    this->kind = kind;
}

ConstantSymbolEntry::ConstantSymbolEntry(Type *type, int intvalue,float floatvalue) : SymbolEntry(type, SymbolEntry::CONSTANT)
{
    if(type->isInt()){
        this->intvalue = intvalue;
    }
    else{
         this->floatvalue = floatvalue;
    }
}

std::string ConstantSymbolEntry::toStr()
{
    std::ostringstream buffer;
    if(type->isInt()){
        buffer << intvalue;
    }
    else{
        buffer << floatvalue;
    }
    return buffer.str();
}

IdentifierSymbolEntry::IdentifierSymbolEntry(Type *type, std::string name, int scope) : SymbolEntry(type, SymbolEntry::VARIABLE), name(name)
{
    this->scope = scope;
}

std::string IdentifierSymbolEntry::toStr()
{
    return name;        //要不要返回值捏,不用就打印个变量名
}

TemporarySymbolEntry::TemporarySymbolEntry(Type *type, int label) : SymbolEntry(type, SymbolEntry::TEMPORARY)
{
    this->label = label;
}

std::string TemporarySymbolEntry::toStr()
{
    std::ostringstream buffer;
    buffer << "t" << label;     //ostringstream完成字符串拼接
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
    // Todo 在同一个作用域的符号表里，不可能有同名变量；找着就是找着，没找着就去上一个表找
    /*std::map<std::string, SymbolEntry*>::iterator it;   
    for (it = symbolTable.begin(); it != symbolTable.end(); ++it) {
        if (it->first == name) {
            return it->second;
        }
    }
    //这个符号表里没有
    //如果这个符号表就是最头表，没治了
    if (this->level == 0) {
        return nullptr;
    }
    //否则它还有前一个
    SymbolTable* pst = this->prev;
    return pst->lookup(name);*/
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
    symbolTable[name] = entry;      //这是SymbolTable里的map结构symbolTable
}

int SymbolTable::counter = 0;
static SymbolTable t;
SymbolTable *identifiers = &t;
SymbolTable *globals = &t;      //为啥都指t，咋区分？ 因为初始状态大家都是全局变量，在遇到{}后创建一个新的identifiers，同时通过前向指针指向
                                //原来的identifiers，这样在任意时刻identifiers都代表了当前的作用域，globals始终是全局作用域
