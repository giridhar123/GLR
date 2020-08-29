%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "progetto.h"
%}
%define parse.error verbose /* per vedere l'intero report dalla yyerror */

/* yylval diventa una union, in questo modo posso passare sia int sia stringhe tra flex e bison */
%union {
    struct ast *a;
    char * string;
    double d;
    int fn;			/* which function */
    struct symbol *s;		/* which symbol */
    struct symlist *sl;
    struct channel *c;
};

%token <d> NUMBER
%token EOL
%token <string> NAME
%token DEFINE
%token TAB
%token READ

%left '+' '-'
%left '*' '/'

%type <a> expr channel channelList define 

%start glr
%%

glr: /* nothing */
    | glr expr EOL { printf("= %4.4g\n ", eval($2)); }
    | glr stmt EOL { printf("Statement\n"); }
    | glr READ path EOL { /*printf("%s",$3);  parseFile($3); */ }
;

path:
    NAME '.' NAME {   strcat($1,strcat('.',$3));  }
    | NAME '/' path {  /*strcat($1,strcat("/",$3)); */ }
    
;

stmt:
    define { }
    | assegnazione
;

define:
    DEFINE NAME ':' EOL channelList { $$ = newDefine($2, $5); }
;

channelList: TAB channel EOL { $$ = newChannelList($2, NULL); }
    | TAB channel EOL channelList { $$ = newChannelList($2, $4); }
;

channel:
    NUMBER NAME { $$ = newChannel($1, $2); }
;

assegnazione:
    NAME NAME '=' expr { newFixture($1, $2, eval($4)); }
    | NAME '.' NAME '=' NUMBER { setChannelValue($1, $3, $5); }
;

expr:
    expr '+' expr { $$ = newast('+', $1, $3); }
    | expr '-' expr { $$ = newast('-', $1, $3); }
    | expr '*' expr { $$ = newast('*', $1, $3); }
    | expr '/' expr { $$ = newast('/', $1, $3); }
    | NUMBER { $$ = newnum($1); }
    | NAME { $$ = newInvoke($1); }
;

%%
