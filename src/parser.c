#include "headers/parser.h"
#include "headers/evalFunctions.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"
#include "headers/free.h"

struct var * dmxOccupied[513];
struct fileList * fileList;

extern int yylex_destroy(void);

void* startParser(void * param)
{
    // Inizio del parsing

    // Inizializzo il puntatore input stream con il parametro passato alla funzione che può essere
    // o un file oppure stdin
    
    yyin = (FILE *) param; 

    // Inizio il parsing
    if(!yyparse())
    {
        printf("\nParsing complete\n");
    }   
    else
    {
        printf("\nParsing failed\n");
        yylex_destroy();
        if(DEBUG)
            startParser(stdin); // Restart del parser
    }
    
    return NULL;
}

void yyerror(const char *s, ...)
{
    // funzione base del parser per stampare gli errori
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

struct evaluated eval(struct ast *a)
{
    // La funzione mi permette di fare una valutazione di una struct.
    // In base al tipo di ast passato come parametro descritta dal nodetype
    // Vengono fatte operazioni diverse
    struct evaluated evaluated = getEvaluatedFromInt(-1);

    // Se a è null ritorno errore
    if(!a)
    {
        yyerror("internal error, null eval");
        return evaluated;
    }

    // Se esaurisco la memoria a mia disposizione per creare l'evaluated, ritorno null.

    // Switch case della nodetype
    switch(a->nodetype)
    {
        // Sto esaminando un numero. 
        // Può essere double oppure intero, questo controllo è fatto tramite una sottrazione.
        case NUM:
            evaluated = getEvaluatedFromDouble(((struct numval *) a)->number);

            if ((evaluated.doubleVal - evaluated.intVal) == 0)
                evaluated = getEvaluatedFromInt(evaluated.intVal);
        break;


        // Sto esaminando una lookup
        case LOOKUP:
            evaluated = lookupEval((struct lookup *) a);
        break;

        // Sto esaminando una fixture type
        case NEW_FIXTURE:
            newFixtureEval((struct newFixture *)a);
        break;

        // Devo impostare un valore ad un canale
        case SET_CHANNEL_VALUE:
            setChannelValueEval((struct setChannelValue *) a);
        break;

        // Devo eseguire un loop
        case LOOP_TYPE:
            loopEval((struct loop *) a);
        break;

        // Devo effettuare un confronto
        case COMPARE:
        {
            struct compare * cmp = (struct compare *)a;
            double left = eval(cmp->left).doubleVal;
            double right = eval(cmp->right).doubleVal;
            switch(cmp->cmp) 
            {
                case 1: 
                    evaluated = getEvaluatedFromInt(left > right);
                break;
                case 2:
                    evaluated = getEvaluatedFromInt(left < right);
                break;
                case 3: 
                    evaluated = getEvaluatedFromInt(left != right);
                break;
                case 4: 
                    evaluated = getEvaluatedFromInt(left == right);
                break;
                case 5: 
                    evaluated = getEvaluatedFromInt(left >= right);
                break;                
                case 6: 
                    evaluated = getEvaluatedFromInt(left <= right);
                break;
            }
        } 
        break;

        // Sto esamindando un'espressione 
        case PLUS:
        case MINUS:
        case MUL:
        case DIV:
        case CONCAT:
        case MOD:
            evaluated = evalExpr(a);
        break;

        // Devo eseguire una fade, creo un thread
        case FADE_TYPE:
        {
            struct fade * fadeStruct = (struct fade *) a;
            pthread_t fadeThread;

            pthread_create(&fadeThread, NULL, &fadeEval, fadeStruct);
        }
        break;

        // Devo eseguire una delay, creo un thread
        case DELAY_TYPE:
        {
            struct fade * delayStruct = (struct fade *) a;
            pthread_t delayThread;

            pthread_create(&delayThread, NULL, &delayEval, delayStruct);
        }
        break;
        
        // Devo eseguire un if
        case IF_TYPE: 
        {
            // prendo la struttura dell'if e vedo se ha al suo interno il costrutto else.
            struct ifStruct * ifStruct = (struct ifStruct *) a;
            struct astList * astList = eval(ifStruct->cond).intVal == 1 ? ifStruct->thenStmt : ifStruct->elseStmt;

            while(astList != NULL)
            {
                struct ast * currentAst = astList->this; 
                eval(currentAst);
                astList = astList->next;
            }
            break;
        }
        break;

        // Devo eseguire una sleep
        case SLEEP_TYPE:
            sleepEval((struct sleep *)a);
        break;

        // Devo eseguire una macro
        case MACRO_CALL:
            macroCallEval((struct macroCall *) a);
        break;
        
        // Devo prendere il valore di un channel
        case GET_CHANNEL_VALUE:
        {
            // Estraggo la struct getChannelValue e la variabile
            struct getChannelValue * g = (struct getChannelValue *)a;
            struct var * variable = g->lookup->var;

            // Se è stata effettuata su un tipo di fixture e non su una variabile
            // restituisco l'indice del canale richiesto
            if (g->lookup->fixtureType != NULL)
            {
                int address = getChannelAddress(g->lookup->fixtureType, g->channelName);
                evaluated = getEvaluatedFromInt(address);
                break;
            }            
            // Devo estrarre il value channel all'interno di un array
            else if (variable->varType == ARRAY_VAR && g->lookup->index != NULL)
            {
                int myIndex = eval(g->lookup->index).intVal;

                struct array * array = variable->array;
                while (array != NULL)
                {
                    if (array->index == myIndex)
                    {
                        variable = array->var;
                        break;
                    }
                    array = array->next;
                }
            }
            // E' una fixture
            if (variable->varType == FIXTURE_VAR)
            {
                int address = getChannelAddress(variable->fixtureType, g->channelName);
                if (address != -1)
                {
                    address += variable->intValue - 1; //offset
                    evaluated = getEvaluatedFromInt((int) dmxUniverse[address]);
                }
                else
                    printf("Canale inesistente.\n");
            }
        }
        break;

        // Sto esaminando una stringa
        case STRING_TYPE:
            evaluated = getEvaluatedFromString(((struct string *)a)->value);
        break;
        
        // Devo stampare qualcosa
        case PRINT_TYPE:
        {
            struct print * p = (struct print *)a;
            struct evaluated value = eval(p->a);
            if (value.type == STRING_VAR)
                printf("%s\n", value.stringVal);
            else if (value.type == DOUBLE_VAR)
                printf("%f\n", value.doubleVal);
            else if (value.type == INT_VAR)
                printf("%d\n", value.intVal);
            else if (DEBUG)
                printf("Errore print \n");

            freeEvaluated(value);
        }
        break;

        // Devo assegnare un valore
        case NEW_ASGN:
            newAsgnEval((struct asgn *) a);
        break;

        // Devo creare un array
        case CREATE_ARRAY:
            createArrayEval((struct createArray *) a);
        break;

        // Devo prendere un input da tastiera
        case INPUT_TYPE:
        {
            char input[256];
            printf("> ");
            scanf("%s", input);
            int intValue = atoi(input);
            double doubleValue = atof(input);

            if (doubleValue == intValue && intValue == 0 && input[0] != '0')
                return getEvaluatedFromString(input);
            if ((doubleValue - intValue) > 0)
                return getEvaluatedFromDouble(doubleValue);
            else
                return getEvaluatedFromInt(intValue);
        }
        break;

        // Nel caso in cui non ho riconosciuto il nodetype, stampo il nome del nodetype
        default:
        {
            printf("Nodetype non valido: %d\n", a->nodetype);
        }
    }
    return evaluated;
}