%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "headers/parser.h"
    #include "headers/astbuilder.h"
    #include "headers/parserUtils.h"
    #include "headers/sharedVariables.h"
    #include "headers/free.h"
    
    //per eliminare i warning
    int yylex(void);
    int yyparse();
    void deleteMacro();
    void deleteVar();
%}
%define parse.error verbose // per vedere l'intero report dalla yyerror

// yylval diventa una union, in questo modo posso passare sia int sia stringhe tra flex e bison
%union {
    struct ast *a;
    char * string;
    double d;
    int fn;	
    struct var * v;	
    struct symlist *sl;
    struct channel *c;
    struct channelList *cl;
    struct lookup * l;
    struct astList * al;
};

// Variable ed expr tokens
%token <d> NUMBER 
%token <string> NAME
%token <string> STRING

// Features
%token DEFINE READ SLEEP MACRO PRINT INPUT EXTENDS DELETE

// Fade/Delay/Loop tokens 
%token FADE DELAY IN SECONDS LOOP FROM TO 

// IF-ELSE tokens
%token IF THEN ELSE 

// Brackets and EOL tokens
%token O_COMMENT C_COMMENT O_BRACKET C_BRACKET O_ARRAY C_ARRAY EOL

// Utilities token
%token FIXTURES CLEAR SETCOLOR RESETCOLOR 

// Serial port tokens
%token CONNECT DISCONNECT

%right '='
%left '+' '-'
%left '*' '/'
%left '%' '^'

// Returns section */
%nonassoc <fn> CMP
%type <string> path sig
%type <a> expr assignment stmt loopStmt singleStmt ifStmt 
%type <al> stmtList exprList instructionsBlock elseStmt
%type <l> variable
%type <c> channel
%type <cl> channelList


%start glr
%%

glr: /* nothing */
    | glr expr EOL          { struct evaluated evalu = eval($2); freeEvaluated(&evalu); freeAst($2); }
    | glr stmt EOL          { struct evaluated evalu = eval($2); freeEvaluated(&evalu); freeStmt($2); }
    | glr preprocessing EOL { }
    | glr EOL               { }
;

expr:
    expr '+' expr       { $$ = newast(PLUS, $1, $3); }
    | expr '^' expr     { $$ = newast(CONCAT, $1, $3); }
    | expr '-' expr     { $$ = newast(MINUS, $1, $3); }
    | expr '*' expr     { $$ = newast(MUL, $1, $3); }
    | expr '/' expr     { $$ = newast(DIV, $1, $3); }
    | expr '%' expr     { $$ = newast(MOD, $1, $3); }
    | expr CMP expr     { $$ = newCompare($2, $1, $3); }
    | '(' expr ')'      { $$ = $2; }
    | NUMBER            { $$ = newnum($1); }
    | '-' expr          { $$ = newast(MINUS, newnum(0), $2); } 
    | variable          { $$ = (struct ast *) $1; }
    | variable '.' NAME { $$ = newGetChannelValue($1, $3); }
    | STRING            { $$ = newString($1); }
    | INPUT             { $$ = newInput(); }
;

stmt:
    assignment          { $$ = $1; }
    | loopStmt          { $$ = $1; }
    | ifStmt            { $$ = $1; }
    | SLEEP expr SECONDS { $$ = newSleep($2); } /* a sleep */
    | NAME '(' ')'      { $$ = newMacroCall($1); } /* to call a macro */
    | PRINT expr        { $$ = newPrint($2); }
;

preprocessing:
    define                         { }
    | MACRO NAME instructionsBlock { newMacroDefine($2, $3); } /* to define a macro */
    | delete                       { }
    | FIXTURES                     { printAllFixtures(); }
    | CLEAR                        { system("clear");  }
    | SETCOLOR NAME                { setColor($2); }
    | RESETCOLOR                   { printf("\033[0m"); }
    | CONNECT path                 { connectDmx($2); }
    | DISCONNECT path              { disconnectDmx($2); }
;

