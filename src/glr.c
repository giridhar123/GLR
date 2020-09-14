#include "headers/parser.h"
#include "headers/dmx.h"
#include "headers/free.h"
#include "headers/sharedVariables.h"

int main (int argc, char ** argv)
{
    signal(SIGINT, freeEverything);

    FILE * source = stdin;

    if (argc > 1)
        source = fopen(argv[1], "r");
    
    // Iniziliazzazione del vettore dmxUniverse il quale viene inviato tramite la porta seriale ai dispositivi
    for (int i = 0; i < 513; ++i)
        dmxUniverse[i] = 0;

    for (int i = 0; i < N_THREADS; ++i)
    {
        DmxOpen[i] = 0;
        DmxName[i] = NULL;
    }

    startParser(source);

    freeEverything();
    return 0;
}
