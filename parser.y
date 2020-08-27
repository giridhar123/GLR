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
};

%token <d> NUMBER
%token EOL

%left '+' '-'
%left '*' '/'

%type <a> expr

%start glr
%%

glr: /* nothing */
    | glr expr EOL { printf("= %4.4g\n> ", eval($2)); }
;

expr:
    expr '+' expr { $$ = newast('+', $1, $3); }
    | expr '-' expr { $$ = newast('-', $1, $3); }
    | expr '*' expr { $$ = newast('*', $1, $3); }
    | expr '/' expr { $$ = newast('/', $1, $3); }
    | NUMBER { $$ = newnum($1); }
;

%%
