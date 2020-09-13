#include "headers/astbuilder.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"
#include "headers/parser.h"
#include "headers/dmx.h"

unsigned int varhash(char *var)
{
    //Funzione per fare l hash
    unsigned int hash = 0;
    unsigned c;

    while ( (c = *var++) )
    hash = hash*9 ^ c;

    return hash;
}

struct var * lookupVar(char * name)
{
    // La funzione lookupVar controlla all'interno dell'array vartab se c'è o meno il nome di una variabile.
    // Se la trova la restituisce
    // Se NON la trova inizializza una nuova variabile 

    // Recupero la variabile in base alla funzione di hash 
    struct var *var = &vartab[varhash(name)%NHASH];
    int scount = NHASH;		/* contatore, lo inizializzo alla dimensione massima possibile (9997) */

    // Inizio ciclo, finisco appena lo scorro tutto oppure trovo un valore in tabella e ritorno il valore trovato
    while(--scount >= 0) 
    {
        // Se la variabile trovata ha lo stesso nome di quella richiesta, la restituisce
        if (var->name && !strcmp(var->name, name))
            return var;

        // Se la variabile non contiene alcun nome (praticamente, non è stata allocata) la inizializza
        if(!var->name) 
        {
            // Inizializzo una nuova variabile 
            var->nodetype = VARIABLE;
            var->varType = NONE;
            var->name = strdup(name);            
            var->intValue = 0;
            var->doubleValue = 0;
            var->stringValue = NULL;
            var->fixtureType = NULL;
            var->array = NULL;
            return var;
        }

        //Altrimenti va avanti e se va fuori dall'array, riparte dall'inizio
        if(++var >= vartab+NHASH)
            var = vartab;
    }
    yyerror("symbol table overflow\n");
    abort(); // La tabella è piena e non ho trovato nessuna variabile
}
struct fixtureType * lookupFixtureType(char * name)
{
    // Analoga alla funzione "lookupVar"

    // La funzione mi ritorna una specifica fixture type se essa è presente all'interno della tabella typetab. 
    // Altrimenti ritorna null
    int index = varhash(name)%NHASH;
    struct fixtureType *ft = typetab[index];
    int scount = NHASH;		

    while(--scount >= 0)
    {
        if (ft == NULL)
        {
            ft = malloc(sizeof(struct fixtureType));
            ft->name = strdup(name);
            ft->cl = NULL;
            typetab[index] = ft;
            return ft;
        }

        if (ft->name && !strcmp(ft->name, name))
            return ft;

        if(++ft >= *typetab+NHASH)
            ft = *typetab;

        ++index;
        index = index % NHASH;
    }

    return NULL;

    yyerror("symbol table overflow\n");
    abort(); 
}

struct macro * lookupMacro(char * name)
{ 
    // La funzione mi ritorna una specifica macro se essa è presente all'interno della tabella macrotab. 
    // Altrimenti ritorna null    
    int index = varhash(name)%NHASH;
    struct macro *m = macrotab[index];
    int scount = NHASH;		

    while(--scount >= 0)
    {
        if (m == NULL)
        {
            m = malloc(sizeof(struct macro));
            m->macroName = strdup(name);
            m->instruction = NULL;
            macrotab[index] = m;
            return m;
        }

        if (m->macroName && !strcmp(m->macroName, name))
            return m;

        if(++m >= *macrotab+NHASH)
            m = *macrotab;

        ++index;
        index = index % NHASH;
    }

    return NULL;

    yyerror("symbol table overflow\n");
    abort(); 
}

void newMacroDefine(char * name, struct astList * instructions)
{
    struct macro * m = lookupMacro(name);

    m->instruction = instructions;

    printf("Ho creato una macro di nome: %s\n",  m->macroName);
}

