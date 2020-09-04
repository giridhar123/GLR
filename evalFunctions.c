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

    channel += fixture->intValue - 1;

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

    channel += fixture->intValue - 1;
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
            printf("Fixture dichiarata\n Nome variabile: %s\nNome tipo: %s\nIndirizzo: %d\n", newFixture->fixture->name, newFixture->fixture->fixtureType->name, (int) newFixture->fixture->intValue);
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

    createArray->array->varType = ARRAY_VAR;

    int startAddress = (int) eval(createArray->startAddress);
    createArray->array->intValue = startAddress;
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

double lookupEval(struct lookup * l)
{
    double v = 0;
    if (l->fixtureType != NULL) //It's a fixtureType
    {
        struct channelList * cl = l->fixtureType->cl;
        while (cl != NULL)
        {
            ++v;
            cl = cl->next;
        }
    }
    else
    {
        struct var * variable = l->var;
        int varType = variable->varType;
        switch (varType)
        {
            case ARRAY_VAR:
                if (l->index != NULL) //It's a variable of an array
                {
                    struct array * array = variable->array;
                    int myIndex = (int) eval(l->index);

                    while (array != NULL)
                    {
                        if (array->index == myIndex)
                        {
                            v = array->var->intValue;
                            break;
                        }
                        array = array->next;
                    }
                    if (v == 0)
                    {
                        printf("\nERROR: Index out of bound!\n");
                        return 0;
                    }
                }
                else
                {
                    struct array * array = l->var->array;
                    while (array != NULL)
                    {
                        ++v;
                        array = array->next;
                    }
                }
            break;
            case INT_VAR:
            case FIXTURE_VAR:
                v = variable->intValue;
            break;
            case DOUBLE_VAR:
                v = variable->doubleValue;
            break;
            default:
                printf("\nERROR: Variable type not found\n");
            break;
        }
    }

    return v;
}

void evalPrint(struct ast * a, char ** string)
{
    if (a == NULL)
        return;

    char * stringValue = NULL;
    int value = -1;
    if (a->nodetype == STRING_TYPE)
    {
        struct string * myString = (struct string *)a;
        *string = myString->value;
        return;
    }
    
    if (a->nodetype == PLUS ||
        a->nodetype == MINUS ||
        a->nodetype == MUL ||
        a->nodetype == DIV ||
        a->nodetype == CONCAT)
    {
        char * stringLeft;
        char * stringRight;
        evalPrint(a->l, &stringLeft);
        evalPrint(a->r, &stringRight);
        switch (a->nodetype)
        {
            case PLUS:
                value = strlen(stringLeft) + strlen(stringRight);
            break;
            case MINUS:
                value = strlen(stringLeft) - strlen(stringRight);
            break;
            case MUL:
                value = strlen(stringLeft) * strlen(stringRight);
            break;
            case DIV:
                value = strlen(stringLeft) / strlen(stringRight);
            break;
            case CONCAT:
                stringValue = malloc(sizeof(char) * (strlen(stringLeft) + strlen(stringRight)));
                stringValue = strcat(stringValue, stringLeft);
                stringValue = strcat(stringValue, stringRight);
            break;
        }
    }
    else if (a->nodetype == LOOKUP)
    {
        struct lookup * l = (struct lookup *)a;
        if (l->var->varType == STRING_TYPE)
            stringValue = l->var->stringValue;
    }
    else
    {
        double value = eval(a);
        //Converto il double in una stringa
        stringValue = malloc(sizeof(value));
        snprintf(stringValue, 8, "%2.4f", value);
    }

    char * newString;
    if (stringValue != NULL)
    {
        newString = malloc(sizeof(char) * (strlen(*string) + strlen(stringValue)));
        newString = strcat(newString, *string);
        newString = strcat(newString, stringValue);
        *string = newString;
    }
    else
    {
        newString = malloc((sizeof(char) * strlen(*string) + sizeof(value)));
        newString = strcat(newString, *string);
        snprintf(newString, 32, "%d", value);
        *string = newString;
    }
}