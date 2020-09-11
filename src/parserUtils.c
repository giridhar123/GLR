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
    // La funzione lookupVar controlla all'interno della tabella vartab se c'è o meno il nome di una variabile.
     // Se la trova la ritorna
     // Se NON la trova inizializza una nuova variabile 

    // Inizializzo il puntatore all'indirizzo di memoria della vartab alla posizione che viene dall'hash%nhash
    struct var *var = &vartab[varhash(name)%NHASH];
    int scount = NHASH;		/* contatore, lo inizializzo alla dimensione massima possibile (9997) */

    // Inizio ciclo, finisco appena lo scorro tutto oppure trovo un valore in tabella e ritorno il valore trovato
    while(--scount >= 0) 
    {
        // Se trovo la variabile inserita come parametro della funzione all'interno della tabella, la ritorno.
        if (var->name && !strcmp(var->name, name))
        { 
            return var;
        }

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

        if(++var >= vartab+NHASH)
            var = vartab; // Provo la prossima entrata 
    }
    yyerror("symbol table overflow\n");
    abort(); // La tabella è piena e non ho trovato nessuna variabile
}


int createFixture(struct fixtureType * fixtureType, int startAddress, struct var * fixture)
{
    // Se non è presente lookupFixtureType stampa il messaggio d'errore e non fare nulla
    if (fixtureType == NULL)
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

    // Il numero massimo degli address è partenza + il numero dei canali di quella fixturetype - 1 
    int maxAddress = startAddress + getNumberOfChannels(fixtureType) - 1;

    // Verifico se gli spazi che mi servono per creare una nuova fixtures, in caso stampo il messaggio d'errore e non faccio nulla
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

    // Setto la struct fixture dandogli la tipologia e l'indirizzo di partenza 
    fixture->varType = FIXTURE_VAR;
    fixture->fixtureType = fixtureType;
    fixture->intValue = startAddress;

    // Ritorno esito positivo
    return 1;
}

void createFixtureArray(struct fixtureType * fixtureType, int startAddress, struct lookup * lookup)
{
    // Se non è presente lookupFixtureType stampa il messaggio d'errore e non fare nulla
    if (fixtureType == NULL)
    {
        printf("Il tipo non esiste\n");
        return;
    }

        // Se l'attributo array non è diverso da null, significa che è già stato dichiarato.
    if (lookup->var->array != NULL)
    {
        printf("Array già dichiarato\n");
        return;
    }

    // Valutiamo la dimensione 
    int size = eval(lookup->index)->intVal;

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
    for (int i = 1; i < size; ++i) // Scorro l'intero array
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
struct fixtureType * lookupFixtureType(char * name)
{
    // La funzione mi ritorna una specifica fixture type se essa è presente all'interno della tabella typetab. 
     // Altrimenti ritorna null
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

int addFixtureType(struct fixtureType * fixtureType)
{
    // Se la fixture type inserita non ha un parente ( NON è un extends )
    if(fixtureType->parentName == NULL)
    {
        int index = varhash(fixtureType->name)%NHASH;
        struct fixtureType *ft = typetab[index];
        int scount = NHASH;

    /* @SP */
        while(--scount >= 0)
        {
            if (ft == NULL) {
                typetab[index] = fixtureType;
                return 1;
            }

            if(++ft >= *typetab+NHASH)
                ft = *typetab;

            ++index;
            index = index % NHASH;
        }
    }
    else
    {
        struct fixtureType * parent = lookupFixtureType(fixtureType->parentName);

        if(parent != NULL)
        {
            struct channelList * tmp = fixtureType->cl;

            while (tmp->next != NULL)
            {
                tmp = tmp->next;
            }

            tmp->next = parent->cl;

            fixtureType->parentName = NULL;

            addFixtureType(fixtureType);
            
            return 1;
        }
        else
        {
            yyerror("Parent FixturType non found");
        }
        
    }
    
   

    return 0;

    yyerror("symbol table overflow\n");
    abort(); 
}
struct macro * lookupMacro(char * name)
{ 
   // La funzione mi ritorna una specifica macro se essa è presente all'interno della tabella macrotab. 
     // Altrimenti ritorna null    
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


/* Evaluated functions #3 */
/* Servono fondamentalmente per fare l'evaluate di un double,int o una stringa */
struct evaluated * getEvaluatedFromDouble(double value)
{
    struct evaluated * evaluated = malloc(sizeof(struct evaluated));

    evaluated->type = DOUBLE_VAR;
    evaluated->doubleVal = value;
    evaluated->intVal = (int) floor(value);
    evaluated->stringVal = malloc(sizeof(value));
    snprintf(evaluated->stringVal, 8, "%2.4f", value);

    return evaluated;
}

struct evaluated * getEvaluatedFromString(char * value)
{
    struct evaluated * evaluated = malloc(sizeof(struct evaluated));

    evaluated->type = STRING_VAR;
    evaluated->doubleVal = strlen(value);
    evaluated->intVal = strlen(value);
    evaluated->stringVal = strdup(value);

    return evaluated;
}

struct evaluated * getEvaluatedFromInt(int value)
{
    struct evaluated * evaluated = malloc(sizeof(struct evaluated));

    evaluated->type = INT_VAR;
    evaluated->doubleVal = (double) value;
    evaluated->intVal = value;
    evaluated->stringVal = malloc(sizeof(value));
    sprintf(evaluated->stringVal, "%d", value);

    return evaluated;
}

void PrintAllFixtures()
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

void SetColor(char * color)
{
        // La funzione permette, passato un colore all'interno della lista, di colorare la console
        if(strcmp(color,"red") == 0) printf("\033[0;31m");
        if(strcmp(color,"green") == 0) printf("\033[0;32m");
        if(strcmp(color,"blue") == 0) printf("\033[0;34m");
        if(strcmp(color,"cyan") == 0) printf("\033[0;36m");

}

void ConnectDmx(char * port)
{   
    // La funzione crea un thread per gestire una porta seriale
    pthread_t serialPortThread;
    pthread_create(&serialPortThread, NULL, &startDMX, port);
}

void DisconnectDmx(char * port)
{
    //Cerco in che indice è il mio thread
    for (int i = 0 ; i < 10 ; i++)
    {
        if (strcmp(DmxName[i], port) == 0)
        {
            DmxOpen[i] = 0;
            break;
        }
    }
}