int createFixture(struct fixtureType * fixtureType, int startAddress, struct var * fixture)
{
    // Se non è presente il tipo di fixture da creare stampa il messaggio d'errore e non fare nulla
    if (fixtureType->cl == NULL)
    {
        printf("Il tipo non esiste!\n");
        return 0;
    }

    // Se l'indirizzo non è corretto stampa il messaggio d'errore e non fare nulla
    if (startAddress < 1 || startAddress > 512)
    {
        printf("Indirizzo non valido\n");
        return 0;
    }

    // Se la variabile è già dichiarata stampa il messaggio d'errore e non fare nulla
    if (fixture->fixtureType != NULL)
    {
        printf("Variabile già dichiarata\n");
        return 0;
    }
    
    // Il numero massimo è ottenuto da quello di partenza + il numero dei canali di quella fixturetype - 1 
    int maxAddress = startAddress + getNumberOfChannels(fixtureType) - 1;

    // Verifico se sono liberi gli spazi che mi servono per creare una nuova fixtures
    // in caso stampo il messaggio d'errore e non faccio nulla
    for (int i = startAddress; i <= maxAddress; ++i)
    {
        if (dmxOccupied[i] != NULL)
        {
            printf("Indirizzo già occupato\n");
            return 0;
        }
    }

    // Una volta che ho superato tutti i controlli, occupo il dmx con la variabile fixture
    for (int i = startAddress; i <= maxAddress; ++i)
        dmxOccupied[i] = fixture;

    // Viene "configurata" la variabile impostando il tipo di variabile, il tipo di fixture associato e l'indirizzo di partenza
    fixture->varType = FIXTURE_VAR;
    fixture->fixtureType = fixtureType;
    fixture->intValue = startAddress;

    // Ritorno esito positivo
    return 1;
}

// Questa funzione crea un array di fixture facendo in modo che l'indirizzo di partenza della fixture in
// cella (i+1)-esima sia adiacente all'indirizzo finale della fixture in cella i-esima
void createFixtureArray(struct fixtureType * fixtureType, int startAddress, struct lookup * lookup)
{
    // Se non è presente lookupFixtureType stampa il messaggio d'errore e non fare nulla
    if (fixtureType->cl == NULL)
    {
        printf("Il tipo non esiste\n");
        return;
    }

    // Se l'attributo array è diverso da null, significa che è già stato dichiarato.
    if (lookup->var->array != NULL)
    {
        printf("Array già dichiarato\n");
        return;
    }

    // Valutiamo la dimensione 
    int size = eval(lookup->index).intVal;

    // Se la dimensione che si vuole attribuire all'array è negativa ritorno un messaggio d'errore
    if (size <= 0)
    {
        printf("Dimensione non consentita\n");
        return;
    }

    // Inizializzo la struct array
    struct var * variable = lookup->var;
    variable->varType = ARRAY_VAR;
    variable->fixtureType = fixtureType;
    variable->intValue = size;
    
    variable->array = malloc(sizeof(struct array));
    struct array * arrayList = variable->array;

    struct var * var = malloc(sizeof(struct var));
    createFixture(fixtureType, startAddress, var);
    
    arrayList->index = 0;
    arrayList->var = var;

    int numberOfChannels = getNumberOfChannels(fixtureType);
    for (int i = 1; i < size; ++i) // Viene creato l'intero array
    {
        arrayList->next = malloc(sizeof(struct array));
        arrayList = arrayList->next;

        struct var * var = malloc(sizeof(struct var));
        createFixture(fixtureType, startAddress + (numberOfChannels * i), var);
        
        arrayList->index = i;
        arrayList->var = var;
    }
}

int getChannelAddress(struct fixtureType * fixtureType, char * channelName)
{
    // La funzione mi ritorna l'indirizzo del canale di una fixtures type esistente
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
    // La funzioen Restituisce il numero di canali di una fixture type esistente
    int count = 0;

    struct channelList * channelList = fixtureType->cl;
    while (channelList != NULL)
    {
        ++count;
        channelList = channelList->next;
    }

    return count;
}

