%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "headers/parser.h"
    #include "headers/astbuilder.h"
    #include "headers/parserUtils.h"
    #include "headers/sharedVariables.h"
    
    //per eliminare i warning
    int yylex(void);
    void freeAst();
    int yyparse();
    void deleteMacro();
    void deleteVar();
%}
%define parse.error verbose /* per vedere l'intero report dalla yyerror */

/* yylval diventa una union, in questo modo posso passare sia int sia stringhe tra flex e bison */
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

/* Variable ed expr tokens */
%token <d> NUMBER 
%token <string> NAME
%token <string> STRING

//%token TAB
//%token DO

/* altro */ 
%token DEFINE READ SLEEP MACRO PRINT INPUT EXTENDS DELETE
/* Fade/Delay/Loop tokens */
%token FADE DELAY IN SECONDS LOOP FROM TO 

//%token <fn> FUNC
/* IF-ELSE tokens */ 
%token IF THEN ELSE 

/* Brackets and EOL tokens */
%token O_COMMENT C_COMMENT O_BRACKET C_BRACKET O_ARRAY C_ARRAY EOL

/* Utilities token */ 
%token FIXTURES CLEAR SETCOLOR RESETCOLOR 

/* Serial port tokens */
%token CONNECT DISCONNECT


%left '+' '-'
%left '*' '/'

/* Returns section */
%nonassoc <fn> CMP
%type <string> path sig
%type <a> expr assignment stmt loopStmt sleep macroDefine signleStmt ifStmt macroCall 
%type <al> stmtList exprList instructionsBlock elseStmt
%type <l> variable
%type <c> channel
%type <cl> channelList


%start glr
%%

glr: /* nothing */
    | glr expr EOL { eval($2); freeAst($2); }
    | glr stmt EOL { eval($2); }
    | glr preprocessing EOL { }
    | glr EOL { }
;

preprocessing:
    define {}
    | macroDefine {}
    | delete {}
    | FIXTURES { PrintAllFixtures(); }
    | CLEAR {  system("clear");  }
    | SETCOLOR NAME { SetColor($2); }
    | RESETCOLOR { printf("\033[0m"); }
    | CONNECT path { ConnectDmx($2); }
    | DISCONNECT path {DisconnectDmx($2); }
;
path:

     NAME { } 
    | NAME sig path {
                        $$ = malloc(sizeof(char) * (strlen($1) + 1 + strlen($3) + 2));
                        $$ = strcat($$, $1);
                        $$ = strcat($$, (char *)$2);
                        $$ = strcat($$, $3);
                    } 
;
sig:
    '/' { $$ = "/"; }
    | '-' { $$ = "-"; }
    | '.' { $$ = "."; }
    | '\\' { $$ = "\\"; }
    | '_' { $$ = "_"; }

;
delete:
    DELETE variable { deleteVar($2->var); }
    | DELETE NAME '(' ')' { deleteMacro($2); }
    | DELETE EOL variable { deleteVar($3->var); }
    | DELETE EOL NAME '(' ')' { deleteMacro($3); }
;

define:
    DEFINE NAME openBlock channelList C_BRACKET { newFixtureType($2, $4, NULL);  }
    | DEFINE NAME EXTENDS NAME openBlock channelList C_BRACKET { newFixtureType($2, $6, $4); }
;

channelList: 
    channel EOL { $$ = newChannelList($1, NULL); }
    | channel EOL channelList { $$ = newChannelList($1, $3); }
;

channel:
    NUMBER NAME { $$ = newChannel($1, $2); }
    | NUMBER EOL NAME { $$ = newChannel($1, $3); }
;

stmt:
    assignment { $$ = $1; }
    | loopStmt { $$ = $1; }
    | ifStmt { $$ = $1; }
    | sleep {$$ = $1; }
    | macroCall {$$ = $1;}
    | PRINT expr { $$ = newPrint($2); }
;

assignment:
    NAME variable '=' expr { $$ = newFixture($1, $2, $4); }
    | variable '=' expr { $$ = newAsgn($1, $3); }
    | variable '=' O_BRACKET exprList C_BRACKET { $$ = newCreateArray($1, $4); }
    | variable '.' NAME '=' expr { $$ = newSetChannelValue($1, $3, $5); }
    | variable '.' NAME '=' expr FADE IN expr SECONDS { $$ = newFade($1, $3, $5, $8); }
    | variable '.' NAME '=' expr DELAY IN expr SECONDS { $$ = newDelay($1, $3, $5, $8); }
;

variable:
    NAME { $$ = newLookup($1); } /* fixtureType oppure varName */
    | NAME O_ARRAY expr C_ARRAY { $$ = newLookupFromArray($1, $3); }
;

loopStmt:
    LOOP NAME FROM expr TO expr openBlock stmtList C_BRACKET  { $$ = newLoop($2, $4, $6, $8); } 
    | LOOP NAME FROM expr TO expr signleStmt { $$ = newLoop($2, $4, $6, AstToAstList($7)); }
;

ifStmt:
    IF expr instructionsBlock elseStmt { $$ = newIf($2, $3, $4); }
    | IF expr signleStmt EOL elseStmt {$$ = newIf($2, AstToAstList($3), $5); }
;

openBlock:
    O_BRACKET { }
    | EOL O_BRACKET EOL { }
    | O_BRACKET EOL { }
;

closeBlock:
    C_BRACKET
    | C_BRACKET EOL { }
;

instructionsBlock:
    openBlock stmtList closeBlock {$$ = $2;}
;

elseStmt: /* nothing */ { $$ = NULL; }
    | ELSE instructionsBlock { $$ = $2; }
    | ELSE signleStmt { $$ = AstToAstList($2); }
;

signleStmt: expr { $$ = $1; }
    | stmt { $$ = $1; }
    | EOL signleStmt { $$ = $2; }
;

sleep:
    SLEEP expr SECONDS { $$ = newSleep($2); }
;

macroDefine:
    MACRO NAME EOL O_BRACKET EOL stmtList C_BRACKET { newMacroDefine($2, $6); }
;

macroCall:
    NAME '(' ')' { $$ = newMacroCall($1);}
;

stmtList:
    stmt EOL { $$ = newAstList($1, NULL); }
    | expr EOL { $$ = newAstList($1, NULL); }
    | stmt EOL stmtList { $$ = newAstList($1, $3); }
    | expr EOL stmtList { $$ = newAstList($1, $3); } 
;

expr:
    expr '+' expr { $$ = newast(PLUS, $1, $3); }
    | expr '^' expr { $$ = newast(CONCAT, $1, $3); }
    | expr '-' expr { $$ = newast(MINUS, $1, $3); }
    | expr '*' expr { $$ = newast(MUL, $1, $3); }
    | expr '/' expr { $$ = newast(DIV, $1, $3); }
    | expr '%' expr { $$ = newast(MOD, $1, $3); }
    | expr CMP expr { $$ = newCompare($2, $1, $3); }
    | NUMBER { $$ = newnum($1); }
    | variable { $$ = (struct ast *) $1; }
    | variable '.' NAME { $$ = newGetChannelValue($1, $3); }
    | STRING { $$ = newString($1); }
    | INPUT { $$ = newInput(); }
;

exprList:
    expr { $$ = newAstList($1, NULL); }
    | expr ',' exprList { $$ = newAstList($1, $3); }
;

%%
