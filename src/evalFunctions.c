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
    int size = eval(createArray->size)->intVal;
    int startAddress = eval(createArray->startAddress)->intVal;

    createArray->array->intValue = size;
    
    createArray->array->array = malloc(sizeof(struct array));
    struct array * arrayList = createArray->array->array;

    struct var * var = malloc(sizeof(struct var));
    createFixture(createArray->fixtureType, startAddress, var);
    
    arrayList->index = 0;
    arrayList->var = var;

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
    if (l->fixtureType != NULL) //It's a fixtureType
    {
        int value = 0;
        struct channelList * cl = l->fixtureType->cl;
        while (cl != NULL)
        {
            ++value;
            cl = cl->next;
        }

        return getEvaluatedFromInt(value);
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
                            return getEvaluatedFromInt(array->var->intValue);
                            
                        array = array->next;
                    }
                    
                    printf("\nERROR: Index out of bound!\n");
                    return getEvaluatedFromInt(-1);
                }
                else
                {
                    //Restituisco la dimensione dell'array
                    return getEvaluatedFromInt(l->var->intValue);
                }
            break;
            case INT_VAR:
            case FIXTURE_VAR:
                return getEvaluatedFromInt(variable->intValue);
            break;
            case DOUBLE_VAR:
                return getEvaluatedFromDouble(variable->doubleValue);
            break;
            case STRING_VAR:
                return getEvaluatedFromString(variable->stringValue);
            break;
            default:
                printf("\nERROR: Variable type not found\n");
            break;
        }
    }

    return NULL;
}

struct evaluated * evalExpr(struct ast * a)
{
    
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
        right = strlen(evalRight->stringVal);
    else
    {
        //@TODO: o il double o l'int
        right = evalRight->doubleVal;
    }

    switch (a->nodetype)
    {
        // caso espressioni 
        case PLUS:
            return getEvaluatedFromDouble(left + right);
        case MINUS:
            return getEvaluatedFromDouble(left - right);
        case MUL:
            return getEvaluatedFromDouble(left * right);
        case DIV:
            return getEvaluatedFromDouble(left / right);
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

            char * newString = malloc(sizeof(leftString) + sizeof(rightString));
            newString = strcat(newString, leftString);
            newString = strcat(newString, rightString);
            
            return getEvaluatedFromString(newString);
        }
        default:
            return NULL;
    }
}

void newAsgnEval(struct asgn * asg)
{
    struct evaluated * evaluated = eval(asg->value); 

    asg->lookup->var->varType = evaluated->type;
    asg->lookup->var->stringValue = evaluated->stringVal;
    asg->lookup->var->doubleValue = evaluated->doubleVal;
    asg->lookup->var->intValue = evaluated->intVal;
}


void deleteVar(struct lookup *lookup )
    {
        struct var * var = lookup->var ;
        int startAddress = var->intValue;
        int maxAddress = startAddress + getNumberOfChannels(var->fixtureType) - 1;


            printf("partenza: %d || fine %d", startAddress,maxAddress);
         for (int i = startAddress; i <= maxAddress; ++i)
        {
            if (dmxOccupied[i] != NULL)
            {
                dmxOccupied[i] = NULL;
            }
        }

     {
            var->nodetype = -1;
            var->varType = -1;
            var->name = NULL;            
            var->intValue = 0;
            var->doubleValue = 0;
            var->stringValue = NULL;
            var->fixtureType = NULL;
            var->array = NULL;
     }

     
}



/*
void deleteMacro(struct macro *macro)
    {

        struct macro *m = macrotab[varhash(macro->macroName)%NHASH];
        int scount = NHASH;	
             
        {
            m->nodetype = NULL;
            m->macroName = NULL;
            m->instruction = NULL;
        }
}
*/


