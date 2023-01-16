%code top{
    #include <iostream>
    #include <assert.h>
    #include "parser.h"
    #include<vector>    //加
    extern Ast ast;
    int yylex();
    int yyerror( char const * );
    std::string tempType;
}

%code requires {
    #include "Ast.h"
    #include "SymbolTable.h"
    #include "Type.h"
}

%union {
    int itype;
    float ftype;        //加
    char* strtype;
    StmtNode* stmttype;
    ExprNode* exprtype;
    Type* type;
    SymbolEntry* se;    //加
}

%start Program
%token <strtype> ID 
%token <itype> INTEGER
%token <ftype> FLOATNUMBER    //加
%token IF ELSE  WHILE
%token INT VOID FLOAT
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON    COMM LBRACK RBRACK
%token ADD SUB MUL DIV MOD OR AND NOT LESS GREAT EQ NE LE GE ASSIGN
%token CONST
%token RETURN CONTINUE BREAK

%nterm <stmttype> Stmts Stmt AssignStmt BlockStmt IfStmt ReturnStmt DeclStmt FuncDef WhileStmt BrkStmt CtnueStmt ConstDecl ConstDefList ConstDef VarDecl VarDefList VarDef FuncParams FuncParam  BlankStmt ExprStmt //多最后一个
%nterm <exprtype> Exp AddExp Cond LOrExp PrimaryExp LVal RelExp LAndExp  UnaryExp MulExp EqExp   FuncCall ParaList  //FuncRParams ExprList
%nterm <type> Type

%precedence THEN    //优先级else更高
%precedence ELSE
%%
Program
    : Stmts {
        ast.setRoot($1);
    }
    ;
Stmts
    : Stmt {$$=$1;}
    | Stmts Stmt{
        $$ = new SeqNode($1, $2);
    }
    ;
Stmt
    : AssignStmt {$$=$1;}
    | BlockStmt {$$=$1;}
    | IfStmt {$$=$1;}
    | ReturnStmt {$$=$1;}
    | DeclStmt {$$=$1;}
    | FuncDef {$$=$1;}
    | WhileStmt {$$=$1;}
    | BrkStmt {$$=$1;}
    | CtnueStmt {$$=$1;}
    | BlankStmt {$$=$1;}    //加
    | ExprStmt {$$=$1;}     //加
    ;
//加 
DeclStmt
    : ConstDecl SEMICOLON {$$=$1;}
    | VarDecl SEMICOLON {$$=$1;}
    ;
ConstDecl
    : CONST Type ConstDefList {$$ = $3;}
    ;
Type
    : INT {
        //标注一下遇到了这个类型，在变量常量声明时会用到，float同理
        tempType="INT";
        $$ = TypeSystem::intType;
    }
    | FLOAT {
        tempType="FLOAT";
        $$ = TypeSystem::floatType;
    }
    | VOID {
        $$ = TypeSystem::voidType;
    }
    ;
ConstDefList
    : ConstDefList COMM ConstDef {
        $$ = $1;
        $1->setNext($3);    //通过next指针将不同的节点串联起来，暂时只用于变量批量声明时，在同一级打印出各个声明语句
    }
    | ConstDef {$$ = $1;}
    ;
ConstDef
    : ID ASSIGN Exp {
        //创建一个符号表项
        SymbolEntry *se;
        if(tempType=="INT")
        {
            //IdentifierSymbolEntry(Type *type, std::string name, int scope);
            //给定类型：整型常量 变量名，作用域，将初值表达式给到intValue 刚开始先从简单的起步，后续想在这一阶段直接算出表达式的值
            se = new IdentifierSymbolEntry(TypeSystem::intconstType, $1, identifiers->getLevel());
            ((IdentifierSymbolEntry*)se)->setIntValue($3);
        }
        if(tempType=="FLOAT")
        {
            se = new IdentifierSymbolEntry(TypeSystem::floatconstType, $1, identifiers->getLevel());
            ((IdentifierSymbolEntry*)se)->setFloatValue($3);
        }
        //将这一符号表项存入当前作用域的符号表中
            identifiers->install($1, se);
            //创建这一个节点
            //ConstDef拿到了一个DeclStmt的属性
            $$ = new DeclStmt(new Id(se));
            delete []$1;
    }
    ;



VarDecl
    : Type VarDefList {$$ = $2;}
    ;
VarDefList 
    : VarDefList COMM VarDef {
        $$ = $1;
        $1->setNext($3);
    }
    | VarDef { $$ = $1;}
    ;
