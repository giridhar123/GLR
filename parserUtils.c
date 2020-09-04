#include "headers/astbuilder.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"
#include "headers/parser.h"

unsigned int varhash(char *var)
{
    //Funzione per fare l hash
    unsigned int hash = 0;
    unsigned c;

    while (c = *var++)
    hash = hash*9 ^ c;

    return hash;
}

struct var * lookupVar(char * name)
{
    //La funzione lookupVar controlla all'interno della tabella vartab se c'è o meno il nome di una variabile.
     //Se la trova la ritorna
     //Se NON la trova inizializza una nuova variabile 

    //inizializzo il puntatore all'indirizzo di memoria della vartab alla posizione che viene dall'hash%nhash
    struct var *var = &vartab[varhash(name)%NHASH];
    int scount = NHASH;		/* contatore, lo inizializzo alla dimensione massima possibile (9997) */

    //Inizio ciclo, finisco appena lo scorro tutto oppure trovo un valore in tabella e ritorno il valore trovato
    while(--scount >= 0) 
     
    {
        //Se trovo la variabile inserita come parametro della funzione all'interno della tabella, la ritorno.
        if (var->name && !strcmp(var->name, name))
        { 
            return var;
        }

        if(!var->name) 
        {
            /* inizializzo una nuova variabile */
            var->nodetype = VARIABLE;
            var->name = strdup(name);
            var->value = 0;
            var->func = NULL;
            var->fixtureType = NULL;
            var->array = NULL;
            return var;
        }

        if(++var >= vartab+NHASH)
            var = vartab; /* try the next entry */
    }
    yyerror("symbol table overflow\n");
    abort(); /* tried them all, table is full */
}



void parseFile(char * fileName) 
{
    //Apre il file in lettura e starta il parsing tramite il file.
    FILE * file = fopen(fileName, "r");

    printf("Opening new file: %s\n", fileName);
    struct fileList * next = malloc(sizeof(struct fileList));
    next->this = file;
    next->next = fileList;
    fileList = next;

    startParser(file);
}


int createFixture(struct fixtureType * fixtureType, int startAddress, struct var * fixture)
{
    //Se non è presente lookupFixtureType ritorna null
    if (fixtureType == NULL)
    {
        //
        printf("Il tipo non esiste!\n");
        return 0;
    }

    //Se l'indirizzo non è corretto
    if (startAddress < 1 || startAddress > 512)
    {
        printf("Indirizzo non valido\n");
        return 0;
    }

    if (dmxOccupied[startAddress] != NULL)
    {
        printf("Indirizzo già occupato\n");
        return 0;
    }

    //se la variabile è già dichiarata
    if (fixture->fixtureType != NULL)
    {
        printf("Variabile già dichiarata\n");
        return 0;
    }

    int maxAddress = startAddress + getNumberOfChannels(fixtureType) - 1;

    for (int i = startAddress; i < maxAddress; ++i)
        dmxOccupied[i] = fixture;

    //Setto la fixturetype della variabile e l'indirizzo della variabile con quelli trovati con la struct fixtureType
    fixture->fixtureType = fixtureType;
    fixture->value = startAddress;

    return 1;
}





int getChannelAddress(struct fixtureType * fixtureType, char * channelName)
{
    if (fixtureType == NULL)
        printf("NUUUL\n");
    //Cerco l'indirizzo del canale in base al nome
    int address = -1;

    struct channelList * channelList = fixtureType->cl;
    while (channelList != NULL)
    {
        if (!strcmp(channelList->channel->name, channelName))
        {
            address = channelList->channel->address;
            break;
        }
        channelList = channelList->next;
    }

    return address;
}

int getNumberOfChannels(struct fixtureType * fixtureType)
{
    //Cerco l'indirizzo del canale in base al nome
    int count = 0;

    struct channelList * channelList = fixtureType->cl;
    while (channelList != NULL)
    {
        ++count;
        channelList = channelList->next;
    }

    return count;
}

struct fixtureType * lookupFixtureType(char * name)
{
    struct fixtureType *ft = typetab[varhash(name)%NHASH];
    int scount = NHASH;		

    while(--scount >= 0)
    {
        if (ft == NULL)
            return NULL;

        if (ft->name && !strcmp(ft->name, name))
            return ft;

        if(++ft >= *typetab+NHASH)
            ft = *typetab;
    }

    return NULL;

    yyerror("symbol table overflow\n");
    abort(); 
}

struct macro * lookupMacro(char * name)
{
    struct macro *m = macrotab[varhash(name)%NHASH];
    int scount = NHASH;		

    while(--scount >= 0)
    {
        if (m == NULL)
            return NULL;

        if (m->macroName && !strcmp(m->macroName, name))
            return m;

        if(++m >= *macrotab+NHASH)
            m = *macrotab;
    }

    return NULL;

    yyerror("symbol table overflow\n");
    abort(); 
}

