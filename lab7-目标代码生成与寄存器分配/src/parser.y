%code top{
    #include <iostream>
    #include <cstring>//memset
    #include <assert.h>
    #include "parser.h"
    extern Ast ast;
    int yylex();
    int yyerror( char const * );

    std::string tempType;//记录int还是float
    //为while循环中的break和continue做的准备。
    std::vector<int> newWhileStmtPos;
    std::vector<loopctl*> loopctrl;
    int whilenum = 0;//记录while句子个数，如果为0的时候遇见了break或continue要报错
    bool ifFuncDefStmtIsNull=0;
    //数组新加
    ArrayType* arraytype;//数组类型，可能包含嵌套
    int myindex;//数组索引
    int* equalintarray;//存储进的等价一维int数组，重点计算size
    float* equalfloatarray;
    std::stack<Initlist*> initstack;
    Initlist* top;
    int bracenum= 0;
    //m新加
    int Param_num_for_func=0;
    bool functionhasRet=false;
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
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMICOLON COMM
%token ADD SUB MUL DIV MOD OR AND NOT LESS GREAT EQ NE LE GE ASSIGN
%token CONST
%token RETURN CONTINUE BREAK

%nterm <stmttype> Stmts Stmt AssignStmt BlockStmt IfStmt ReturnStmt DeclStmt FuncDef 
%nterm <stmttype> WhileStmt BrkStmt CtnueStmt ConstDecl ConstDefList ConstDef VarDecl VarDefList VarDef FuncParams FuncParam  BlankStmt ExprStmt
%nterm <exprtype> Exp AddExp Cond LOrExp PrimaryExp LVal RelExp LAndExp UnaryExp MulExp EqExp FuncCall ParaList ArrayIndex ConstExp ConstInitVal VarInitVal ConstInitValList VarInitValList FunArrayIndex
%nterm <type> Type

%precedence THEN
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
    : AssignStmt { $$=$1;}
    | BlockStmt {$$=$1;}
    | IfStmt {$$=$1;}
    | ReturnStmt {$$=$1;}
    | DeclStmt {$$=$1; }
    | FuncDef {$$=$1;}
    | WhileStmt {$$=$1;}
    | BrkStmt {$$=$1;}
    | CtnueStmt {$$=$1;}
    | BlankStmt {$$=$1;}    
    | ExprStmt {$$=$1;}  
    ;
LVal
    : ID {  //数组还需完善！
        //从当前的作用域开始找这个变量名是否存在，一直找到全局，遍历的时候是根据作用域的逆序
        SymbolEntry *se;
        se = identifiers->lookup($1);
        if(se == nullptr)
        {
            fprintf(stderr, "identifier %s is undeclared\n", (char*)$1);
            delete [](char*)$1;
            //assert(se != nullptr);
            exit(EXIT_FAILURE); //报错退出
        }
        //找到了对应的符号表项
        //创建了新的ID节点
        $$ = new Id(se);
        delete []$1;
    }
    | ID ArrayIndex{
        SymbolEntry* se;
        se = identifiers->lookup($1);
        if(se == nullptr)
        {
            fprintf(stderr, "identifier %s is undeclared\n", (char*)$1);
            delete [](char*)$1;
            //assert(se != nullptr);
            exit(EXIT_FAILURE); //报错退出
        }
        //找到了对应的符号表项
        //创建了新的ID节点
        $$ = new Id(se, $2);
        delete []$1;
    }
    ;
AssignStmt
    :
    LVal ASSIGN Exp SEMICOLON {
        
        //检查左值ID和Exp的类型一样不   //目前不一样还未考虑类型转换，只要不一样就报错  //bug解决：a=getInt(),左边intType，右边funcType，应该取函数类型的返回值类型！
        SymbolEntry *se =$1-> getSymPtr();
        Type * exptype;
        if($3->getSymPtr()->getType()->isFunc() ){
            exptype = ((FunctionType *)($3->getSymPtr()->getType()))-> getRetType();   
        } 
        else if ($3->getSymPtr()->getType()->isArray() ){
            exptype = ((ArrayType  *)($3->getSymPtr()->getType()))-> getFinalType();   
        } 
        else exptype =$3->getSymPtr()->getType();

        Type * lvaltype;
        if(((IdentifierSymbolEntry*)se)->getType()->isArray() ){
            lvaltype = ((ArrayType *)(((IdentifierSymbolEntry*)se)->getType()))-> getFinalType();//多维还需完善
            
        } 
        else lvaltype=((IdentifierSymbolEntry*)se)->getType();
        if(lvaltype == TypeSystem::intconstType){
            lvaltype=TypeSystem::intType;
        }
        if(exptype== TypeSystem::intconstType){
            exptype=TypeSystem::intType;
        }
        if(lvaltype == TypeSystem::floatconstType){
            lvaltype=TypeSystem::floatType;
        }
        if(exptype == TypeSystem::floatconstType){
           exptype=TypeSystem::floatType;
        }
        //1.移到放过之前
        if((lvaltype==TypeSystem::intType) && (exptype==TypeSystem::floatType) ){
            fprintf(stderr,"int = float !\n");
            //类型转化
            $$ = new AssignStmt($1, new FloatToInt($3,($3->getFloatValue()) ));
        }
        else if((lvaltype==TypeSystem::floatType) && (exptype==TypeSystem::intType) ){
            fprintf(stderr,"float = int !\n");
            //类型转化
            $$ = new AssignStmt($1, new IntToFloat($3,($3->getValue()) ));
        }
        
        else $$ = new AssignStmt($1, $3);

        //2.考虑类型转换,放过
        if((lvaltype==TypeSystem::intType) && (exptype==TypeSystem::floatType) ){
            exptype=TypeSystem::intType;
        }
        if((lvaltype==TypeSystem::floatType) && (exptype==TypeSystem::intType) ){
            exptype=TypeSystem::floatType;
        }
        if(lvaltype!=exptype){
            fprintf(stderr, "ID %s : %s,Exp : %s,type dismatch！！\n", ((IdentifierSymbolEntry*)se)->getName().c_str(),lvaltype->toStr().c_str (),
               exptype->toStr().c_str ());
            exit(EXIT_FAILURE);
        }
        
        
        
    }
    ;
BlockStmt
    :   LBRACE 
        {identifiers = new SymbolTable(identifiers);} 
        Stmts RBRACE 
        {
            //遇见左大括号时，创建新的作用域，其中的语句中涉及的变量作用域都会随之更新，直至遇见右大括号，恢复原来的作用域
            $$ = new CompoundStmt($3);
            SymbolTable *top = identifiers;
            identifiers = identifiers->getPrev();
            delete top;
        }
        |   LBRACE RBRACE{
            $$=new CompoundStmt();
            ifFuncDefStmtIsNull=1;//类型检查加
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
        functionhasRet=true;
        $$ = new ReturnStmt($2);
    }
    | RETURN SEMICOLON {
        functionhasRet=true;
        $$ = new ReturnStmt();
    }   
    ;
WhileStmt
    : WHILE
    {
        whilenum++;
        newWhileStmtPos.push_back(loopctrl.size());
    }   //对每个while都插入一个此前break和continue句子的总数
     LPAREN Cond RPAREN Stmt {
        $$ = new WhileStmt($4, $6);
        
        int lastloop=newWhileStmtPos[newWhileStmtPos.size()-1];
        newWhileStmtPos.erase(newWhileStmtPos.end()-1);
        for(int i=lastloop;i<(int)loopctrl.size();i++)
        {
            loopctrl[i]->setWhileStmt(dynamic_cast<WhileStmt*>($$));
        }
        loopctrl.erase(loopctrl.begin()+lastloop,loopctrl.end());
        whilenum--;
    }
    ;
BrkStmt
    : BREAK SEMICOLON {
        $$ = new BreakStmt();
        loopctrl.push_back(dynamic_cast<loopctl*>($$));
        if(whilenum==0){
            fprintf(stderr, "BrkStmt not in WhileStmt！\n");
            exit(EXIT_FAILURE);
        }

    }
    ;
CtnueStmt
    : CONTINUE SEMICOLON {
        $$ = new ContinueStmt();
        loopctrl.push_back(dynamic_cast<loopctl*>($$));
        if(whilenum==0){
            fprintf(stderr, "CtnueStmt not in WhileStmt！\n");
            exit(EXIT_FAILURE); //make: *** [Makefile:51：run] 错误 1 ：不要慌，是自己设置的错误，不是语法的段错误
        }
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
    :LPAREN LOrExp RPAREN { //把Exp换成LorExp  //对于if( (i<10) && (j==10) )
        $$ = $2;
    }
    | LVal {
        $$ = $1;
    }
    | INTEGER {
        SymbolEntry* se = new ConstantSymbolEntry(TypeSystem::intType, $1);    //改
        $$ = new Constant(se);
        
    }
    | FLOATNUMBER {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::floatType, $1);  //改
        $$ = new Constant(se);
    } 
    | FuncCall {$$=$1;  }     //加(从exp那里移到这里)
    ;

UnaryExp 
    : PrimaryExp {$$ = $1;}
    | ADD UnaryExp {
        //SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        //$$ = new UnaryExpr(se, UnaryExpr::ADD, $2);
        $$=$2;
    }
    | SUB UnaryExp {
        SymbolEntry* se;
        if($2->getSymPtr()->getType()->isFloat())  se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else if( ($2->getSymPtr()->getType()->isFunc()) && (((FunctionType *)($2->getSymPtr()->getType()))-> getRetType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else  se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se, UnaryExpr::SUB, $2);
    }
    | NOT UnaryExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se, UnaryExpr::NOT, $2);
    }
    ;

