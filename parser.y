%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "progetto.h"
%}
%define parse.error verbose /* per vedere l'intero report dalla yyerror */

/* yylval diventa una union, in questo modo posso passare sia int sia stringhe tra flex e bison */
%union {
    struct ast *a;
    double d;
    int fn;			/* which function */
    struct symbol *s;		/* which symbol */
    struct symlist *sl;
};

%token <d> NUMBER
%token EOL
%token <s> NAME
%token DEFINE
%token TAB

%left '+' '-'
%left '*' '/'

%type <a> expr stmt

%start glr
%%

glr: /* nothing */
    | glr stmt EOL { printf("= %4.4g\n ", eval($2)); }
;

stmt:
    define { }
    | expr
;

define:
    DEFINE NAME ':' EOL channelList { printf("HAI Scritto define!!\n"); }
;

channelList: 
    | TAB channel EOL channelList { printf("Burro\n"); }
;

channel:
    NUMBER NAME { printf("Hai definito un canale\n"); }
;

expr:
    expr '+' expr { $$ = newast('+', $1, $3); }
    | expr '-' expr { $$ = newast('-', $1, $3); }
    | expr '*' expr { $$ = newast('*', $1, $3); }
    | expr '/' expr { $$ = newast('/', $1, $3); }
    | NUMBER { $$ = newnum($1); }
    | NAME { $$ = newref($1); }
;

%%
