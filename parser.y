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

    struct astList * al;
};

%token <d> NUMBER
%token EOL
%token <string> NAME
%token DEFINE
%token TAB
%token READ
%token LOOP
%token FROM
%token TO
%token O_BRACKET
%token C_BRACKET

%token <fn> FUNC
%token IF

%nonassoc <fn> CMP
%left '+' '-'
%left '*' '/'

%type <string> path 
%type <a> expr channel channelList define assignment stmt loop
%type <al> stmtList

%start glr
%%

glr: /* nothing */
    | glr expr EOL { printf("= %4.4g\n ", eval($2)); freeAst($2); }
    | glr stmt EOL { eval($2); }
    | glr preprocessing EOL { }
;

preprocessing:
    read {}
    | define {}
;

read:
    READ path { parseFile($2); }
;


path:
    NAME '.' NAME {
                        $$ = malloc(sizeof(char) * (strlen($1) + 1 + strlen($3) + 2));
                        $$ = strcat($$, $1);
                        $$ = strcat($$, ".");
                        $$ = strcat($$, $3);
                    }
    | NAME '/' path {
                        $$ = malloc(sizeof(char) * (strlen($1) + 1 + strlen($3) + 2));
                        $$ = strcat($$, $1);
                        $$ = strcat($$, "/");
                        $$ = strcat($$, $3);
                    }   
;

define:
    DEFINE NAME EOL O_BRACKET channelList C_BRACKET { $$ = newDefine($2, $5); }
    | DEFINE NAME EOL O_BRACKET EOL channelList C_BRACKET { $$ = newDefine($2, $6); }
;

channelList: 
    channel EOL { $$ = newChannelList($1, NULL); }
    | channel EOL channelList { $$ = newChannelList($1, $3); }
;

channel:
    NUMBER NAME { $$ = newChannel($1, $2); }
;

stmt:
    assignment { $$ = $1; }
    | loop { $$ = $1; }
;

assignment:
    NAME NAME '=' expr { $$ = newFixture($1, $2, eval($4)); }
    | NAME '.' NAME '=' NUMBER { $$ = setChannelValue($1, $3, $5); }
;

stmtList:
    stmt EOL { $$ = newAstList($1, NULL); }
    | stmt EOL stmtList { $$ = newAstList($1, $3); } 
;

loop:
    LOOP FROM NUMBER TO NUMBER EOL O_BRACKET stmtList C_BRACKET { $$ = newLoop("i", $3, $5, $8); } 
    | LOOP FROM NUMBER TO NUMBER EOL O_BRACKET EOL stmtList C_BRACKET { $$ = newLoop("i", $3, $5, $9); } 
;

expr:
    expr '+' expr { $$ = newast('+', $1, $3); }
    | expr '-' expr { $$ = newast('-', $1, $3); }
    | expr '*' expr { $$ = newast('*', $1, $3); }
    | expr '/' expr { $$ = newast('/', $1, $3); }
    | NUMBER CMP NUMBER { $$ = ifcase($2,$1,$3); }
    | NUMBER { $$ = newnum($1); }
    | NAME { $$ = newInvoke($1); }

;

%%
