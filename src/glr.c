#include "headers/parser.h"
#include "headers/dmx.h"
#include "headers/free.h"
#include "headers/sharedVariables.h"

int main (int argc, char ** argv)
{
    ThreadCounter = 0 ; //inizializzo a 0 @WIP
    signal(SIGINT, freeEverything); 

    pthread_t parser;

    FILE * source = stdin;

    if (argc > 1)
    {
        source = fopen(argv[1], "r");
    }    
    
    // Iniziliazzazione del vettore dmxUniverse il quale viene inviato tramite la porta seriale ai dispositivi
    for (int i = 0; i < 513; ++i)
        dmxUniverse[i] = 0;

    for (int i = 0; i < N_THREADS; ++i)
    {
        DmxOpen[i] = 0;
        DmxName[i] = NULL;
    }

    pthread_create(&parser, NULL, &startParser, source);
    // pthread_create(&serialPortThread, NULL, &startDMX, NULL); //devo vedere se la connect funziona

    // Join solo sul parser, se quest'ultimo termina, termina anche la serial port
    pthread_join(parser, NULL);

    freeEverything();
    return 0;
}
