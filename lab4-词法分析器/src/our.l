%option noyywrap
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

    int chars = 0;

    // https://www.docin.com/p-1224415272.html
    // https://www.pianshen.com/article/46271998229/
    struct Symbol{
        string name;
        int kind;
        void* typePtr;
        int value;
        // 变量的话表示 dir/indir
        // 函数的话表示 actual/formal
        bool access;
        int level;
        int offset;
    };

    Symbol symbol[10][10];
    int symbolLen[10];
    int symbolArrIdx = 0;
    int symbolIdx = 0;

    #include <stack>
    #include <list>

    list<int> zero{0};

    stack<int,list<int>> scope(zero);
    

    void DEBUG_FOR_LAB4(std::string s, Symbol* ptr = NULL){
        string DEBUG_INFO;
        if(ptr == NULL)
            DEBUG_INFO = "[DEBUG LAB4]: \t" + s + "\t" + to_string(yylineno) + "\t" + to_string(chars) + "\n";
        else{
            char temp[20];
            //长整型，16进制
            sprintf(temp, "0x%lx", (long unsigned int)ptr);
            DEBUG_INFO = "[DEBUG LAB4]: \t" + s + "\t" + to_string(yylineno) + "\t" + to_string(chars) + "\t" +string(temp) + "\n";
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
BLOCKCOMMENTBEGIN \/\*
BLOCKCOMMENTELEMENT .
BLOCKCOMMENTEND \*\/
%x BLOCKCOMMENT
LINECOMMENT \/\/[^\n]*

%%

"int" {
    /*
    * Questions: 
    *   Q1: Why we need to return INT in further labs?
    *   Q2: What is "INT" actually?
    */
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("INT\tint");
        chars += strlen("int");
    #else
        return INT;
    #endif
}
"void" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("VOID\tvoid");
        chars += strlen("void");
    #else
        return VOID;
    #endif 
}
"const" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("CONST\tconst");
        chars += strlen("const");
    #else
        return CONST;
    #endif 
}
"if" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("IF\tif");
        chars += strlen("if");
    #else
        return IF;
    #endif
}
"else" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ELSE\telse");
        chars += strlen("else");
    #else
        return ELSE;
    #endif
}
"return" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RETURN\treturn");
        chars += strlen("return");
    #else
        return RETURN;
    #endif
}
"while" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("WHILE\twhile");
        chars += strlen("while");
    #else
        return WHILE;
    #endif
}
"break" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("BREAK\tbreak");
        chars += strlen("break");
    #else
        return BREAK;
    #endif
}
"continue" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("CONTINUE\tcontinue");
        chars += strlen("continue");
    #else
        return CONTINUE;
    #endif
}
"==" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("EQUAL\t==");
        chars += strlen("==");
    #else
        return EQUAL;
    #endif
}
"!=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("NOTEQUAL\t!=");
        chars += strlen("!=");
    #else
        return NOTEQUAL;
    #endif
}
"=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ASSIGN\t=");
        chars += strlen("=");
    #else
        return ASSIGN;
    #endif
}
"<=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LESSEQUAL\t<=");
        chars += strlen("<=");
    #else
        return LESSEQUAL;
    #endif
}
">=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("GREATEREQUAL\t<=");
        chars += strlen(">=");
    #else
        return GREATEREQUAL;
    #endif
}
"<" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LESS\t<");
        chars += strlen("<");
    #else
        return LESS;
    #endif
}
">" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("GREATER\t>");
        chars += strlen(">");
    #else
        return GREATER;
    #endif
}
"+" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ADD\t+");
        chars += strlen("+");
    #else
        return ADD;
    #endif
}
"-" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("SUB\t-");
        chars += strlen("-");
    #else
        return SUB;
    #endif
}
"*" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("MUL\t*");
        chars += strlen("*");
    #else
        return MUL;
    #endif
}
"/" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("DIV\t/");
        chars += strlen("/");
    #else
        return DIV;
    #endif
}
"%" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("MOD\t%");
        chars += strlen("%");
    #else
        return MOD;
    #endif
}
"&&" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("AND\t&&");
        chars += strlen("&&");
    #else
        return AND;
    #endif
}
"||" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("OR\t||");
        chars += strlen("||");
    #else
        return OR;
    #endif
}
"!" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("NOT\t!");
        chars += strlen("!");
    #else
        return NOT;
    #endif
}
";" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("SEMICOLON\t;");
        chars += strlen(";");
    #else
        return SEMICOLON;
    #endif
}
"(" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LPAREN\t(");
        chars += strlen("(");
    #else
        return LPAREN;
    #endif
}
")" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RPAREN\t)");
        chars += strlen(")");
    #else
    return RPAREN;
    #endif
}
"{" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LBRACE\t{");
        chars += strlen("{");
        symbolLen[symbolArrIdx] = symbolIdx + 1;
        scope.push(++symbolArrIdx);
        symbolIdx = 0;
    #else
        return LBRACE;
    #endif
}
"}" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RBRACE\t}");
        chars += strlen("}");
        int temp = 0;
        if(!scope.empty()){
            temp = scope.top();
            scope.pop();
        }
        symbolIdx = (symbolLen[temp] - 1) >= 0 ? (symbolLen[temp] - 1): 0;
    #else
        return RBRACE;
    #endif
}
"[" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LBRACKET\t[");
        chars += strlen("[");
    #else
        return LBRACKET;
    #endif
}
"]" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RBRACKET\t}");
        chars += strlen("]");
    #else
        return RBRACKET;
    #endif
}
"," {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("COMMA\t,");
        chars += strlen(",");
    #else
        return COMMA;
    #endif
}


