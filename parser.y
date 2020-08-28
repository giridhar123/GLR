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
%token <a> NAME
%token DEFINE
%token TAB

%left '+' '-'
%left '*' '/'

%type <a> expr stmt channel channelList define

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
    DEFINE NAME ':' EOL channelList { $$ = newDefine($2, $5); }
;

channelList: TAB channel EOL { $$ = newChannelList($2, NULL); }
    | TAB channel EOL channelList { $$ = newChannelList($2, $4); }
;

channel:
    NUMBER NAME { $$ = newChannel($1, $2); }
;

expr:
    expr '+' expr { $$ = newast('+', $1, $3); }
    | expr '-' expr { $$ = newast('-', $1, $3); }
    | expr '*' expr { $$ = newast('*', $1, $3); }
    | expr '/' expr { $$ = newast('/', $1, $3); }
    | NUMBER { $$ = newnum($1); }
    | NAME { $$ = newref(lookup($1)); }
;

%%
