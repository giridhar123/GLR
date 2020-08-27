%{
    #include <stdio.h>
    #include <pthread.h>
    #include <unistd.h>

    extern FILE *yyin;
    void yyerror(const char *s);    
    void* startDMX(void * params);
    void* startParser(void * params);
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

int main (int argc, char ** argv)
{ 
    pthread_t serialPortThread, parser;

    pthread_create(&serialPortThread, NULL, &startDMX, NULL);
    pthread_create(&parser, NULL, &startParser, NULL);
    
    //Join solo sul parser, se quest'ultimo termina, termina anche la serial port
    pthread_join(parser, NULL);

    return 0;
}

void yyerror(const char *s)
{
	printf("ERROR: %s\n", s);
}

void* startDMX(void * params)
{
    for (int i = 0; i < 5; ++i)
    {
        fprintf(stdout, "Thread: %d\n", i);
        sleep(2);
    }

    return NULL;
}

void* startParser(void * params)
{
    yyin = stdin;
    if(!yyparse())
        printf("\nParsing complete\n");
    else
        printf("\nParsing failed\n");
}
