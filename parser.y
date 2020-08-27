%{
    #include <stdio.h>
%}
%define parse.error verbose /* per vedere l'intero report dalla yyerror */

/* yylval diventa una union, in questo modo posso passare sia int sia stringhe tra flex e bison */
%union {
    int num;
    char * str;
};

%token <str> ANYTHING
%token ITALIC_OPEN
%token NEW_LINE

%type <str> expr
%type <str> tag
%%

test:
    | test expr { fprintf(stdout, "hey"); }
;

expr:
    ANYTHING { $$ = $1; }
    | tag { $$ = $1; }
;

tag:
    ITALIC_OPEN { $$ = "<i>"; }
;

%%