{DECIMIAL} {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4(string("NUMBER\t") + string(yytext));\
        chars += strlen(yytext);
    #else
        return INTEGER;
    #endif
}

{OCTAL} {
    #ifdef ONLY_FOR_LEX
        int temp;
        sscanf(yytext, "%o", &temp);
        DEBUG_FOR_LAB4(string("NUMBER\t") + to_string(temp));
        chars += strlen(yytext);
    #else
        return INTEGER;
    #endif
}

{HEXADECIMAL} {
    #ifdef ONLY_FOR_LEX
        int temp;
        sscanf(yytext, "%x", &temp);
        DEBUG_FOR_LAB4(string("NUMBER\t") + to_string(temp));
        chars += strlen(yytext);
    #else
        return INTEGER;
    #endif
}


<*>{EOL} {
    chars += strlen(yytext);
    yylineno++;
    chars = 0;
}

{WHITE} {chars += strlen(yytext);}
{LINECOMMENT} {}
{BLOCKCOMMENTBEGIN} {BEGIN BLOCKCOMMENT;}
<BLOCKCOMMENT>{BLOCKCOMMENTELEMENT} {}
<BLOCKCOMMENT>{BLOCKCOMMENTEND} {BEGIN INITIAL;}

{ID} {
    stack<int> temp;
    bool flag = false;
    while(!scope.empty()){
        int arrIdx = scope.top();
        scope.pop();
        temp.push(arrIdx);
        for(int i=0;i<symbolLen[arrIdx];i++){
            if(symbol[arrIdx][i].name == string(yytext)){
                DEBUG_FOR_LAB4(string("ID\t") + string(yytext), &(symbol[arrIdx][i]));
                flag = true;
                break;
            }
        }
        if(flag){
            break;
        }
    }
    while(!temp.empty()){
        scope.push(temp.top());
        temp.pop();
    }
    if(!flag){
        int arrIdx = scope.top();
        for(int i=0;i<symbolIdx;i++)
            if(symbol[arrIdx][i].name == string(yytext)){
                DEBUG_FOR_LAB4(string("ID\t") + string(yytext), &(symbol[arrIdx][i]));
                flag = true;
                break;
            }
        if(!flag){
            symbol[arrIdx][symbolIdx].name = string(yytext);
            DEBUG_FOR_LAB4(string("ID\t") + string(yytext), &(symbol[arrIdx][symbolIdx++]));
        }.
    }
}

%%

#ifdef ONLY_FOR_LEX
int main(int argc, char **argv){
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