%{
    /************************
        yacc1.y
        YACC file
        Date:2022/10/11
        18147667093@163.com
    *************************/
#include<stdio.h>
#include<stdlib.h>
#ifndef YYSTYPE
#define YYSTYPE double
#endif
int yylex();
extern int yyparse();
FILE* yyin;
void yyerror(const  char*s);
%}

%token ADD SUB MUL DIV NUM LPARE RPARE
%left ADD SUB
%left MUL DIV
%right UMINUS

%%
lines : lines expr '\n' {printf("%f\n",$2);}
    | lines '\n'
    |
    ;

expr : expr ADD expr {$$ = $1 + $3;}
    | expr SUB expr {$$ = $1 - $3;}
    | expr MUL expr {$$ = $1 * $3;}
    | expr DIV expr {$$ = $1 / $3;}
    | LPARE expr RPARE {$$ = $2 ;}
    | SUB expr %prec UMINUS {$$ = -$2;}
    | NUM {$$=$1;}
    ;
%%

int yylex()
{
    int t;
    t=getchar();
	if(t=='+')
	{
		return ADD;
	}
	if(t=='-')
	{
		return SUB;
	}
	if(t=='*')
	{
		return MUL;
	}
	if(t=='/')
	{
		return DIV;
	}
    if(t=='(')
    {
        return LPARE;
    }
    if(t==')')
    {
        return RPARE;
    }
    if((t=='0')||(t=='1')||(t=='2')||(t=='3')||(t=='4')||(t=='5')||(t=='6')||(t=='7')||(t=='8')||(t=='9'))
    {
        switch(t)
        {
            case '0':yylval=0;break;
            case '1':yylval=1;break;
            case '2':yylval=2;break;
            case '3':yylval=3;break;
            case '4':yylval=4;break;
            case '5':yylval=5;break;
            case '6':yylval=6;break;
            case '7':yylval=7;break;
            case '8':yylval=8;break;
            case '9':yylval=9;break;
        }
        return NUM;
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
