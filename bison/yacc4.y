%{
    /************************
        yacc4.y
        YACC file
        Date:2022/10/11
        18147667093@163.com
    *************************/
#include<iostream>
#include<stdlib.h>
#include<ctype.h>
#include<string>
#include<map>
#ifndef YYSTYPE
#define YYSTYPE double
#endif
using namespace std;
string idStr;
map<string,double> maps;
int yylex();
extern int yyparse();
FILE* yyin;
void yyerror(const  char*s);
%}

%token ADD SUB MUL DIV NUM LPARE RPARE EQL ID
%left ADD SUB
%left MUL DIV
%right UMINUS

%%
lines : lines expr ';' {cout<<$2<<endl;}
    | lines assign ';'
    | lines ';'
    |
    ;
expr : expr ADD expr {$$ = $1 + $3;}
    | expr SUB expr {$$ = $1 - $3;}
    | expr MUL expr {$$ = $1 * $3;}
    | expr DIV expr {$$ = $1 / $3;}
    | LPARE expr RPARE {$$ = $2 ;}
    | SUB expr %prec UMINUS {$$ = -$2;}
    | NUM {$$=$1;}
    | ID {map<string,double>::iterator resl;
            resl=maps.find(idStr);
            if(resl!=maps.end())
                $$=resl->second;
            else
                $$=0;
            }
    ;

assign : ID EQL expr  {maps[idStr]=$3;}
    ;
%%

int yylex()
{
    int t;
    while(1){
        t=getchar();
        if(t==' '||t=='\t'||t=='\n'){
            //do nothing
        }else if(isdigit(t)){
            yylval=0;
            while(isdigit(t)){
                yylval=yylval*10+t-'0';
                t=getchar();
            }
            ungetc(t,stdin);
            return NUM;
        }else if((t>='a'&&t<='z')||(t>='A'&&t<='Z')||(t=='_')){
            idStr="";
            while((t>='a'&&t<='z')||(t>='A'&&t<='Z')||(t=='_')||(t>='0'&&t<='9')){
                idStr=idStr+(char)t;
                t=getchar();
            }
            //yylval=idStr;
            ungetc(t,stdin);
            return ID;
        }else if(t=='+'){
            return ADD;
        }else if(t=='-'){
            return SUB;
        }else if(t=='*'){
            return MUL;
        }else if(t=='/'){
            return DIV;
        }else if(t=='('){
            return LPARE;
        }else if(t==')'){
            return RPARE;
        }else if(t=='='){
            return EQL;
        }
        else{
            return t;
        }
    }
}
int main(void)
{
    yyin=stdin;
    do{
        yyparse();
    }while(!feof(yyin));
    return 0;
}
void yyerror(const char*s){
    fprintf(stderr,"Parse error:%s\n",s);
    exit(1);
}