void newFixtureType(char * name, struct channelList * cl, char * parentName)
{
    struct fixtureType * ft = lookupFixtureType(name);

    if (parentName == NULL)
        ft->cl = cl;
    else
    {
        struct fixtureType * parent = lookupFixtureType(parentName);

        if(parent->cl == NULL)
        {
            printf("Il tipo da estendere non esiste.\n");
            return;
        }
        
        ft->cl = cl;

        int tmpArray[513];
        for (int i = 0; i < 513; ++i)
            tmpArray[i] = 0;

        struct channelList * tmpChannelList = ft->cl;

        while (tmpChannelList->next != NULL)
        {
            tmpArray[tmpChannelList->channel->address] = 1;
            tmpChannelList = tmpChannelList->next;
        }

        struct channelList * parentList = parent->cl;
        while (parentList != NULL)
        {
            if (tmpArray[parentList->channel->address] == 0)
            {
                tmpChannelList->next = malloc(sizeof(struct channelList));
                tmpChannelList = tmpChannelList->next;
                tmpChannelList->channel = malloc(sizeof(struct channel));
                tmpChannelList->channel->address = parentList->channel->address;
                tmpChannelList->channel->name = strdup(parentList->channel->name);
            }
            parentList = parentList->next;
        }
    }
}

void printAllFixtures()
{  
    // La funzione mi permette di stampare tutte le features.
    // Scorro l'intero array dmx e stampo tutte le fixtures ,con relativo indirizzo, una sola volta
    for (int i = 1 ; i < 513 ; i++)
    {
        if (dmxOccupied[i] != NULL)
        {
            struct var * fixture = dmxOccupied[i];
            int startAddress = fixture->intValue;
            int maxAddress = startAddress + getNumberOfChannels(fixture->fixtureType) - 1;
          
            printf("%s: dal canale %d al canale: %d\n", dmxOccupied[i]->name, startAddress, maxAddress);
            i = maxAddress + 1;
        }
    }
}

void setColor(char * color)
{
    // La funzione permette, passato un colore all'interno della lista, di colorare la console
    if(strcmp(color,"red") == 0) printf("\033[0;31m");
    if(strcmp(color,"green") == 0) printf("\033[0;32m");
    if(strcmp(color,"blue") == 0) printf("\033[0;34m");
    if(strcmp(color,"cyan") == 0) printf("\033[0;36m");
}

void connectDmx(char * port)
{   
    // La funzione crea un thread per gestire una porta seriale
    pthread_t serialPortThread;
    pthread_create(&serialPortThread, NULL, &startDmx, port);
}

void disconnectDmx(char * port)
{
    //Cerco in che indice è il mio thread in base al nome della porta seriale
    for (int i = 0 ; i < 10 ; i++)
    {
        if (strcmp(DmxName[i], port) == 0)
        {
            DmxOpen[i] = 0;
            break;
        }
    }
}

/* Evaluated functions #3 */
/* Servono fondamentalmente per creare le struct di tipo evaluated in base a un double, int o una stringa */
struct evaluated getEvaluatedFromDouble(double value)
{
    struct evaluated evaluated;

    evaluated.type = DOUBLE_VAR;
    evaluated.doubleVal = value;
    evaluated.intVal = (int) floor(value);
    evaluated.stringVal = NULL;

    return evaluated;
}

struct evaluated getEvaluatedFromString(char * value)
{
    struct evaluated evaluated;

    evaluated.type = STRING_VAR;
    evaluated.doubleVal = strlen(value);
    evaluated.intVal = strlen(value);
    evaluated.stringVal = malloc(sizeof(char) * strlen(value));
    evaluated.stringVal = strcpy(evaluated.stringVal, value);

    return evaluated;
}

struct evaluated getEvaluatedFromInt(int value)
{
    struct evaluated evaluated;

    evaluated.type = INT_VAR;
    evaluated.doubleVal = (double) value;
    evaluated.intVal = value;
    evaluated.stringVal = NULL;

    return evaluated;
}