assignment:
    NAME variable '=' expr                              { $$ = newFixture($1, $2, $4); }
    | variable '=' expr                                 { $$ = newAsgn($1, $3); }
    | variable '=' O_BRACKET exprList C_BRACKET         { $$ = newCreateArray($1, $4); }
    | variable '.' NAME '=' expr                        { $$ = newSetChannelValue($1, $3, $5); }
    | variable '.' NAME '=' expr FADE IN expr SECONDS   { $$ = newFade($1, $3, $5, $8); }
    | variable '.' NAME '=' expr DELAY IN expr SECONDS  { $$ = newDelay($1, $3, $5, $8); }
;

loopStmt:
    LOOP NAME FROM expr TO expr instructionsBlock   { $$ = newLoop($2, $4, $6, $7); } 
    | LOOP NAME FROM expr TO expr singleStmt        { $$ = newLoop($2, $4, $6, astToAstList($7)); }
;

ifStmt:
    IF expr instructionsBlock EOL elseStmt  { $$ = newIf($2, $3, $5); }
    | IF expr singleStmt EOL elseStmt       { $$ = newIf($2, astToAstList($3), $5); }
;

variable:
    NAME                        { $$ = newLookup($1); } /* fixtureType oppure varName */
    | NAME O_ARRAY expr C_ARRAY { $$ = newLookupFromArray($1, $3); }
;

exprList: /* nothing */ { $$ = NULL; }
    | expr              { $$ = newAstList($1, NULL); }
    | expr ',' exprList { $$ = newAstList($1, $3); }
;

openBlock:
    O_BRACKET EOL   { }
    | EOL openBlock { }
;

stmtList:
    expr EOL            { $$ = newAstList($1, NULL); }
    | stmt EOL          { $$ = newAstList($1, NULL); }
    | stmt EOL stmtList { $$ = newAstList($1, $3); }
    | expr EOL stmtList { $$ = newAstList($1, $3); } 
    | EOL stmtList      { $$ = $2; }
;

singleStmt: expr     { $$ = $1; }
    | stmt           { $$ = $1; }
    | EOL singleStmt { $$ = $2; }
;

instructionsBlock:
    openBlock stmtList closeBlock {$$ = $2;}
;

elseStmt: /* nothing */         { $$ = NULL; }
    | ELSE instructionsBlock    { $$ = $2; }
    | ELSE singleStmt           { $$ = astToAstList($2); }
;

closeBlock:
    C_BRACKET           { }
    | EOL closeBlock    { }
;

define:
    DEFINE NAME openBlock channelList C_BRACKET                 { newFixtureType($2, $4, NULL);  }
    | DEFINE NAME EXTENDS NAME openBlock channelList C_BRACKET  { newFixtureType($2, $6, $4); }
;

delete:
    DELETE variable         { deleteVar($2->var); }
    | DELETE NAME '(' ')'   { deleteMacro($2); }
    | DELETE EOL variable   { deleteVar($3->var); }
    | DELETE EOL NAME '(' ')' { deleteMacro($3); }
;

path:
    NAME { } 
    | sig path {
                    $$ = malloc(sizeof(char) * (strlen($1) + strlen($2)));
                    $$ = strcat($$, $1);
                    $$ = strcat($$, $2);
                } 
    | NAME path {
                    $$ = malloc(sizeof(char) * (strlen($1) + strlen($2)));
                    $$ = strcat($$, $1);
                    $$ = strcat($$, $2);
                }
;

channelList: 
    channel EOL                 { $$ = newChannelList($1, NULL); }
    | EOL channelList           { $$ = $2; }
    | channel EOL channelList   { $$ = newChannelList($1, $3); }
;

channel:
    NUMBER NAME         { $$ = newChannel($1, $2); }
    | NUMBER EOL NAME   { $$ = newChannel($1, $3); }
;

sig:
    '/'     { $$ = "/"; }
    | '-'   { $$ = "-"; }
    | '.'   { $$ = "."; }
    | '\\'  { $$ = "\\"; }
    | '_'   { $$ = "_"; }
;

%%
