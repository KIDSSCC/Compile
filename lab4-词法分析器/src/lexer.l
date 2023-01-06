%option noyywrap
%option yylineno
%{
    /*
    * You will need to comment this line in lab5.
    */
    #define ONLY_FOR_LEX
    
    #ifdef ONLY_FOR_LEX
    #else
    #define YYSTYPE void *
    #include "parser.h"
    #endif

    #define YY_NO_UNPUT
    #define YY_NO_INPUT
    #include <string>

    #ifdef ONLY_FOR_LEX
    #include <ostream>
    #include <fstream>
    using namespace std;
    extern FILE *yyin; 
    extern FILE *yyout;


    #include<list>
    #include<stack>
    //符号表结构体，保存id，类型（int或float ps:不确定有没有用，先写着）,变量值，所处的行号，作用域相关，
    struct symbol{
        string idName;
        bool type;//0代表int，1代表float
        int intValue;
        float floatValue;
        int lineno;
        int scopebegin=0;  //作用域的起始，即左大括号或者程序起始（全局变量）
        int scopeend=-1; //作用域的结束，以右大括号所处的行号为准，当其为-1时，说明作用域还没结束，这个id还在生效。
    };
    //存储符号表二等链表结构
    list<symbol>symbolList;
    //存储作用域的栈结构
    stack<int> scope;
    int offset=0;
    bool whetherDefine;   //该变量用以区分是否是一条变量声明语句，在遇到int和float时置位，遇到分号和换行时恢复。
    void DEBUG_FOR_LAB4(std::string s,symbol*p=NULL){
        std::string DEBUG_INFO;
        if(p==NULL)
        {
            DEBUG_INFO = "[DEBUG LAB4]: \t" + to_string(yylineno) + "\t\t" + to_string(offset)+"\t\t"+ s+ "\n";
        }
        else
        {
            char tmp[20];
            sprintf(tmp,"0x%lx",(unsigned long int)p);
            DEBUG_INFO = "[DEBUG LAB4]: \t" + to_string(yylineno) + "\t\t" + to_string(offset)+"\t\t"+ s+ "\t"+string(tmp)+"\n";
        }
        fputs(DEBUG_INFO.c_str(), yyout);
    }
    #endif
%}

DECIMIAL ([1-9][0-9]*|0)
OCTAL (0[0-7]+)
HEXADECIMAL (0[xX][0-9A-Fa-f]+)
ID [[:alpha:]_][[:alpha:][:digit:]_]*
EOL (\r\n|\n|\r)
WHITE [\t ]

COMMENTBEGIN   \/\*                        
COMMENTELEMENT .|\n                         
COMMENTEND \*\/
%x BLOCKCOMMENT
SINGLECOMMENT \/\/[^\n]* 

DECIMIALFLOAT  ([1-9][0-9]*|0)(\.[0-9]+)?([Ee][+-]?([1-9][0-9]*|0))?
OCTALFLOAT     (0[0-7]+)(\.[0-7]+)?([Pp][+-]?[0-7]+)?
HEXADECIMALFLOAT  (0[xX][0-9a-fA-F]+)(\.[0-9a-fA-F]+)?([Pp][+-]?[0-9a-fA-F]+)?
%%

