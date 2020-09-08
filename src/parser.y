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
%token <string> STRING

%token <fn> FUNC
%token IF
%token THEN 
%token ELSE
%token DO
%token SLEEP
%token MACRO
%token PRINT
%token INPUT

%token O_COMMENT
%token C_COMMENT

%token DELETE 
%nonassoc <fn> CMP
%left '+' '-'
%left '*' '/'

%type <a> expr channel channelList define assignment stmt loopStmt sleep macroDefine strutturaifsingle ifStmt macroCall 
%type <al> stmtList exprList instructionsBlock
%type <l> variable


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
;

delete:
    DELETE variable { deleteVar($2->var); }
    | DELETE NAME '(' ')' { deleteMacro($2); }
    | DELETE EOL variable { deleteVar($3->var); }
    | DELETE EOL NAME '(' ')' { deleteMacro($3); }
;


define:
    DEFINE NAME EOL O_BRACKET channelList C_BRACKET { $$ = newFixtureType($2, $5);  }
    | DEFINE NAME EOL O_BRACKET EOL channelList C_BRACKET { $$ = newFixtureType($2, $6); }
    | DEFINE NAME channelList { $$ = newFixtureType($2,$3); }
    | DEFINE NAME EOL channelList { $$ = newFixtureType($2,$4); }
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
    LOOP NAME FROM expr TO expr instructionsBlock  { $$ = newLoop($2, $4, $6, $7); } 
    | LOOP NAME FROM expr TO expr EOL instructionsBlock { $$ = newLoop($2, $4, $6, $8); } 
    | LOOP NAME FROM expr TO expr stmt { $$ = newLoop($2, $4, $6, AstToAstList($7)); }
    | LOOP NAME FROM expr TO expr expr { $$ = newLoop($2, $4, $6, AstToAstList($7)); }
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

instructionsBlock:
    O_BRACKET stmtList C_BRACKET {$$ = $2;}
    | O_BRACKET EOL stmtList C_BRACKET {$$ = $3;}
%%
