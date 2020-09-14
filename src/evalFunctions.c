#include "headers/astbuilder.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"
#include "headers/evalFunctions.h"
#include "headers/free.h"
#include "headers/parser.h"

#include <unistd.h>
#include <sys/time.h>
void* fadeEval(void * params)
{
    // La funzione mi permette di fare l'evaluate del fade
    struct fade * fadeStruct = (struct fade *)params;
    struct var * fixture = fadeStruct->fixture;

    // Verifico che non siano nulli i parametri
    if (fixture == NULL || fixture->fixtureType == NULL)
    {
        printf("La variabile non esiste.\n");
        return NULL;
    }

    // Prendo il channel address della fixture
    int channel = getChannelAddress(fixture->fixtureType, fadeStruct->channelName);

    // Verifico che il canale sia valido
    if (channel == -1)
        return NULL;

    channel += fixture->intValue - 1;

    unsigned char currentValue = dmxUniverse[channel];
    
    // Calcolo il valore che ho passato dentro la fade struct
    int value = eval(fadeStruct->value).intVal;
    // Calcolo la differenza
    double difference = value - currentValue;

    // Conto lo step e mi calcolo il time
    int step = difference > 0 ? 1 : -1;
    int time = fabs((eval(fadeStruct->time).intVal * 1000 * 1000) / difference);
    
    // Ogni time, aggiungo al channel di riferimento il valore di step.
    while (dmxUniverse[channel] != value)
    {
        dmxUniverse[channel] = dmxUniverse[channel] + step;
        usleep(time);
    }
    
    return NULL;
}

void * delayEval(void * params)
{
    // La funzione mmi permette di attribuire il valore ad un channel dopo tot tempo
    struct fade * delayStruct = (struct fade *)params;
    struct var * fixture = delayStruct->fixture;
    // Solite verifiche
    if (fixture == NULL || fixture->fixtureType == NULL)
    {
        printf("La fixture non esiste.\n");
        return NULL;
    }

    int channel = getChannelAddress(fixture->fixtureType, delayStruct->channelName);

    if (channel == -1)
        return NULL;

    // Calcolo il tempo
    channel += fixture->intValue - 1;
    int time = eval(delayStruct->time).intVal;
    // Aspetto e passo il parametro, evalutato, all'array del dmx
    usleep(time * 1000 * 1000);
    dmxUniverse[channel] = eval(delayStruct->value).intVal;

    return NULL;
}

void sleepEval(struct sleep * s)
{
    // Valutazione dello sleep
    double seconds = eval(s->seconds).doubleVal;
    int milliseconds = 1000 * seconds;
    usleep(milliseconds * 1000);
}