"int" {
    /*
    * Questions: 
    *   Q1: Why we need to return INT in further labs?
    *   Q2: What is "INT" actually?
    */
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("INT\t\t\tint\t\t");
        offset += strlen("int");
        whetherDefine=true;//声明语句，插一下flag
    #else
        return INT;
    #endif
}
"float" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("FLOAT\t\tfloat\t");
        offset += strlen("float");
        whetherDefine=true;//声明语句，插一下flag
    #else
        return FLOAT;
    #endif
}
"const" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("CONST\t\tconst\t");
        offset += strlen("const");
    #else
        return CONST;
    #endif 
}
"void" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("VOID\t\tvoid\t");
        offset += strlen("void");
    #else
        return VOID;
    #endif 
}
"main" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("MAIN\t\tmain\t");
        offset += strlen("main");
    #else
        return MAIN;
    #endif
}
"if" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("IF\t\t\tif\t\t");
        offset += strlen("if");
    #else
        return IF;
    #endif
};
"else" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ELSE\t\telse\t");
        offset += strlen("else");
    #else
        return ELSE;
    #endif
};
"return" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RETURN\t\treturn\t");
        offset += strlen("return");
    #else
        return RETURN;
    #endif
}
"while" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("WHILE\t\twhile\t");
        offset += strlen("while");
    #else
        return WHILE;
    #endif
}
"break" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("BREAK\t\tbreak\t");
        offset += strlen("break");
    #else
        return BREAK;
    #endif
}
"continue" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("CONTINUE\tcontinue");
        offset += strlen("continue");
    #else
        return CONTINUE;
    #endif
}
"=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ASSIGN\t\t=\t\t");
        offset += strlen("=");
    #else
        return ASSIGN;
    #endif
}
"<" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LESS\t\t<\t\t");
        offset += strlen("<");
    #else
        return LESS;
    #endif
}
">" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("GREAT\t\t>\t\t");
        offset += strlen(">");
    #else
        return GREAT;
    #endif
}
"==" {                                               //新的，与asign优先级？
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("EQ\t\t\t==\t\t");
        offset += strlen("==");
    #else
        return EQ;
    #endif
}
"!=" {                                          //新的
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("NE\t\t\t!=\t\t");
        offset += strlen("!=");
    #else
        return NE;
    #endif
}
"<=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LE\t\t\t<=\t\t");
        offset += strlen("<=");
    #else
        return LE;
    #endif
}
">=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("GE\t\t\t<=\t\t");
        offset += strlen(">=");
    #else
        return GE;
    #endif
}
"+" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ADD\t\t\t+\t\t");
        offset += strlen("+");
    #else
        return ADD;
    #endif
}
"-" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("SUB\t\t\t-\t\t");
        offset += strlen("-");
    #else
        return SUB;
    #endif
}
"*" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("MUL\t\t\t*\t\t");
        offset += strlen("*");
    #else
        return MUL;
    #endif
}
"/" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("DIV\t\t\t/\t\t");
        offset += strlen("/");
    #else
        return DIV;
    #endif
}
"%" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("MOD\t\t\t%\t\t");
        offset += strlen("%");
    #else
        return MOD;
    #endif
}
"&&" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("AND\t\t\t&&\t\t");
        offset += strlen("&&");
    #else
        return AND;
    #endif
}
"||" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("OR\t\t\t||\t\t");
        offset += strlen("||");
    #else
        return OR;
    #endif
}
"!" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("NOT\t\t\t!\t\t");
        offset += strlen("!");
    #else
        return NOT;
    #endif
}
";" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("SEMI\t\t;\t\t");
        offset += strlen(";");
        whetherDefine=false;//对whetherdefine进行恢复
    #else
        return SEMICOLON;
    #endif
}
"," {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("COMM\t\t,\t\t");
        offset += strlen(",");
    #else
        return SEMICOLON;
    #endif
}
"(" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LPAREN\t\t(\t\t");
        offset += strlen("(");
    #else
        return LPAREN;
    #endif
}
")" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RPAREN\t\t)\t\t");
        offset += strlen(")");
    #else
    return RPAREN;
    #endif
}
"[" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LBRACK\t\t[\t\t");
        offset += strlen("[");
    #else
        return LBRACK;
    #endif
}
"]" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RBRACE\t\t]\t\t");
        offset += strlen("]");
    #else
        return RBRACK;
    #endif
}
"{" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LBRACE\t\t{\t\t");
        offset += strlen("{");
        scope.push(yylineno);//一个新的作用域开始，进行压栈
    #else
        return LBRACE;
    #endif
}
"}" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RBRACE\t\t}\t\t");
        offset += strlen("}");
        //一个作用域结束了，遍历当前的符号表，标记所有处于这个作用域内的变量已死亡。
        int scopeBeginNow=scope.top();
        for(list<symbol>::iterator it = symbolList.begin();it!=symbolList.end();it++)
        {
            if(it->scopebegin==scopeBeginNow)
            {
                it->scopeend=yylineno;
            }
        }
        scope.pop();
    #else
        return RBRACE;
    #endif
}

