#include "headers/astbuilder.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"
#include "headers/evalFunctions.h"
#include "headers/free.h"


void* fadeEval(void * params)
{
    struct fade * fadeStruct = (struct fade *)params;
    struct var * fixture = fadeStruct->fixture;
    if (fixture == NULL || fixture->fixtureType == NULL)
    {
        printf("La variabile non esiste.\n");
        return NULL;
    }

    int channel = getChannelAddress(fixture->fixtureType, fadeStruct->channelName);

    if (channel == -1)
        return NULL;

    channel += fixture->intValue - 1;

    unsigned char currentValue = dmxUniverse[channel];
    
    int value = eval(fadeStruct->value)->intVal;
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
    if (fixture == NULL || fixture->fixtureType == NULL)
    {
        printf("La fixture non esiste.\n");
        return NULL;
    }

    int channel = getChannelAddress(fixture->fixtureType, delayStruct->channelName);

    if (channel == -1)
        return NULL;

    channel += fixture->intValue - 1;
    int time = eval(delayStruct->time)->intVal;

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
    //La funzione setChannelValueEval modifica il valore di un canale

    struct var * variable = setChannelValue->lookup->var;
    //Se non è presente lookupFixtureType ritorna null
    if (variable == NULL || variable->fixtureType == NULL)
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
    int address = getChannelAddress(variable->fixtureType, setChannelValue->channelName);
    
    if(address == -1)
    {
        printf("Canale inesistente\n");
        return;
    }

    if (setChannelValue->lookup->index != NULL)
    {
        int myIndex = eval(setChannelValue->lookup->index)->intVal;
        struct array * arrayList = setChannelValue->lookup->var->array;

        while (arrayList != NULL)
        {
            if (arrayList->index == myIndex)
            {
                variable = arrayList->var;
                break;
            }

            arrayList = arrayList->next;
        }

        if (variable == NULL)
        {
            if (DEBUG)
                printf("Variable not found\n");
            return;
        }
    }
    
    address += variable->intValue - 1;

    dmxUniverse[address] = value;
    
    if (DEBUG)
        printf("Canale %d - Valore: %d\n", address, dmxUniverse[address]);
}

void newFixtureEval(struct newFixture * newFixture)
{
    //Inizializzo il valore della fixturetype con quella contenuta all'interno della typetab
    struct fixtureType * fixtureType = lookupFixtureType(newFixture->fixtureTypeName);
    int startAddress = eval(newFixture->address)->intVal;

    if (newFixture->lookup->index == NULL) //It's a variable
        createFixture(fixtureType, startAddress, newFixture->lookup->var);
    else //It's an array
        createFixtureArray(fixtureType, startAddress, newFixture->lookup);

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
        if (variable->varType == ARRAY_VAR)
        {
            if (l->index != NULL) //It's a variable of an array
            {
                int found = 0;
                struct array * array = variable->array;
                int myIndex = eval(l->index)->intVal;

                if (myIndex < variable->intValue)
                {
                    while (array != NULL)
                    {
                        if (array->index == myIndex)
                        {
                            printf("My index: %d\nIndex: %d\n", myIndex, array->index);
                            variable = array->var;
                            found = 1;
                            break;
                        }
                            
                        array = array->next;
                    }

                    if (!found)
                        return getEvaluatedFromInt(0);
                }
                else
                {
                    printf("\nERROR: Index out of bound!\n");
                    return getEvaluatedFromInt(-1);
                }
            }
            else
            {
                //Restituisco la dimensione dell'array
                return getEvaluatedFromInt(l->var->intValue);
            }
        }

        switch (variable->varType)
        {
            case INT_VAR:
            case FIXTURE_VAR:
                return getEvaluatedFromInt(variable->intValue);
            case DOUBLE_VAR:
                return getEvaluatedFromDouble(variable->doubleValue);
            case STRING_VAR:
                return getEvaluatedFromString(variable->stringValue);
            default:
                return getEvaluatedFromString("Variabile inesistente.");
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
        left = evalLeft->doubleVal;

    if (evalRight->type == STRING_VAR)
        right = strlen(evalRight->stringVal);
    else
        right = evalRight->doubleVal;

    struct evaluated * evaluated = NULL;
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
        case MOD:
        {
            int intLeft = (int) left;
            int intRight = (int) right;
            evaluated = getEvaluatedFromInt(intLeft % intRight);
        }
            break;
        case CONCAT:
        {
            char * leftString = evalLeft->stringVal;
            char * rightString = evalRight->stringVal;
            char * newString = malloc(sizeof(char) * (strlen(leftString) + strlen(rightString)));

            newString = strcat(newString, leftString);
            newString = strcat(newString, rightString);
            
            return getEvaluatedFromString(newString);
        }
        default:
            return NULL;
    }

    if (evaluated->type == DOUBLE_VAR && (evaluated->doubleVal - evaluated->intVal) == 0)
        evaluated = getEvaluatedFromInt(evaluated->intVal);

    return evaluated;
}

