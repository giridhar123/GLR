#include "headers/astbuilder.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"
#include "headers/evalFunctions.h"


void* fadeEval(void * params)
{
    struct fade * fadeStruct = (struct fade *)params;

    struct var * fixture = fadeStruct->fixture;

    int channel = getChannelAddress(fixture->fixtureType, fadeStruct->channelName);
    if (channel == -1)
        return NULL;

    channel += fixture->value - 1;

    unsigned char currentValue = dmxUniverse[channel];
    
    int value = (int) eval(fadeStruct->value);
    double difference = value - currentValue;

    int step = difference > 0 ? 1 : -1;
    int time = fabs((eval(fadeStruct->time) * 1000 * 1000) / difference);
    
    while (dmxUniverse[channel] != value)
    {
        dmxUniverse[channel] = dmxUniverse[channel] + step;
        usleep(time);
    }

    return NULL;
}

void * delayEval(void * params)
{
    struct fade * delayStruct = (struct fade *)params;

    struct var * fixture = delayStruct->fixture;
    struct fixtureType * fixtureType = fixture->fixtureType;

    int channel = getChannelAddress(fixture->fixtureType, delayStruct->channelName);

    if (channel == -1)
        return NULL;

    channel += fixture->value - 1;
    int time = (int) eval(delayStruct->time);

    usleep(time * 1000 * 1000);
    dmxUniverse[channel] = eval(delayStruct->value);

    return NULL;
}

void sleepEval(struct sleep * s)
{
    double seconds = eval(s->seconds);
    int milliseconds = 1000 * seconds;
    usleep(milliseconds * 1000);
    fflush(stdout);
    printf("Ho dormito %d \n", milliseconds);
}

void setChannelValueEval(struct setChannelValue * setChannelValue)
{
    //La funzione setChannelValueEval fa l'evaluate del canale

    //Se non è presente lookupFixtureType ritorna null
    if (setChannelValue->lookup->var == NULL)
    {
        printf("La fixture non esiste!\n");
        return;
    }
    
    int value = (int) eval(setChannelValue->value);
    
    //Se il valore non è corretto
    if (value < 0 || value > 255)
    {
        printf("Valore non consentito\n");
        return;
    }
    
    // Prendo l'indirizzo del canale
    int address = getChannelAddress(setChannelValue->lookup->var->fixtureType, setChannelValue->channelName);
    
    if(address == -1)
    {
        printf("Canale inesistente\n");
        return;
    }
    
    address += ((int)eval((struct ast *)setChannelValue->lookup)) - 1;

    dmxUniverse[address] = value;
    printf("Valore settato: %d\n", dmxUniverse[address]);
}

void newFixtureEval(struct newFixture * newFixture)
{
    //La funzione newFixtureEval fa l'evaluate delle fixture

    //Inizializzo il valore della fixturetype con quella contenuta all'interno della typetab
    struct fixtureType * fixtureType = lookupFixtureType(newFixture->fixtureTypeName);
    int startAddress = (int) eval(newFixture->address);

    if (createFixture(fixtureType, startAddress, newFixture->fixture))
    {
        if (DEBUG)
            printf("Fixture dichiarata\n Nome variabile: %s\nNome tipo: %s\nIndirizzo: %d\n", newFixture->fixture->name, newFixture->fixture->fixtureType->name, (int) newFixture->fixture->value);
    }    
}

void createArrayEval(struct createArray * createArray)
{
    if (createArray->fixtureType == NULL)
    {
        printf("Il tipo non esiste\n");
        return;
    }

    if (createArray->array->array != NULL)
    {
        printf("Array già dichiarato\n");
        return;
    }

    if (createArray->size <= 0)
    {
        printf("Dimensione non consentita\n");
        return;
    }

    int startAddress = (int) eval(createArray->startAddress);
    createArray->array->value = startAddress;
    createArray->array->array = malloc(sizeof(struct array));
    struct array * arrayList = createArray->array->array;

    struct var * var = malloc(sizeof(struct var));
    createFixture(createArray->fixtureType, startAddress, var);
    
    arrayList->index = 0;
    arrayList->var = var;

    int size = (int) eval(createArray->size);
    int numberOfChannels = getNumberOfChannels(createArray->fixtureType);
    for (int i = 1; i < size; ++i)
    {
        arrayList->next = malloc(sizeof(struct array));
        arrayList = arrayList->next;

        struct var * var = malloc(sizeof(struct var));
        createFixture(createArray->fixtureType, startAddress + (numberOfChannels * i), var);
        
        arrayList->index = i;
        arrayList->var = var;
    }

    printf("Array creato\n");
}

void macroCallEval(struct macro * m)
{
    struct macro * mc = lookupMacro(m->macroName);

    if(mc == NULL)
    {
        printf("Unknown macro %s\n", m->macroName);
        return;
    }

    struct astList * instructionsList = mc->instruction;
    
    while (instructionsList != NULL)
    {
        eval(instructionsList->this);
        instructionsList = instructionsList->next;
    } 
    
    
}