{EOL} {offset=0;whetherDefine=false;}
{WHITE} {offset += strlen(yytext);}
{SINGLECOMMENT}   {}          //啥都不干
{COMMENTBEGIN} {BEGIN BLOCKCOMMENT;}    //多行注释
<BLOCKCOMMENT>{COMMENTELEMENT} {}
<BLOCKCOMMENT>{COMMENTEND} {BEGIN INITIAL;}
{ID} {
    //当前的作用域起始位置
    int scopeBeginNow=scope.top();
    //变量名
    string id=string(yytext);
    //查到当前变量之前是否出现过，用以记录查询结果的一个bool变量
    bool whetherExist1=false;
    for(list<symbol>::iterator it = symbolList.begin();it!=symbolList.end();it++)
    {
        //it->scopebegin==scopeBeginNow，查找在当前作用域内，有没有已经声明过的同名变量，如果有，则这两个变量是同一个。
        if((it->idName==id)&&(it->scopebegin==scopeBeginNow))
        {
            whetherExist1=true;
            DEBUG_FOR_LAB4("ID\t\t\t"+id+"\t\t",&(*it));
            break;
        }
    }
    //看看在上面的for循环中有没有找到同名的变量，如果条件成立，则说明在同一个作用域内，没有与其同名的变量。
    if(!whetherExist1)
    {
        //看一下这是否是一条声明语句，如果是，说明这一定是一个新出现的变量，将其接入链表，
        if(whetherDefine)
        {
            symbol tmp;
            tmp.idName=id;
            tmp.lineno=yylineno;
            tmp.scopebegin=scopeBeginNow;
            symbolList.push_back(tmp);
            //这里的symbolList.end()也是一个迭代器，而且指向最后一个元素结束的位置，因此进行--，来使其指向最后一个元素开始的地方。
            list<symbol>::iterator back=symbolList.end();
            back--;
            DEBUG_FOR_LAB4("ID\t\t\t"+id+"\t\t",&(*back));
        }
        else
        {
            //在一对大括号里，之前没有声明过，这也不是一条声明语句，说明这个变量应该是一个外部的变量，因此去外部，且仍处于有效状态的变量中去寻找，
            bool whetherExist2=false;
            for(list<symbol>::iterator it = symbolList.begin();it!=symbolList.end();it++)
            {
                //it->scopeend==-1即说明这个变量还没有遇到属于自己的大括号，还是有效的
                if((it->idName==id)&&(it->scopeend==-1))
                {
                    whetherExist2=true;
                    DEBUG_FOR_LAB4("ID\t\t\t"+id+"\t\t",&(*it));
                    break;
                }
            }
            //在这种情况下还是没有找到，就像测试样例中的_testid1，是一个没有声明过就直接使用的变量，虽然是错误的语法，也...新建一下吧，怪可怜的。
            if(!whetherExist2)
            {
                symbol tmp;
                tmp.idName=id;
                tmp.lineno=yylineno;
                tmp.scopebegin=scopeBeginNow;
                symbolList.push_back(tmp);
                list<symbol>::iterator back=symbolList.end();
                back--;
                DEBUG_FOR_LAB4("ID\t\t\t"+id+"\t\t",&(*back));
            }
        }
    }
    offset += strlen(yytext);
}
{DECIMIAL} {
    #ifdef ONLY_FOR_LEX 
        string temp="NUMBER\t\t";
        string arg=temp+yytext;     
        DEBUG_FOR_LAB4(arg+"\t\t"); //可以吧？
    #else
        return INTEGERCONST;
    #endif
}
{OCTAL} {
    #ifdef ONLY_FOR_LEX
        int temp;
        sscanf(yytext, "%o", &temp);    //sscanf是把字符串转为八进制数值赋给temp
        DEBUG_FOR_LAB4("NUMBER\t\t"+to_string(temp)+"\t\t");
    #else
        return INTEGERCONST;
    #endif
}
{HEXADECIMAL} {
    #ifdef ONLY_FOR_LEX
        int temp;
        sscanf(yytext, "%x", &temp);
        DEBUG_FOR_LAB4("NUMBER\t\t"+to_string(temp)+"\t\t");
    #else
        return INTEGERCONST;
    #endif
}

{DECIMIALFLOAT} {
    #ifdef ONLY_FOR_LEX
        float temp;
        sscanf(yytext, "%f", &temp);                //totring会显示6位小数，不是原模原样！
        DEBUG_FOR_LAB4("NUMBER\t\t"+to_string(temp)+"\t\t");    
    #else
        return FLOATCONST;
    #endif
}
{OCTALFLOAT} {
    #ifdef ONLY_FOR_LEX
        float temp;
        sscanf(yytext, "%f", &temp);                     //如何既o又f？
        DEBUG_FOR_LAB4("NUMBER\t\t"+to_string(temp)+"\t\t");
    #else
        return FLOATCONST;
    #endif
}
{HEXADECIMALFLOAT} {
    #ifdef ONLY_FOR_LEX
        float temp;
        sscanf(yytext, "%f", &temp);                     //如何既x又f？
        DEBUG_FOR_LAB4("NUMBER\t\t"+to_string(temp)+"\t\t");
    #else
        return FLOATCONST;
    #endif
}
%%

#ifdef ONLY_FOR_LEX
int main(int argc, char **argv){
    scope.push(0);
    if(argc != 5){
        fprintf(stderr, "Argument Not Enough");
        exit(EXIT_FAILURE);
    }

    if(!(yyin = fopen(argv[1], "r"))){
        fprintf(stderr, "No such file or directory: %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    if(!(yyout = fopen(argv[3], "w"))){
        fprintf(stderr, "No such file or directory: %s", argv[3]);
        exit(EXIT_FAILURE);
    }

    yylex();
    return 0;
}
#endif