void newAsgnEval(struct asgn * asg)
{
    struct evaluated * value = eval(asg->value); 
    struct var * variable = asg->lookup->var;
    int myIndex = -1;
    
    if (asg->lookup->index != NULL)
        myIndex = eval(asg->lookup->index)->intVal;

    if (myIndex >= 0 && variable->varType == NONE)
    {
        variable->varType = ARRAY_VAR;
        variable->intValue = myIndex + 1;
    }

    if (myIndex >= 0 && variable->varType == ARRAY_VAR)
    {
        if (variable->intValue <= myIndex)
            variable->intValue = myIndex + 1;

        struct array * array = variable->array;

        if (array == NULL)
        {
            variable->array = malloc(sizeof(struct array));
            array = variable->array;
        }
        else
        {
            //Vado in ultima posizione
            while (array->next != NULL && array->index != myIndex)
                array = array->next;

            array->next = malloc(sizeof(struct array));
            array = array->next;
        }
        array->index = myIndex;
        array->var = malloc(sizeof(struct var));
        variable = array->var;
    }

    if (variable->varType != FIXTURE_VAR &&
        variable->varType != ARRAY_VAR)
    {
        variable->varType = value->type;
        variable->stringValue = value->stringVal;
        variable->doubleValue = value->doubleVal;
        variable->intValue = value->intVal;
    }
}

void createArrayEval(struct createArray * createArray)
{
    struct var * variable = createArray->lookup->var;
    if (createArray->lookup->index == NULL)
    {
        printf("ERROR - you have not declared an array.\n");
        return;
    }

    int size = eval(createArray->lookup->index)->intVal;

    if (size <= 0)
    {
        printf("ERROR - size not valid.\n");
        return;
    }

    variable->varType = ARRAY_VAR;
    variable->intValue = size;
    struct astList * values = createArray->values;
    struct array * arrayList;

    for (int i = 0; i < size; ++i)
    {
        if (i == 0)
        {
            variable->array = malloc(sizeof(struct array));
            arrayList = variable->array;
        }
        else
        {
            arrayList->next = malloc(sizeof(struct array));
            arrayList = arrayList->next;
        }

        arrayList->var = malloc(sizeof(struct var));
        variable = arrayList->var;
        arrayList->index = i;
        
        struct evaluated * value = values != NULL ? eval(values->this) : getEvaluatedFromInt(0);
        variable->varType = value->type;
        variable->doubleValue = value->doubleVal;
        variable->intValue = value->intVal;
        variable->stringValue = value->stringVal;
        
        values = values != NULL ? values->next : NULL;
    }
}

void deleteVar(struct var * var )
{
    //cancellazione di una fixture
    if (var == NULL)
    {
        printf("La variabile non esiste.\n");
        return;
    }

    int startAddress, maxAddress;
    if(var->fixtureType != NULL)
    {
        if (var->array == NULL)
        {
            startAddress = var->intValue;
            maxAddress = startAddress + getNumberOfChannels(var->fixtureType) - 1;
        }
        else
        {
            startAddress = var->array->var->intValue;
            maxAddress = startAddress + (getNumberOfChannels(var->fixtureType) * var->intValue) - 1;
        }
        
        for (int i = startAddress; i <= maxAddress; ++i)
                dmxOccupied[i] = NULL;
    }
    
    freeVariable(var);
}

void deleteMacro(char * macroName)
{
    struct macro * m = lookupMacro(macroName);
    
    freeMacro(m);
}