VarDef
    : ID {
        //创建一个符号表项
        SymbolEntry *se;
        if(tempType=="INT")
        {
            //IdentifierSymbolEntry(Type *type, std::string name, int scope);
            //给定类型：整型常量 变量名，作用域，将初值表达式给到intValue 刚开始先从简单的起步，后续想在这一阶段直接算出表达式的值，shm学长仓库有，
            se = new IdentifierSymbolEntry(TypeSystem::intType, $1, identifiers->getLevel());
        }
        if(tempType=="FLOAT")
        {
            se = new IdentifierSymbolEntry(TypeSystem::floatType, $1, identifiers->getLevel());
        }
        //将这一符号表项存入当前作用域的符号表中
            identifiers->install($1, se);
            //创建这一个节点
            //VarDef拿到了一个DeclStmt的属性
            $$ = new DeclStmt(new Id(se));
            delete []$1;
    }
    | ID ASSIGN Exp {
        //创建一个符号表项
        SymbolEntry *se;
        if(tempType=="INT")
        {
            //IdentifierSymbolEntry(Type *type, std::string name, int scope);
            //给定类型：整型常量 变量名，作用域，将初值表达式给到intValue 刚开始先从简单的起步，后续想在这一阶段直接算出表达式的值，shm学长仓库有，
            se = new IdentifierSymbolEntry(TypeSystem::intType, $1, identifiers->getLevel());
            ((IdentifierSymbolEntry*)se)->setIntValue($3);
        }
        if(tempType=="FLOAT")
        {
            se = new IdentifierSymbolEntry(TypeSystem::floatType, $1, identifiers->getLevel());
            ((IdentifierSymbolEntry*)se)->setFloatValue($3);
        }
        //将这一符号表项存入当前作用域的符号表中
            identifiers->install($1, se);
            //创建这一个节点
            //ConstDef拿到了一个DeclStmt的属性
            $$ = new DeclStmt(new Id(se));
            delete []$1;
    }
    ;

LVal
    : ID {  //数组还需完善！
        //从当前的作用域开始找这个变量名是否存在，一直找到全局，遍历的时候是根据作用域的逆序
        SymbolEntry *se;
        se = identifiers->lookup($1);   //name字符串<strtype> ID ；SymbolTable *identifiers;
        if(se == nullptr)
        {    //一直到全局符号表都没有找到这个变量，说明没有声明过
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);//一条断言，意义...不大？如果这个条件不成立的话，程序就会在这里终止
        }
        //找到了对应的符号表项
        //创建了新的ID节点
        $$ = new Id(se);    //class Id : public ExprNode
        delete []$1;
    }
    ;
AssignStmt
    :
    LVal ASSIGN Exp SEMICOLON {
        //赋值语句，输出要求：我要给谁，赋什么值 
        //不用具体去存储？
        $$ = new AssignStmt($1, $3);
    }
    ;
BlockStmt
    :   LBRACE  
        {identifiers = new SymbolTable(identifiers);} //创建了新的作用域，更新identifiers
        Stmts RBRACE 
        {
            $$ = new CompoundStmt($3);  //遇到右括号，建一个CompoundStmt
            SymbolTable *top = identifiers;
            identifiers = identifiers->getPrev();   //更新identifiers
            delete top;      //使当前作用域失效
        }
    |   LBRACE RBRACE{
        $$=new CompoundStmt(nullptr);
    } 
    ;
BlankStmt
    : SEMICOLON {
        $$ = new BlankStmt();
    }
    ;
IfStmt
    : IF LPAREN Cond RPAREN Stmt %prec THEN {
        $$ = new IfStmt($3, $5);
    }
    | IF LPAREN Cond RPAREN Stmt ELSE Stmt {
        $$ = new IfElseStmt($3, $5, $7);
    }
    ;
ReturnStmt
    :
    RETURN Exp SEMICOLON{
        $$ = new ReturnStmt($2);
    }
    | RETURN SEMICOLON {
        $$ = new ReturnStmt();
    }   //加
    ;
//加4个
WhileStmt
    : WHILE LPAREN Cond RPAREN Stmt {
        $$ = new WhileStmt($3, $5);
    }
    ;
BrkStmt
    : BREAK SEMICOLON {
        $$ = new BreakStmt();
    }
    ;
CtnueStmt
    : CONTINUE SEMICOLON {
        $$ = new ContinueStmt();
    }
    ;
ExprStmt
    : Exp SEMICOLON {
        $$ = new ExprStmt($1);
    }
    ;  
Exp
    :AddExp {$$ = $1;}
   
    ;
Cond
    :
    LOrExp {$$ = $1;}
    ;
PrimaryExp
    :LPAREN Exp RPAREN {
        $$ = $2;
    }
    | LVal {
        $$ = $1;
    }
    | INTEGER {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::intType, $1,0.0);
        $$ = new Constant(se);
    } 
    | FLOATNUMBER {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::floatType, 0,$1);
        $$ = new Constant(se);
    }
    | FuncCall {$$=$1;  }     //加(从exp那里移到这里)
    ;

UnaryExp 
    : PrimaryExp {$$ = $1;}
    | ADD UnaryExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se, UnaryExpr::ADD, $2);
    }
    | SUB UnaryExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se, UnaryExpr::SUB, $2);
    }
    | NOT UnaryExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se, UnaryExpr::NOT, $2);
    }
    ;

//新加4个
MulExp
    : UnaryExp {$$ = $1;}
    | MulExp MUL UnaryExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MUL, $1, $3);
    }
    | MulExp DIV UnaryExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::DIV, $1, $3);
    }
    | MulExp MOD UnaryExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MOD, $1, $3);
    }
    ;

