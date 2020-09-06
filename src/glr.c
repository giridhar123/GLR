#include "headers/parser.h"
#include "headers/dmx.h"
#include "headers/free.h"
#include "headers/sharedVariables.h"

int main (int argc, char ** argv)
{
    signal(SIGINT, freeEverything); 

    pthread_t serialPortThread, parser;

    FILE * source = stdin;

    if (argc > 1)
    {
        source = fopen(argv[1], "r");
    }    
    
    pthread_create(&parser, NULL, &startParser, source);
    pthread_create(&serialPortThread, NULL, &startDMX, NULL);

    //Join solo sul parser, se quest'ultimo termina, termina anche la serial port
    pthread_join(parser, NULL);

    freeEverything();
    return 0;
}