void setChannelValueEval(struct setChannelValue * setChannelValue)
{
    // La funzione setChannelValueEval modifica il valore di un canale

    struct var * variable = setChannelValue->lookup->var;
    // Se non è presente lookupFixtureType ritorna null
    if (variable == NULL || variable->fixtureType == NULL)
    {
        printf("La fixture non esiste!\n");
        return;
    }
    
    int value = eval(setChannelValue->value).intVal;
    
    // Se il valore non è corretto
    if (value < 0 || value > 255)
    {
        printf("Valore non consentito\n");
        return;
    }

    // Prendo l'indirizzo del canale
    int address = getChannelAddress(variable->fixtureType, setChannelValue->channelName);
    
    // Verifico il canale
    if(address == -1)
    {
        printf("Canale inesistente\n");
        return;
    }

    // @davide
    if (setChannelValue->lookup->index != NULL)
    {
        int myIndex = eval(setChannelValue->lookup->index).intVal;
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
    int startAddress = eval(newFixture->address).intVal;

    if (newFixture->lookup->index == NULL) // E' una variabile
        createFixture(fixtureType, startAddress, newFixture->lookup->var);
    else // E' un array
        createFixtureArray(fixtureType, startAddress, newFixture->lookup);
}

void macroCallEval(struct macroCall * m)
{
    // Funzione per richiamare una macro
    struct macro * mc = lookupMacro(m->name);

    if(mc->instruction == NULL)
    {
        printf("Unknown macro %s\n", m->name);
        return;
    }

    struct timeval start, end;

    gettimeofday(&start, NULL);

    // Prendo l'intero set di istruzioni all'interno della macro e li valuto
    struct astList * instructionsList = mc->instruction;
    
    while (instructionsList != NULL)
    {
        eval(instructionsList->this);
        instructionsList = instructionsList->next;
    }
    gettimeofday(&end, NULL);

    double time_taken = end.tv_sec + end.tv_usec / 1e6 -
                        start.tv_sec - start.tv_usec / 1e6 ; // in seconds

    printf("time program took %lf seconds to execute\n", time_taken);

}

void loopEval(struct loop * l)
{
    // La funzione mi permette di gestire un loop
    struct var * index = lookupVar(l->indexName);

    // vedo la tipologia della variabile ed il valore
    index->varType = INT_VAR;
    index->intValue = eval(l->start).intVal;

    // Loop in avanti
    if(index->intValue <= eval(l->end).intVal)
    {
        struct astList * astList = NULL;
        struct ast * currentAst = NULL;

        while(index->intValue <= eval(l->end).intVal)
        {
            astList = l->stmtList;
        
            while(astList != NULL)
            {
                currentAst = astList->this; 
                eval(currentAst);
                astList = astList->next;
            }

            index->intValue++;
        }
    }
    else // Loop inverso
    {
        while(index->intValue >= eval(l->end).intVal)
        {
            struct astList * astList = l->stmtList;
        
            while(astList != NULL)
            {
                struct ast * currentAst = astList->this; 
                eval(currentAst);
                astList = astList->next;
            }

            index->intValue--;
        }
    }
    
    

    index->intValue = 0;
}

struct evaluated lookupEval(struct lookup * l)
{
    // La funzione mi permette di effettuare una valutazione di una lookup passata come parametro
     // Se è una fixture type faccio l'evalutate dell'attributo value e setto questo attributo uguale al numero di canali dentro la channel list 
    if (l->fixtureType != NULL) 
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
        // Se è una un array verifico la correttezza dell'indice passato come parametro e faccio l'evaluate
        struct var * variable = l->var;
        if (variable->varType == ARRAY_VAR)
        {
            if (l->index != NULL) 
            {
                int found = 0;
                struct array * array = variable->array;
                int myIndex = eval(l->index).intVal;

                if (myIndex < 0)
                {
                    printf("ERROR: Not valid index\n");
                    return getEvaluatedFromInt(-1);
                }

                if (myIndex < variable->intValue)
                {
                    // Scorro l'intero array
                    while (array != NULL)
                    {
                        if (array->index == myIndex)
                        {
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
                // Restituisco la dimensione dell'array
                return getEvaluatedFromInt(l->var->intValue);
            }
        }

        // Se è una variabile, devo fare l'evaluate in base alla tipologia della variabile (int,double,stringa)
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
                printf("Variabile inesistente.\n");
                return getEvaluatedFromInt(-1);
        }
    }

    return getEvaluatedFromInt(-1);
}

struct evaluated evalExpr(struct ast * a)
{   
    // La funzione mi permette di fare l'avaluate di un espressione.
     // Prendo le due variabili/valori che sono posizionati nell'albero
    struct evaluated evalLeft = eval(a->l);
    struct evaluated evalRight = eval(a->r);

    double left, right;
    // Verifico se sono stringhe o double(o interi)
    if (evalLeft.type == STRING_VAR)
        left = strlen(evalLeft.stringVal);
    else
        left = evalLeft.doubleVal;

    if (evalRight.type == STRING_VAR)
        right = strlen(evalRight.stringVal);
    else
        right = evalRight.doubleVal;

    struct evaluated evaluated;

    // In base a quale sia l'operazione, faccio un evaluate diverso considerando tutto double per poi riportare, in caso, in intero.
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

        // Il caso modulo è particolare perché è sempre un intero
        case MOD:
        {
            int intLeft = (int) left;
            int intRight = (int) right;
            evaluated = getEvaluatedFromInt(intLeft % intRight);
        }
        break;
        // Il caso della concatenazione richiede che siano due stringhe
        case CONCAT:
        {
            char * leftString = NULL;
            char * rightString = NULL;
            // Prendo entrambe le stringhe e creo una variabile che le può contenere entrambe. (malloc newString)
            if (evalLeft.type == INT_VAR)
            {
                leftString = malloc(sizeof(evalLeft.intVal));
                sprintf(leftString, "%d", evalLeft.intVal);
            }
            else if (evalLeft.type == DOUBLE_VAR)
            {
                leftString = malloc(sizeof(evalLeft.doubleVal));
                snprintf(leftString, 8, "%f", evalLeft.doubleVal);
            }
            else if (evalLeft.type == STRING_VAR)
                leftString = strdup(evalLeft.stringVal);


            if (evalRight.type == INT_VAR)
            {
                rightString = malloc(sizeof(evalRight.intVal));
                sprintf(rightString, "%d", evalRight.intVal);
            }
            else if (evalRight.type == DOUBLE_VAR)
            {
                rightString = malloc(sizeof(evalRight.doubleVal));
                snprintf(rightString, 8, "%f", evalRight.doubleVal);
            }
            else if (evalRight.type == STRING_VAR)
                rightString = strdup(evalRight.stringVal);
            
            char * newString = malloc(sizeof(char) * (strlen(leftString) + strlen(rightString) + 2));

            newString = strcpy(newString, leftString);
            newString = strcat(newString, rightString);

            evaluated = getEvaluatedFromString(newString);
            free(newString);
            newString = NULL;
            free(leftString);
            leftString = NULL;
            free(rightString);
            rightString = NULL;
        }
        break;
        default:
            evaluated = getEvaluatedFromInt(-1);
        break;
    }

    // @davide
    if (evaluated.type == DOUBLE_VAR && (evaluated.doubleVal - evaluated.intVal) == 0)
        evaluated.type = INT_VAR;

    freeEvaluated(&evalLeft);
    freeEvaluated(&evalRight);

    return evaluated;
}

void newAsgnEval(struct asgn * asg)
{
    // Questa funzione mi permette di effettuare un'assegnazione.
     // Prendo la variabile e la struct evalauted
    struct evaluated value = eval(asg->value); 
    struct var * variable = asg->lookup->var;
    
    if (asg->lookup->index != NULL)
    {
        int myIndex = eval(asg->lookup->index).intVal;
        if (myIndex < 0)
        {
            printf("ERROR: Not valid index\n");
            return;
        }
        else 
        {
            if (variable->varType == NONE)
            {
                variable->varType = ARRAY_VAR;
                variable->intValue = myIndex + 1;
            }

            if (variable->varType == ARRAY_VAR)
            {
                if (variable->intValue <= myIndex)
                    variable->intValue = myIndex + 1;

                struct array * array = variable->array;

                if (array == NULL)
                {
                    variable->array = malloc(sizeof(struct array));
                    array = variable->array;
                }
            
                // Vado in ultima posizione
                while (array->index != myIndex && array->next != NULL)
                    array = array->next;

                if (array->index != myIndex)
                {
                    array->next = malloc(sizeof(struct array));
                    array = array->next;
                    array->index = myIndex;
                    array->var = malloc(sizeof(struct var));
                }
                variable = array->var;
              
            }
        }
    }

    if (variable->varType == FIXTURE_VAR ||
        variable->varType == ARRAY_VAR)
    {
        printf("Variabile già dichiarata\n"); 
        return;
    }

    variable->varType = value.type;
    variable->stringValue = value.stringVal;
    variable->doubleValue = value.doubleVal;
    variable->intValue = value.intVal;
}

void createArrayEval(struct createArray * createArray)
{
    struct var * variable = createArray->lookup->var;
    if (createArray->lookup->index == NULL)
    {
        printf("ERROR - you have not declared an array.\n");
        return;
    }

    int size = eval(createArray->lookup->index).intVal;

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
        
        struct evaluated value;
        value = values != NULL ? eval(values->this) : getEvaluatedFromInt(0);
        variable->varType = value.type;
        variable->doubleValue = value.doubleVal;
        variable->intValue = value.intVal;
        variable->stringValue = value.stringVal;
        
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