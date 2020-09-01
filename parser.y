%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "headers/parser.h"
    #include "headers/astbuilder.h"
    
    //per eliminare i warning
    int yylex(void);
    void freeAst();
    int yyparse();
%}
%define parse.error verbose /* per vedere l'intero report dalla yyerror */

/* yylval diventa una union, in questo modo posso passare sia int sia stringhe tra flex e bison */
%union {
    struct ast *a;
    char * string;
    double d;
    int fn;			/* which function */
    struct var * v;		/* which symbol */
    struct symlist *sl;
    struct channel *c;

    struct lookup * l;

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
%token O_ARRAY
%token C_ARRAY
%token FADE
%token DELAY
%token IN
%token SECONDS

%token <fn> FUNC
%token IF
%token THEN 
%token ELSE
%token DO
%token SLEEP
%token MACRO

%nonassoc <fn> CMP
%left '+' '-'
%left '*' '/'

%type <string> path 
%type <a> expr channel channelList define assignment stmt loopStmt ifStmt sleep macroDefine strutturaifsingle
%type <al> stmtList
%type <l> variable

%start glr
%%

glr: /* nothing */
    | glr expr EOL { printf("= %4.4g\n ", eval($2)); freeAst($2); }
    | glr stmt EOL { eval($2); }
    | glr preprocessing EOL { }
;

preprocessing:
    READ path { parseFile($2); }
    | define {}
    | macroDefine {}
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
    | loopStmt { $$ = $1; }
    | ifStmt { $$ = $1; }
    | sleep {$$ = $1; }
;

assignment:
    NAME variable '=' expr { $$ = newFixture($1, $2, $4); }
    | NAME NAME O_ARRAY expr C_ARRAY '=' expr { $$ = newCreateArray(lookupFixtureType($1), lookupVar($2), $4, $7); }
    | variable '.' NAME '=' expr { $$ = newSetChannelValue($1, $3, $5); }
    | variable '.' NAME '=' expr FADE IN expr SECONDS { $$ = newFade($1, $3, $5, $8); }
    | variable '.' NAME '=' expr DELAY IN expr SECONDS { $$ = newDelay($1, $3, $5, $8); }
;

variable:
    NAME { $$ = newLookup($1); } /* fixtureType oppure varName */
    | NAME O_ARRAY expr C_ARRAY { $$ = newLookupFromArray($1, $3); }
;

loopStmt:
    LOOP NAME FROM NUMBER TO NUMBER EOL O_BRACKET stmtList C_BRACKET { $$ = newLoop($2, $4, $6, $9); } 
    | LOOP NAME FROM NUMBER TO NUMBER EOL O_BRACKET EOL stmtList C_BRACKET { $$ = newLoop($2, $4, $6, $10); } 
;

ifStmt:
    IF expr EOL O_BRACKET EOL stmtList C_BRACKET { $$ = newIf($2, $6, NULL); /* DA SISTEMARE L ACCAPO DI ELSE */ }
    | IF expr EOL O_BRACKET EOL stmtList C_BRACKET ELSE EOL O_BRACKET EOL stmtList C_BRACKET { $$ = newIf( $2, $6, $12); }
    | IF expr EOL O_BRACKET EOL stmtList C_BRACKET ELSE strutturaifsingle { $$ = newIf( $2, $6, AstToAstList($9)); }
    | IF expr strutturaifsingle EOL {$$ = newIf($2, AstToAstList($3), NULL); }    
    | IF expr strutturaifsingle EOL ELSE strutturaifsingle {$$ = newIf($2, AstToAstList($3), AstToAstList($6)); }
 ;

strutturaifsingle:
     expr { $$ = $1; }
    | stmt {$$ = $1; }
    | EOL expr {$$ =$2; }
    | EOL stmt {$$ =$2; }

;

sleep:
    SLEEP expr SECONDS { $$ = newSleep($2); }
;

macroDefine:
    MACRO NAME EOL O_BRACKET EOL stmtList C_BRACKET { $$ = newMacroDefine($2, $6); }
;

stmtList:
    stmt EOL { $$ = newAstList($1, NULL); }
    | expr EOL { $$ = newAstList($1, NULL); }
    | stmt EOL stmtList { $$ = newAstList($1, $3); }
    | expr EOL stmtList { $$ = newAstList($1, $3); } 
;

expr:
    expr '+' expr { $$ = newast('+', $1, $3); }
    | expr '-' expr { $$ = newast('-', $1, $3); }
    | expr '*' expr { $$ = newast('*', $1, $3); }
    | expr '/' expr { $$ = newast('/', $1, $3); }
    | expr CMP expr { $$ = newCompare($2, $1, $3); }
    | NUMBER { $$ = newnum($1); }
    | variable { $$ = (struct ast *) $1; }
;

%%
