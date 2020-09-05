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
    int time = fabs((eval(fadeStruct->time)->intVal * 1000 * 1000) / difference);
    
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
    dmxUniverse[channel] = eval(delayStruct->value)->intVal;

    return NULL;
}

void sleepEval(struct sleep * s)
{
    double seconds = eval(s->seconds)->doubleVal;
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
    
    int value = eval(setChannelValue->value)->intVal;
    
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
    
    address += eval((struct ast *)setChannelValue->lookup)->intVal - 1;

    dmxUniverse[address] = value;
    printf("Valore settato: %d\n", dmxUniverse[address]);
}

void newFixtureEval(struct newFixture * newFixture)
{
    //La funzione newFixtureEval fa l'evaluate delle fixture

    //Inizializzo il valore della fixturetype con quella contenuta all'interno della typetab
    struct fixtureType * fixtureType = lookupFixtureType(newFixture->fixtureTypeName);
    int startAddress = eval(newFixture->address)->intVal;

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

    int startAddress = eval(createArray->startAddress)->intVal;
    createArray->array->intValue = startAddress;
    createArray->array->array = malloc(sizeof(struct array));
    struct array * arrayList = createArray->array->array;

    struct var * var = malloc(sizeof(struct var));
    createFixture(createArray->fixtureType, startAddress, var);
    
    arrayList->index = 0;
    arrayList->var = var;

    int size = eval(createArray->size)->intVal;
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

struct evaluated * lookupEval(struct lookup * l)
{
    struct evaluated * evaluated = malloc(sizeof(struct evaluated));

    if (!evaluated)
    {
        printf("No memory\n");
        exit(0);
    }

    if (l->fixtureType != NULL) //It's a fixtureType
    {
        struct channelList * cl = l->fixtureType->cl;
        while (cl != NULL)
        {
            evaluated->intVal += 1;
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
                    int myIndex = eval(l->index)->intVal;

                    while (array != NULL)
                    {
                        if (array->index == myIndex)
                        {
                            evaluated->intVal = array->var->intValue;
                            break;
                        }
                        array = array->next;
                    }
                    if (evaluated->intVal == 0)
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
                        evaluated->intVal += 1;
                        array = array->next;
                    }
                }
            break;
            case INT_VAR:
            case FIXTURE_VAR:
                evaluated->intVal = variable->intValue;
            break;
            case DOUBLE_VAR:
                evaluated->doubleVal = variable->doubleValue;
            break;
            default:
                printf("\nERROR: Variable type not found\n");
            break;
        }
    }

    return evaluated;
}

struct evaluated * evalExpr(struct ast * a)
{
    struct evaluated * evaluated = malloc(sizeof(struct evaluated));

    struct evaluated * evalLeft = eval(a->l);
    struct evaluated * evalRight = eval(a->r);

    double left, right;
    if (evalLeft->type == STRING_VAR)
        left = strlen(evalLeft->stringVal);
    else
    {
        //@TODO: o il double o l'int
        left = evalLeft->doubleVal;
    }

    if (evalRight->type == STRING_VAR)
        left = strlen(evalRight->stringVal);
    else
    {
        //@TODO: o il double o l'int
        right = evalRight->doubleVal;
    }

    switch (a->nodetype)
    {
        // caso espressioni 
        case PLUS:
            evaluated = getEvaluatedFromDouble(left + right);
        break;
        case MINUS:
            evaluated = getEvaluatedFromDouble(left - right);
        break;
        case MUL:
            evaluated = getEvaluatedFromDouble(left * right);
        break;
        case DIV:
            evaluated = getEvaluatedFromDouble(left / right);
        break;
        case CONCAT:
        {
            char * leftString;
            if (evalLeft->stringVal != NULL)
                leftString = evalLeft->stringVal;
            else
            {
                leftString = malloc(sizeof(evalLeft->doubleVal));
                snprintf(leftString, 8, "%2.4f", evalLeft->doubleVal);
            }

            char * rightString;
            if (evalRight->stringVal != NULL)
                rightString = evalRight->stringVal;
            else
            {
                rightString = malloc(sizeof(evalRight->doubleVal));
                snprintf(rightString, 8, "%2.4f", evalRight->doubleVal);
            }

            evaluated->stringVal = malloc(sizeof(leftString) + sizeof(rightString));
            evaluated->stringVal = strcat(evaluated->stringVal, leftString);
            evaluated->stringVal = strcat(evaluated->stringVal, rightString);
        }
        break;
    }

    return evaluated;
}