AddExp
    :
    MulExp {$$ = $1;}
    |
    AddExp ADD MulExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::ADD, $1, $3);
    }
    |
    AddExp SUB MulExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::SUB, $1, $3);
    }
    ;
RelExp
    : Exp {$$ = $1;}
    | RelExp LESS AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LT, $1, $3);
    }
    | RelExp GREAT AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GT, $1, $3);
    }
    | RelExp LE AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LE, $1, $3);
    }
    | RelExp GE AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GE, $1, $3);
    }
    ;

EqExp
    : RelExp {$$ = $1;}
    | EqExp EQ RelExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::EQ, $1, $3);
    }
    | EqExp NE RelExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::NE, $1, $3);
    }
    ;

LAndExp
    :
    EqExp {$$ = $1;}
    |
    LAndExp AND EqExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::AND, $1, $3);
    }
    ;
LOrExp
    :
    LAndExp {$$ = $1;}
    |
    LOrExp OR LAndExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::OR, $1, $3);
    }
    ;


FuncDef     //无参的，改成有参形式
    :
    Type ID {
       //声明一个函数类型，函数类型需要存储返回值的类型，同时用动态数组存储各参数的类型。此处参数类型先置空!
        Type *funcType;
        funcType = new FunctionType($1,{}); //{}时置空参数
        //针对于函数名这一层面，新建符号表项，（只是一个变量名的符号表，怪怪的，先记着）
        SymbolEntry *se = new IdentifierSymbolEntry(funcType, $2, identifiers->getLevel());
        //接入当前的符号表，并新建作用域！
        identifiers->install($2, se);
        identifiers = new SymbolTable(identifiers);
    }
    LPAREN FuncParams RPAREN
    {
        //找到函数名所对应的符号表项,将FuncParams开始的所有参数的类型存储到符号表项类型所对应的动态数组中，
        if($5)
        {
            SymbolEntry *se;
            se = identifiers->lookup($2);
            FunctionType* functype=dynamic_cast<FunctionType*>(se->getType());
            std::vector<Type*> paramType;
            //dynamic_cast<IdentifierSymbolEntry*>($5->getID()->symbolEntry)
        
            DeclStmt* tmp=dynamic_cast<DeclStmt*>($5);
            paramType.push_back(dynamic_cast<IdentifierSymbolEntry*>(tmp->getID()->getSymbolEntry())->getType());
            while(tmp->getNext())
            {
                SymbolEntry*tmpsymbol=dynamic_cast<DeclStmt*>(tmp->getNext())->getID()->getSymbolEntry();
                paramType.push_back(dynamic_cast<IdentifierSymbolEntry*>(tmpsymbol)->getType());
                tmp=dynamic_cast<DeclStmt*>(tmp->getNext());
            }
            functype->setparamsType(paramType);
        }
        
    }
    BlockStmt
    {
        SymbolEntry *se;
        se = identifiers->lookup($2);
        assert(se != nullptr);
        $$ = new FunctionDef(se,$5, $8);

        //恢复原来的作用域，和函数本身关系不大
        SymbolTable *top = identifiers;
        identifiers = identifiers->getPrev();
        delete top;
        delete []$2;
    }
    ;

FuncParams
    : FuncParams COMM FuncParam {
        $$ = $1;
        $1->setNext($3);        //设置参数的next
    }
    | FuncParam {
        $$ = $1;
    }
    | %empty { $$=nullptr;}     //epsilon
    ;
FuncParam
    : Type ID {
        //创建一个符号表项
        SymbolEntry *se;
        if(tempType=="INT")
        {
            //IdentifierSymbolEntry(Type *type, std::string name, int scope);
            //给定类型：整型常量 变量名，作用域，将初值表达式给到intValue 刚开始先从简单的起步，后续想在这一阶段直接算出表达式的值，shm学长仓库有，
            se = new IdentifierSymbolEntry(TypeSystem::intType, $2, identifiers->getLevel());
        }
        if(tempType=="FLOAT")
        {
            se = new IdentifierSymbolEntry(TypeSystem::floatType, $2, identifiers->getLevel());
        }
        //将这一符号表项存入当前作用域的符号表中
            identifiers->install($2, se);
            //创建这一个节点
            //VarDef拿到了一个DeclStmt的属性
            $$ = new DeclStmt(new Id(se));
            delete []$2;
            
    }
    ;
FuncCall
    : ID LPAREN ParaList RPAREN {
        //要做的是，新建一个函数调用节点，需要包含的信息是函数名的符号表项，以及各个参数的符号表项
        //查找函数名对应的符号表项
        SymbolEntry *se;
        se = identifiers->lookup($1);
        $$=new FunctionCall(se,$3);
    }
    ;
ParaList
    : ParaList COMM Exp {
        $$ = $1;
        $1->setNext($3);        //设置next
    }
    | Exp { $$=$1;}
    | %empty { $$=nullptr;}
    ;

%%

int yyerror(char const* message)
{
    std::cerr<<message<<std::endl;
    return -1;
}