#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void yyerror(const char *s);    
void* startDMX(void * params);
void* startParser(void * params);
    
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
    if(!yyparse())
        printf("\nParsing complete\n");
    else
        printf("\nParsing failed\n");
}