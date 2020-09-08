#include "headers/astbuilder.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"
#include "headers/parser.h"

unsigned int varhash(char *var)
{
    //Funzione per fare l hash
    unsigned int hash = 0;
    unsigned c;

    //@ Per levare il warning mi ha fatto mettere questa sintassi ( (true ) ) per il while
    while ( (c = *var++) )
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
            var = vartab; /* try the next entry */
    }
    yyerror("symbol table overflow\n");
    abort(); /* tried them all, table is full */
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

    //se la variabile è già dichiarata
    if (fixture->fixtureType != NULL)
    {
        printf("Variabile già dichiarata\n");
        return 0;
    }

    int maxAddress = startAddress + getNumberOfChannels(fixtureType) - 1;

    for (int i = startAddress; i <= maxAddress; ++i)
    {
        if (dmxOccupied[i] != NULL)
        {
            printf("Indirizzo già occupato\n");
            return 0;
        }
    }

    for (int i = startAddress; i <= maxAddress; ++i)
        dmxOccupied[i] = fixture;

    //Setto la fixturetype della variabile e l'indirizzo della variabile con quelli trovati con la struct fixtureType
    fixture->varType = FIXTURE_VAR;
    fixture->fixtureType = fixtureType;
    fixture->intValue = startAddress;

    return 1;
}

void createFixtureArray(struct fixtureType * fixtureType, int startAddress, struct lookup * lookup)
{
    if (fixtureType == NULL)
    {
        printf("Il tipo non esiste\n");
        return;
    }

    if (lookup->var->array != NULL)
    {
        printf("Array già dichiarato\n");
        return;
    }

    int size = eval(lookup->index)->intVal;

    if (size <= 0)
    {
        printf("Dimensione non consentita\n");
        return;
    }

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
    for (int i = 1; i < size; ++i)
    {
        arrayList->next = malloc(sizeof(struct array));
        arrayList = arrayList->next;

        struct var * var = malloc(sizeof(struct var));
        createFixture(fixtureType, startAddress + (numberOfChannels * i), var);
        
        arrayList->index = i;
        arrayList->var = var;
    }

    printf("Array creato\n");
}

int getChannelAddress(struct fixtureType * fixtureType, char * channelName)
{
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
    //Restituisce il numero di canali di una fixtureType
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

int addFixtureType(struct fixtureType * fixtureType)
{
    int index = varhash(fixtureType->name)%NHASH;
    struct fixtureType *ft = typetab[index];
    int scount = NHASH;		

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

    return 0;

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