//新加4个
MulExp
    : UnaryExp {$$ = $1;}
    | MulExp MUL UnaryExp {//debug:float!
        SymbolEntry* se;
        if($1->getSymPtr()->getType()->isFloat() || $3->getSymPtr()->getType()->isFloat())  se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else if( ($1->getSymPtr()->getType()->isFunc()) && (((FunctionType *)($1->getSymPtr()->getType()))-> getRetType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else if( ($3->getSymPtr()->getType()->isFunc()) && (((FunctionType *)($3->getSymPtr()->getType()))-> getRetType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }//debug补充以下
        else if( ($1->getSymPtr()->getType()->isArray()) && (((ArrayType *)($1->getSymPtr()->getType()))-> getFinalType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else if( ($3->getSymPtr()->getType()->isArray()) && (((ArrayType *)($3->getSymPtr()->getType()))-> getFinalType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else  se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MUL, $1, $3);
    }
    | MulExp DIV UnaryExp {
        SymbolEntry* se;
        if($1->getSymPtr()->getType()->isFloat() || $3->getSymPtr()->getType()->isFloat())  se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else if( ($1->getSymPtr()->getType()->isFunc()) && (((FunctionType *)($1->getSymPtr()->getType()))-> getRetType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else if( ($3->getSymPtr()->getType()->isFunc()) && (((FunctionType *)($3->getSymPtr()->getType()))-> getRetType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }//debug补充以下
        else if( ($1->getSymPtr()->getType()->isArray()) && (((ArrayType *)($1->getSymPtr()->getType()))-> getFinalType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else if( ($3->getSymPtr()->getType()->isArray()) && (((ArrayType *)($3->getSymPtr()->getType()))-> getFinalType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else  se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        
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
        SymbolEntry* se;
        if($1->getSymPtr()->getType()->isFloat() || $3->getSymPtr()->getType()->isFloat())  se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else if( ($1->getSymPtr()->getType()->isFunc()) && (((FunctionType *)($1->getSymPtr()->getType()))-> getRetType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else if( ($3->getSymPtr()->getType()->isFunc()) && (((FunctionType *)($3->getSymPtr()->getType()))-> getRetType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }//debug补充以下
        else if( ($1->getSymPtr()->getType()->isArray()) && (((ArrayType *)($1->getSymPtr()->getType()))-> getFinalType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else if( ($3->getSymPtr()->getType()->isArray()) && (((ArrayType *)($3->getSymPtr()->getType()))-> getFinalType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else  se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        
        $$ = new BinaryExpr(se, BinaryExpr::ADD, $1, $3);
    }
    |
    AddExp SUB MulExp
    {
        SymbolEntry* se;
        if($1->getSymPtr()->getType()->isFloat() || $3->getSymPtr()->getType()->isFloat())  se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else if( ($1->getSymPtr()->getType()->isFunc()) && (((FunctionType *)($1->getSymPtr()->getType()))-> getRetType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else if( ($3->getSymPtr()->getType()->isFunc()) && (((FunctionType *)($3->getSymPtr()->getType()))-> getRetType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }//debug补充以下
        else if( ($1->getSymPtr()->getType()->isArray()) && (((ArrayType *)($1->getSymPtr()->getType()))-> getFinalType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else if( ($3->getSymPtr()->getType()->isArray()) && (((ArrayType *)($3->getSymPtr()->getType()))-> getFinalType()->isFloat() ) ){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else  se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
       
        $$ = new BinaryExpr(se, BinaryExpr::SUB, $1, $3);
    }
    ;
RelExp
    : Exp {$$ = $1;}
    | RelExp LESS AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LT, $1, $3);
    }
    | RelExp GREAT AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GT, $1, $3);
    }
    | RelExp LE AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LE, $1, $3);
    }
    | RelExp GE AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GE, $1, $3);
    }
    ;

EqExp
    : RelExp {$$ = $1;}
    | EqExp EQ RelExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::EQ, $1, $3);
    }
    | EqExp NE RelExp {
        SymbolEntry* se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::NE, $1, $3);
    }
    ;

LAndExp
    :
    EqExp {$$ = $1;}
    |
    LAndExp AND EqExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::AND, $1, $3);
    }
    ;
LOrExp
    :
    LAndExp {$$ = $1;}
    |
    LOrExp OR LAndExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::OR, $1, $3);
    }
    ;

//变量常量声明部分
ConstExp
    : AddExp {$$ = $1;}
    ;
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
ArrayIndex
    : LBRACK ConstExp RBRACK {
        $$ = $2;
    }
    | ArrayIndex LBRACK ConstExp RBRACK {
        $$ = $1;
        $1->setNext($3);//把每个维度串起来
    }
    ;
ConstInitVal
    : ConstExp {
        $$ = $1;
        if(!initstack.empty()){
            if(tempType=="INT"){
                equalintarray[myindex++] =$1->getValue();
                Type* artp = initstack.top()->getSymPtr()->getType();
                if(artp == TypeSystem::intconstType)
                    initstack.top()->AddExp($1);
                else
                    while(artp){
                        if(((ArrayType*)artp)->getArrayType() != TypeSystem::intconstType){
                            artp = ((ArrayType*)artp)->getArrayType();
                            SymbolEntry* se = new ConstantSymbolEntry(artp);
                            Initlist* initlist = new Initlist(se);
                            initstack.top()->AddExp(initlist);
                            initstack.push(initlist);
                        }else{
                            initstack.top()->AddExp($1);
                            while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                                artp = ((ArrayType*)artp)->getParentType();
                                initstack.pop();
                            }
                            break;
                        }
                    }
            } 

            if(tempType=="FLOAT"){
                equalfloatarray[myindex++] =$1->getFloatValue();
                Type* artp = initstack.top()->getSymPtr()->getType();
                if(artp == TypeSystem::floatconstType)
                    initstack.top()->AddExp($1);
                else
                    while(artp){
                        if(((ArrayType*)artp)->getArrayType() != TypeSystem::floatconstType){
                            artp = ((ArrayType*)artp)->getArrayType();
                            SymbolEntry* se = new ConstantSymbolEntry(artp);
                            Initlist* initlist = new Initlist(se);
                            initstack.top()->AddExp(initlist);
                            initstack.push(initlist);
                        }else{
                            initstack.top()->AddExp($1);
                            while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                                artp = ((ArrayType*)artp)->getParentType();
                                initstack.pop();
                            }
                            break;
                        }
                    }
            } 
        }   
    }
    | LBRACE RBRACE {
        if(tempType=="INT"){
            SymbolEntry* se;
            ExprNode* initlist;
            if(initstack.empty()){
                // 如果只用一个{}初始化数组，那么栈一定为空
                // 此时也没必要再加入栈了
                memset(equalintarray, 0, arraytype->getArraySize());
                myindex += arraytype->getArraySize() / ((IntType *)TypeSystem::intconstType)->getSize();
                se = new ConstantSymbolEntry(arraytype);
                initlist = new Initlist(se);
            }else{
                // 栈不空说明肯定不是只有{}
                // 此时需要确定{}到底占了几个元素
                int size = ((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArraySize();
                int len = size / ((IntType *)TypeSystem::intconstType)->getSize();
                memset(equalintarray + myindex, 0, size);
                myindex += len;
                Type* type = ((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArrayType();
                se = new ConstantSymbolEntry(type);
                initlist = new Initlist(se);
                initstack.top()->AddExp(initlist);
                while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                    initstack.pop();
                }
            }
            $$ = initlist;
        }

        if(tempType=="FLOAT"){
            SymbolEntry* se;
            ExprNode* initlist;
            if(initstack.empty()){
                // 如果只用一个{}初始化数组，那么栈一定为空
                // 此时也没必要再加入栈了
                memset(equalfloatarray, 0, arraytype->getArraySize());
                myindex += arraytype->getArraySize() / ((FloatType *)TypeSystem::floatconstType)->getSize();
                se = new ConstantSymbolEntry(arraytype);
                initlist = new Initlist(se);
            }else{
                // 栈不空说明肯定不是只有{}
                // 此时需要确定{}到底占了几个元素
                int size = ((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArraySize();
                int len = size / ((FloatType *)TypeSystem::floatconstType)->getSize();
                memset(equalfloatarray + myindex, 0, size);
                myindex += len;
                Type* type = ((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArrayType();
                se = new ConstantSymbolEntry(type);
                initlist = new Initlist(se);
                initstack.top()->AddExp(initlist);
                while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                    initstack.pop();
                }
            }
            $$ = initlist;
        }
        
    }
    | LBRACE {
        bracenum++;
        SymbolEntry* se;
        if(!initstack.empty())
            arraytype = (ArrayType*)(((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArrayType());
        se = new ConstantSymbolEntry(arraytype);
        if(arraytype->getArrayType() != TypeSystem::intconstType || arraytype->getArrayType() != TypeSystem::floatconstType){
            arraytype = (ArrayType*)(arraytype->getArrayType());
        }
        Initlist* expr = new Initlist(se);
        if(!initstack.empty())
            initstack.top()->AddExp(expr);
        initstack.push(expr);
        $<exprtype>$ = expr;

    } 
      ConstInitValList RBRACE {
        bracenum--;
        while(initstack.top() != $<exprtype>2 && initstack.size() > (long unsigned int)(bracenum + 1))
            initstack.pop();
        if(initstack.top() == $<exprtype>2)
            initstack.pop();
        $$ = $<exprtype>2;
        if(!initstack.empty())
            while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                initstack.pop();
            }
        if(tempType=="INT"){
            while(myindex % (((ArrayType*)($$->getSymPtr()->getType()))->getArraySize()/ sizeof(int)) !=0 )
            equalintarray[myindex++] = 0;
        }
        if(tempType=="FLOAT"){
            while(myindex % (((ArrayType*)($$->getSymPtr()->getType()))->getArraySize()/ sizeof(float)) !=0 )
            equalfloatarray[myindex++] = 0.0;
        }
        
        if(!initstack.empty())
            arraytype = (ArrayType*)(((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArrayType());
    }
    ;
ConstDef
    : ID ASSIGN ConstInitVal {
        SymbolEntry *se;
        if(tempType=="INT")
        {
            //IdentifierSymbolEntry(Type *type, std::string name, int scope);
            se = new IdentifierSymbolEntry(TypeSystem::intconstType, $1, identifiers->getLevel());

            //这里得直接获取表达式的值
            //类型转化
            if($3->getSymPtr()->getType()->isFloat()){
                //fprintf(stderr,"int = float!\n");
                ((IdentifierSymbolEntry*)se)->setIntValue((int)($3->getFloatValue()));
            }
            else dynamic_cast<IdentifierSymbolEntry*>(se)->setIntValue($3->getValue());
        }
        if(tempType=="FLOAT")
        {
            se = new IdentifierSymbolEntry(TypeSystem::floatconstType, $1, identifiers->getLevel());
            //类型转化
            if($3->getSymPtr()->getType()->isInt()){
                //fprintf(stderr,"float = int!\n");
                ((IdentifierSymbolEntry*)se)->setFloatValue((float)($3->getValue()));
            }
            else ((IdentifierSymbolEntry*)se)->setFloatValue($3->getFloatValue());
        }
         
        if(!identifiers->install1($1, se)){
            fprintf(stderr, "ConstID %s is already defined\n", (char*)$1);
            exit(EXIT_FAILURE);
        }
                

        //检查左值ID和Exp的类型一样不   //目前不一样还未考虑类型转换，只要不一样就报错  //bug解决：a=getInt(),左边intType，右边funcType，应该取函数类型的返回值类型！
        Type * exptype;                                                             //bug解决：const int a =10;左边是TypeSystem::intconstType；右边是PrimaryExp里的int常数改为TypeSystem::intType，不匹配
        if($3->getSymPtr()->getType()->isFunc() ){
            Type * tmp = ((FunctionType *)($3->getSymPtr()->getType()))-> getRetType(); 
            if(tmp ==TypeSystem::intType ) exptype =TypeSystem::intconstType;
            if(tmp ==TypeSystem::floatType) exptype =TypeSystem::floatconstType;
        } 
        else if ($3->getSymPtr()->getType()->isArray() ){
            exptype = ((ArrayType  *)($3->getSymPtr()->getType()))-> getFinalType();   
        } 
        else if($3->getSymPtr()->getType() ==TypeSystem::intType ){
            exptype =TypeSystem::intconstType;
        }else if($3->getSymPtr()->getType() ==TypeSystem::floatType){
            exptype =TypeSystem::floatconstType;
        }
        else exptype =$3->getSymPtr()->getType();
        
        Type * lvaltype;
        if(((IdentifierSymbolEntry*)se)->getType()->isArray() ){
            lvaltype = ((ArrayType *)(((IdentifierSymbolEntry*)se)->getType()))-> getFinalType();//多维还需完善
            
        } 
        else lvaltype=((IdentifierSymbolEntry*)se)->getType();
        if(lvaltype == TypeSystem::intconstType){
            lvaltype=TypeSystem::intType;
        }
        if(exptype== TypeSystem::intconstType){
            exptype=TypeSystem::intType;
        }
        if(lvaltype == TypeSystem::floatconstType){
            lvaltype=TypeSystem::floatType;
        }
        if(exptype == TypeSystem::floatconstType){
           exptype=TypeSystem::floatType;
        }
        //考虑类型转换,放过
        if((lvaltype==TypeSystem::intType) && (exptype==TypeSystem::floatType) ){
            exptype=TypeSystem::intType;
        }
        if((lvaltype==TypeSystem::floatType) && (exptype==TypeSystem::intType) ){
            exptype=TypeSystem::floatType;
        }
        if(lvaltype!=exptype){
            fprintf(stderr, "ID %s : %s,Exp : %s,type dismatch\n", ((IdentifierSymbolEntry*)se)->getName().c_str(),lvaltype->toStr().c_str (),
               exptype->toStr().c_str ());
            exit(EXIT_FAILURE);
        }
 
        //将这一符号表项存入当前作用域的符号表中
           //identifiers->install($1, se);
            //创建这一个节点
            //ConstDef拿到了一个DeclStmt的属性
            if(se->getType()->isInt() && $3->getSymPtr()->getType()->isFloat() ){
                $$ = new DeclStmt(new Id(se),new FloatToInt($3,($3->getFloatValue())) );
            }
            else if(se->getType()->isFloat() && $3->getSymPtr()->getType()->isInt() ){
                $$ = new DeclStmt(new Id(se),new IntToFloat($3,$3->getValue() ));
            }
            else $$ = new DeclStmt(new Id(se),$3);
            delete []$1;
    }
    | ID ArrayIndex ASSIGN {
        std::vector<int> nums;//存储各维大小
        ExprNode* eachdimension = $2;
        while(eachdimension){
            nums.push_back(eachdimension->getValue());
            eachdimension = (ExprNode*)(eachdimension->getNext());
        }
        Type* type;
        Type* temp1;
        if(tempType=="INT")  type = TypeSystem::intconstType;
        if(tempType=="FLOAT") type = TypeSystem::floatconstType;
        while(!nums.empty()){
            temp1 = new ArrayType(type, nums.back(),true);
            if(type->isArray())
                ((ArrayType*)type)->setParentType(temp1);//1.
            type = temp1;
            nums.pop_back();
        }
        arraytype = (ArrayType*)type;
        myindex = 0;
        std::stack<Initlist*>().swap(initstack);//2.
        SymbolEntry* se = new IdentifierSymbolEntry(arraytype, $1, identifiers->getLevel());
        $<se>$ = se;//?
        if(tempType=="INT")  equalintarray =new int[arraytype->getArraySize()];
        if(tempType=="FLOAT") equalfloatarray =new float[arraytype->getArraySize()];
    }
    ConstInitVal{
        if(tempType=="INT")   ((IdentifierSymbolEntry*)$<se>4)->setIntArrayValue(equalintarray);
        if(tempType=="FLOAT")  ((IdentifierSymbolEntry*)$<se>4)->setFloatArrayValue(equalfloatarray);  
        identifiers->install($1, $<se>4);
        if($<se>4->getType()->isInt() && $5->getSymPtr()->getType()->isFloat() ){
            $$ = new DeclStmt(new Id($<se>4),new FloatToInt($5,$5->getFloatValue()) );
        }
        else if($<se>4->getType()->isFloat() && $5->getSymPtr()->getType()->isInt() ){
            $$ = new DeclStmt(new Id($<se>4),new IntToFloat($5,$5->getValue()) );
        }
        else $$ = new DeclStmt(new Id($<se>4,$2), $5);
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
        //检查该ID此前是否已被定义过！别用lookup查（查所有作用域必能查找无辜的变量），只查当前作用域identifiers里有没有就行
        /*if (identifiers->getsymbolTable().find($1) != identifiers->getsymbolTable().end()) //查找了，报错
        {
            fprintf(stderr, "VarID %s is already defined\n", (char*)$1);
            exit(EXIT_FAILURE);
        }   */
            
        if(!identifiers->install1($1, se)){
            fprintf(stderr, "VarID %s is already defined\n", (char*)$1);
            exit(EXIT_FAILURE);
        }
 
        //将这一符号表项存入当前作用域的符号表中
            //identifiers->install($1, se);
            //创建这一个节点
            //VarDef拿到了一个DeclStmt的属性
            $$ = new DeclStmt(new Id(se));
            delete []$1;
    }
    | ID ASSIGN VarInitVal {
        SymbolEntry *se;
        if(tempType=="INT")
        {
            //IdentifierSymbolEntry(Type *type, std::string name, int scope);
            //给定类型：整型常量 变量名，作用域，将初值表达式给到intValue 刚开始先从简单的起步，后续想在这一阶段直接算出表达式的值，shm学长仓库有，
            se = new IdentifierSymbolEntry(TypeSystem::intType, $1, identifiers->getLevel());

            //同理，也得直接传入值;
            //类型转化
            if($3->getSymPtr()->getType()->isFloat()){
                //fprintf(stderr,"int = float!\n");
                ((IdentifierSymbolEntry*)se)->setIntValue((int)($3->getFloatValue()));
            }
            else  ((IdentifierSymbolEntry*)se)->setIntValue($3->getValue());
        }
        
        if(tempType=="FLOAT")
        {
            se = new IdentifierSymbolEntry(TypeSystem::floatType, $1, identifiers->getLevel());
            //类型转化
            if($3->getSymPtr()->getType()->isInt()){
                //fprintf(stderr,"float = int!\n");
                ((IdentifierSymbolEntry*)se)->setFloatValue((float)($3->getValue()));
            }
            else ((IdentifierSymbolEntry*)se)->setFloatValue($3->getFloatValue());
        }
        //检查重定义
        if(!identifiers->install1($1, se)){
            fprintf(stderr, "VarID %s is already defined\n", (char*)$1);
            exit(EXIT_FAILURE);
        }
        //检查左值ID和Exp的类型一样不   //目前不一样还未考虑类型转换，只要不一样就报错  //bug解决：a=getInt(),左边intType，右边funcType，应该取函数类型的返回值类型！
                                                                                   
        Type * exptype;
        if($3->getSymPtr()->getType()->isFunc() ){
            exptype = ((FunctionType *)($3->getSymPtr()->getType()))-> getRetType();
            
        } 
        else if ($3->getSymPtr()->getType()->isArray() ){
            exptype = ((ArrayType  *)($3->getSymPtr()->getType()))-> getFinalType();   
        } 
        else exptype =$3->getSymPtr()->getType();

        Type * lvaltype;
        if(((IdentifierSymbolEntry*)se)->getType()->isArray() ){
            lvaltype = ((ArrayType *)(((IdentifierSymbolEntry*)se)->getType()))-> getFinalType();//多维还需完善
            
        } 
        else lvaltype=((IdentifierSymbolEntry*)se)->getType();

        if(lvaltype == TypeSystem::intconstType){
            lvaltype=TypeSystem::intType;
        }
        if(exptype== TypeSystem::intconstType){
            exptype=TypeSystem::intType;
        }
        if(lvaltype == TypeSystem::floatconstType){
            lvaltype=TypeSystem::floatType;
        }
        if(exptype == TypeSystem::floatconstType){
           exptype=TypeSystem::floatType;
        }
        //考虑类型转换,放过
        if((lvaltype==TypeSystem::intType) && (exptype==TypeSystem::floatType) ){
            exptype=TypeSystem::intType;
        }
        if((lvaltype==TypeSystem::floatType) && (exptype==TypeSystem::intType) ){
            exptype=TypeSystem::floatType;
        }
        if(lvaltype!=exptype){
            fprintf(stderr, "ID %s : %s,Exp : %s,type dismatch!!!!!\n", ((IdentifierSymbolEntry*)se)->getName().c_str(),lvaltype->toStr().c_str (),
               exptype->toStr().c_str ());
            exit(EXIT_FAILURE);
        }
        

        //将这一符号表项存入当前作用域的符号表中
           //identifiers->install($1, se);
            //创建这一个节点
            //ConstDef拿到了一个DeclStmt的属性
            if(se->getType()->isInt() && $3->getSymPtr()->getType()->isFloat() ){
                fprintf(stderr,"int = float 1203\n");
                $$ = new DeclStmt(new Id(se),new FloatToInt($3,$3->getFloatValue()) );
            }
            else if(se->getType()->isFloat() && $3->getSymPtr()->getType()->isInt() ){
                fprintf(stderr," float =int  1506\n");
                $$ = new DeclStmt(new Id(se),new IntToFloat($3,$3->getValue()) );
            }//debug补充以下
            else if(se->getType()->isFloat() && $3->getSymPtr()->getType()->isFunc()){
                fprintf(stderr," float =func  1509\n");
                Type* ttype = ((FunctionType *)($3->getSymPtr()->getType()))-> getRetType();
                if(ttype->isInt())
                    $$ = new DeclStmt(new Id(se),new IntToFloat($3,$3->getValue()));
                else $$ = new DeclStmt(new Id(se),$3);
            }
            else if(se->getType()->isInt() && $3->getSymPtr()->getType()->isFunc()){
                Type* ttype = ((FunctionType *)($3->getSymPtr()->getType()))-> getRetType();
                if(ttype->isFloat())
                    $$ = new DeclStmt(new Id(se),new FloatToInt($3,$3->getFloatValue()) );
                else $$ = new DeclStmt(new Id(se),$3);
            }
            else if(se->getType()->isFloat() && $3->getSymPtr()->getType()->isArray()){
                fprintf(stderr," float =array  1509\n");
                Type* ttype = ((ArrayType *)($3->getSymPtr()->getType()))-> getFinalType();
                if(ttype->isInt())
                    $$ = new DeclStmt(new Id(se),new IntToFloat($3,$3->getValue()));
                else $$ = new DeclStmt(new Id(se),$3);
            }
            else if(se->getType()->isInt() && $3->getSymPtr()->getType()->isArray()){
                Type* ttype = ((ArrayType *)($3->getSymPtr()->getType()))-> getFinalType();
                if(ttype->isFloat())
                    $$ = new DeclStmt(new Id(se),new FloatToInt($3,$3->getFloatValue()) );
                else $$ = new DeclStmt(new Id(se),$3);
            }
            else $$ = new DeclStmt(new Id(se),$3);
            delete []$1;
    }
    | ID ArrayIndex {
        std::vector<int> nums;//存储各维大小
        ExprNode* eachdimension = $2;
        while(eachdimension){
            nums.push_back(eachdimension->getValue());
            eachdimension = (ExprNode*)(eachdimension->getNext());
        }
        Type *type;
        Type* temp1;
        if(tempType=="INT")  type = TypeSystem::intType;
        if(tempType=="FLOAT") { type = TypeSystem::floatType;}
        for(auto it = nums.rbegin(); it != nums.rend(); it++) {
            temp1 = new ArrayType(type, *it);
            if(type->isArray())
                ((ArrayType*)type)->setParentType(temp1);//1.
            type = temp1;
        }
        arraytype = (ArrayType*)type;
        SymbolEntry* se = new IdentifierSymbolEntry(arraytype, $1, identifiers->getLevel());
        ((IdentifierSymbolEntry*)se)->setquan0();//加
        if(tempType=="INT")  {
            equalintarray =new int[arraytype->getArraySize()];
            ((IdentifierSymbolEntry*)se)->setIntArrayValue(equalintarray);
        }
        if(tempType=="FLOAT"){
            equalfloatarray =new float[arraytype->getArraySize()];
            ((IdentifierSymbolEntry*)se)->setFloatArrayValue(equalfloatarray);  
        } 
        identifiers->install($1, se);
        $$ = new DeclStmt(new Id(se,$2));
        delete []$1;

    }
    | ID ArrayIndex ASSIGN {
        std::vector<int> nums;//存储各维大小
        ExprNode* eachdimension = $2;
        while(eachdimension){
            nums.push_back(eachdimension->getValue());
            eachdimension = (ExprNode*)(eachdimension->getNext());
        }
        Type* type;
        Type* temp1;
        if(tempType=="INT")  type = TypeSystem::intType;
        if(tempType=="FLOAT") type = TypeSystem::floatType;
        while(!nums.empty()){
            temp1 = new ArrayType(type, nums.back());
            if(type->isArray())
                ((ArrayType*)type)->setParentType(temp1);//1.
            type = temp1;
            nums.pop_back();
        }
        arraytype = (ArrayType*)type;
        myindex = 0;
        std::stack<Initlist*>().swap(initstack);//2.
        SymbolEntry* se = new IdentifierSymbolEntry(arraytype, $1, identifiers->getLevel());
        
        $<se>$ = se;//?
        if(tempType=="INT")  equalintarray =new int[arraytype->getArraySize()];
        if(tempType=="FLOAT") equalfloatarray =new float[arraytype->getArraySize()];
        
    }
      VarInitVal{ 
        if(tempType=="INT")   ((IdentifierSymbolEntry*)$<se>4)->setIntArrayValue(equalintarray);
        if(tempType=="FLOAT")  ((IdentifierSymbolEntry*)$<se>4)->setFloatArrayValue(equalfloatarray);  
        if(((Initlist*)$5)->ifempty())
            ((IdentifierSymbolEntry*)$<se>4)->setquan0();
        identifiers->install($1, $<se>4);
        if($<se>4->getType()->isInt() && $5->getSymPtr()->getType()->isFloat() ){
            $$ = new DeclStmt(new Id($<se>4),new FloatToInt($5,$5->getFloatValue()) );
        }
        else if($<se>4->getType()->isFloat() && $5->getSymPtr()->getType()->isInt() ){
            $$ = new DeclStmt(new Id($<se>4),new IntToFloat($5,$5->getValue()) );
        }
        else $$ = new DeclStmt(new Id($<se>4,$2), $5);
        
        delete []$1;
        
    }
    ;
VarInitVal 
    : Exp {  
        $$ = $1;
        if(!initstack.empty())
        {
            if(tempType=="INT")
            {
                equalintarray[myindex++] =$1->getValue();
                Type* artp = initstack.top()->getSymPtr()->getType();
                if(artp == TypeSystem::intType)
                    initstack.top()->AddExp($1);
                else
                    while(artp){
                        if(((ArrayType*)artp)->getArrayType() != TypeSystem::intType){
                            artp = ((ArrayType*)artp)->getArrayType();
                            SymbolEntry* se = new ConstantSymbolEntry(artp);
                            Initlist* initlist = new Initlist(se);
                            initstack.top()->AddExp(initlist);
                            initstack.push(initlist);
                        }else{
                            initstack.top()->AddExp($1);
                            while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                                artp = ((ArrayType*)artp)->getParentType();
                                initstack.pop();
                            }
                            break;
                        }
                    }
            } 

            if(tempType=="FLOAT"){
                equalfloatarray[myindex++] =$1->getFloatValue();
                Type* artp = initstack.top()->getSymPtr()->getType();
                if(artp == TypeSystem::floatType)
                    initstack.top()->AddExp($1);
                else
                    while(artp){
                        if(((ArrayType*)artp)->getArrayType() != TypeSystem::floatType){
                            artp = ((ArrayType*)artp)->getArrayType();
                            SymbolEntry* se = new ConstantSymbolEntry(artp);
                            Initlist* initlist = new Initlist(se);
                            initstack.top()->AddExp(initlist);
                            initstack.push(initlist);
                        }else{
                            initstack.top()->AddExp($1);
                            while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                                artp = ((ArrayType*)artp)->getParentType();
                                initstack.pop();
                            }
                            break;
                        }
                    }
            } 
        }       
    }
    | LBRACE RBRACE {
        
        if(tempType=="INT"){ 
            SymbolEntry* se;
            ExprNode* initlist;
            if(initstack.empty()){
                // 如果只用一个{}初始化数组，那么栈一定为空
                // 此时也没必要再加入栈了
                memset(equalintarray, 0, arraytype->getArraySize());
                
                myindex += arraytype->getArraySize() / ((IntType *)TypeSystem::intType)->getSize();
                
                se = new ConstantSymbolEntry(arraytype);
                initlist = new Initlist(se);
            }else{
                // 栈不空说明肯定不是只有{}
                // 此时需要确定{}到底占了几个元素
                int size = ((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArraySize();
                int len = size / ((IntType *)TypeSystem::intType)->getSize();
                memset(equalintarray + myindex, 0, size);
                myindex += len;
                Type* type = ((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArrayType();
                se = new ConstantSymbolEntry(type);
                initlist = new Initlist(se);
                initstack.top()->AddExp(initlist);
                while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                    initstack.pop();
                }
            }
            $$ = initlist;
        }

        if(tempType=="FLOAT"){
            SymbolEntry* se;
            ExprNode* initlist;
            if(initstack.empty()){
                // 如果只用一个{}初始化数组，那么栈一定为空
                // 此时也没必要再加入栈了
                memset(equalfloatarray, 0, arraytype->getArraySize());
                myindex += arraytype->getArraySize() / ((FloatType *)TypeSystem::floatType)->getSize();
                se = new ConstantSymbolEntry(arraytype);
                initlist = new Initlist(se);
            }else{
                // 栈不空说明肯定不是只有{}
                // 此时需要确定{}到底占了几个元素
                int size = ((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArraySize();
                int len = size / ((FloatType *)TypeSystem::floatType)->getSize();
                memset(equalfloatarray + myindex, 0, size);
                myindex += len;
                Type* type = ((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArrayType();
                se = new ConstantSymbolEntry(type);
                initlist = new Initlist(se);
                initstack.top()->AddExp(initlist);
                while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                    initstack.pop();
                }
            }
            $$ = initlist;
        }
        
    }
    | LBRACE {
        bracenum++;
        SymbolEntry* se;
        if(!initstack.empty())
            arraytype = (ArrayType*)(((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArrayType());
        se = new ConstantSymbolEntry(arraytype);
        /*if(arraytype->getArrayType() != TypeSystem::intType || arraytype->getArrayType() != TypeSystem::floatType){
            arraytype = (ArrayType*)(arraytype->getArrayType());
        }*/
        if(arraytype->getArrayType()->isArray()){
            arraytype = (ArrayType*)(arraytype->getArrayType());
        }
        Initlist* expr = new Initlist(se);
        if(!initstack.empty())
            initstack.top()->AddExp(expr);
        initstack.push(expr);
        $<exprtype>$ = expr;
        
    } 
      VarInitValList RBRACE {
        bracenum--;
        while(initstack.top() != $<exprtype>2 && initstack.size() > (long unsigned int)(bracenum + 1))
            initstack.pop();
        if(initstack.top() == $<exprtype>2)
            initstack.pop();
        $$ = $<exprtype>2;
        if(!initstack.empty())
            while(initstack.top()->ifFull() && initstack.size() != (long unsigned int)bracenum){
                initstack.pop();
            }
        if(tempType=="INT"){
            int x=((ArrayType*)($$->getSymPtr()->getType()))->getArraySize()/ TypeSystem::intType->getOSize();
            while(myindex % x !=0 )
                equalintarray[myindex++] = 0;
        }
        if(tempType=="FLOAT"){
            while(myindex % (((ArrayType*)($$->getSymPtr()->getType()))->getArraySize()/ sizeof(float)) !=0 )
            equalfloatarray[myindex++] = 0.0;
        }
        
        if(!initstack.empty())
            arraytype = (ArrayType*)(((ArrayType*)(initstack.top()->getSymPtr()->getType()))->getArrayType());
        
    }
    ;


VarInitValList
    : VarInitVal {
        $$ = $1;
    }
    | VarInitValList COMM VarInitVal {
        $$ = $1;
    }
    ;
ConstInitValList
    : ConstInitVal {
        $$ = $1;
    }
    | ConstInitValList COMM ConstInitVal {
        $$ = $1;
    }
    ;
FuncDef     //无参的，改成有参形式
    :
    Type ID {
        //声明一个函数类型，函数类型需要存储返回值的类型，同时用动态数组存储各参数的类型。此处参数类型先置空!
        //Type *funcType;
        //funcType = new FunctionType($1,{}); //{}时置空参数
        //针对于函数名这一层面，新建符号表项，（只是一个变量名的符号表，怪怪的，先记着）
        //接入当前的符号表，并新建作用域！
        identifiers = new SymbolTable(identifiers);
        Param_num_for_func=0;//m新加
        functionhasRet=false;
        
    }
    LPAREN FuncParams RPAREN
    {
        FunctionType* functype=new FunctionType($1,{}); 
        //找到函数名所对应的符号表项,将FuncParams开始的所有参数的类型存储到符号表项类型所对应的动态数组中，
        if($5)
        {
            std::vector<Type*> paramType;
            std::vector<SymbolEntry*> params;
            //dynamic_cast<IdentifierSymbolEntry*>($5->getID()->symbolEntry)
        
            DeclStmt* tmp=dynamic_cast<DeclStmt*>($5);
            params.push_back(dynamic_cast<IdentifierSymbolEntry*>(tmp->getID()->getSymPtr()));
            paramType.push_back(dynamic_cast<IdentifierSymbolEntry*>(tmp->getID()->getSymPtr())->getType());
            while(tmp->getNext())
            {
                SymbolEntry*tmpsymbol=dynamic_cast<DeclStmt*>(tmp->getNext())->getID()->getSymPtr();
                params.push_back(dynamic_cast<IdentifierSymbolEntry*>(tmpsymbol));
                paramType.push_back(dynamic_cast<IdentifierSymbolEntry*>(tmpsymbol)->getType());
                tmp=dynamic_cast<DeclStmt*>(tmp->getNext());
            }
            //SymbolEntry *se;
            //se = identifiers->lookup($2);
            
            functype->setparams(params);
            functype->setparamsType(paramType);
        }   
        SymbolEntry *se = new IdentifierSymbolEntry(functype, $2, identifiers->getPrev()->getLevel());
        //identifiers->getPrev()->install($2, se); 
        if(!identifiers->getPrev()->install2($2, se)){
            fprintf(stderr, "func %s is already defined\n", (char*)$2);
            exit(EXIT_FAILURE);
        }    
    }
    BlockStmt
    {
        SymbolEntry *se;
        se = identifiers->lookup($2);
        assert(se != nullptr);
        if(($1==TypeSystem::voidType)&&(!functionhasRet))
        {
            auto tmpnode=new SeqNode($8,new ReturnStmt());
            $8=tmpnode;
        }
        FunctionDef* newFunc=new FunctionDef(se,$5, $8);
        $$ = newFunc;

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
            se = new IdentifierSymbolEntry(TypeSystem::intType, $2, identifiers->getLevel(),Param_num_for_func++);//m新加
        }
        if(tempType=="FLOAT")
        {
            se = new IdentifierSymbolEntry(TypeSystem::floatType, $2, identifiers->getLevel(),Param_num_for_func++);
        }
        //将这一符号表项存入当前作用域的符号表中
            identifiers->install($2, se);
            //创建这一个节点
            //VarDef拿到了一个DeclStmt的属性
            dynamic_cast<IdentifierSymbolEntry*>(se)->setLabel(SymbolTable::getLabel());
            dynamic_cast<IdentifierSymbolEntry*>(se)->setAddr(new Operand(se));
            $$ = new DeclStmt(new Id(se));
            delete []$2;
            
    }
    | Type ID FunArrayIndex{
        // 这里也需要求值
        if(tempType=="FLOAT")  {
            SymbolEntry* se;
            ExprNode* temp = $3;
            Type* arr = TypeSystem::floatType;
            Type* arr1;
            std::stack<ExprNode*> stk;
            while(temp){
                stk.push(temp);
                temp = (ExprNode*)(temp->getNext());
            }
            while(!stk.empty()){
                arr1 = new ArrayType(arr, stk.top()->getValue());
                if(arr->isArray())
                    ((ArrayType*)arr)->setParentType(arr1);
                arr = arr1;
                stk.pop();
            }
            se = new IdentifierSymbolEntry(arr, $2, identifiers->getLevel(),Param_num_for_func++);
            identifiers->install($2, se);
            ((IdentifierSymbolEntry*)se)->setLabel(SymbolTable::getLabel());
            ((IdentifierSymbolEntry*)se)->setAddr(new Operand(se));
            $$ = new DeclStmt(new Id(se));
            delete []$2;
        }
        else{
            SymbolEntry* se;
            ExprNode* temp = $3;
            Type* arr = TypeSystem::intType;
            Type* arr1;
            std::stack<ExprNode*> stk;
            while(temp){
                stk.push(temp);
                temp = (ExprNode*)(temp->getNext());
            }
            while(!stk.empty()){
                arr1 = new ArrayType(arr, stk.top()->getValue());
                if(arr->isArray())
                    ((ArrayType*)arr)->setParentType(arr1);
                arr = arr1;
                stk.pop();
            }
            se = new IdentifierSymbolEntry(arr, $2, identifiers->getLevel(),Param_num_for_func++);
            identifiers->install($2, se);
            ((IdentifierSymbolEntry*)se)->setLabel(SymbolTable::getLabel());
            ((IdentifierSymbolEntry*)se)->setAddr(new Operand(se));
            $$ = new DeclStmt(new Id(se));
            delete []$2;
        }
        
    }
    ;
FunArrayIndex 
    : LBRACK RBRACK {
        
        $$ = new ExprNode(nullptr);//bug解决：实现三个纯虚
    }
    | FunArrayIndex  LBRACK Exp RBRACK {
        $$ = $1;
        $$->setNext($3);
    }
    ;
FuncCall
    : ID LPAREN ParaList RPAREN {
        //要做的是，新建一个函数调用节点，需要包含的信息是函数名的符号表项，以及各个参数的符号表项
        //查找函数名对应的符号表项
        SymbolEntry *se;
        se = identifiers->lookup($1);
        //函数没有被声明过
        if(se == nullptr){
            fprintf(stderr, "function %s is undeclared\n", (char*)$1);
            exit(EXIT_FAILURE);
        }
        
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
