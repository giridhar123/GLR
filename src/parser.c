#include "headers/parser.h"
#include "headers/evalFunctions.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"

struct var * dmxOccupied[513];
struct fileList * fileList;

void* startParser(void * param)
{
    //Inizio del parsing

    // inizializzo il puntatore input stream al parametro passato alla funzione che può essere
     // o un file oppure lo standard stdin passandogli stdin oppure il valore NULL
    
    yyin = (FILE *) param; 

    //inizio il parsing
    if(!yyparse())
    {
        printf("\nParsing complete\n");
    }   
    else
    {
        printf("\nParsing failed\n");
        if(DEBUG)
            startParser(stdin); ///Restart parser
    }
    
    return NULL;
}

void yyerror(const char *s, ...)
{
    // funzione base del parser
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

struct evaluated * eval(struct ast *a)
{
    // La funzione mi permette di fare un evaluate di una struct.
     // In base a cosa sia l ast passata come parametro, descritta tramite il nodetype da un elenco enum all'interno del file structs.h
      // Faccio operazioni diverse. ritorno sempre la variabile: evaluated.
    struct evaluated * evaluated = malloc(sizeof(struct evaluated));

    // Se a è null ritorno errore
    if(!a)
    {
        yyerror("internal error, null eval");
        return NULL;
    }

    // Se esaurisco la memoria a mia disposizione per creare l'evaluated, ritorno null.
    if (!evaluated)
    {
        yyerror("internal error, no free memory");
        return NULL;
    }

    // Switch case della nodetype
    switch(a->nodetype)
    {
        // Sto esaminando un numero. 
         // Può essere double oppure intero, questo controllo è fatto tramite una sottrazione.
        case NUM:
            evaluated = getEvaluatedFromDouble(((struct numval *) a)->number);

            if ((evaluated->doubleVal - evaluated->intVal) == 0)
                evaluated = getEvaluatedFromInt(evaluated->intVal);
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

        // Devo effettuare una compare per l'if
        case COMPARE:
        {
            struct compare * cmp = (struct compare *)a;
            double left = eval(cmp->left)->doubleVal;
            double right = eval(cmp->right)->doubleVal;
            switch(cmp->cmp) 
            {
                case 1: 
                    evaluated->intVal = left > right;
                break;
                case 2:
                    evaluated->intVal = left < right;
                break;
                case 3: 
                    evaluated->intVal = left != right;
                break;
                case 4: 
                    evaluated->intVal = left == right;
                break;
                case 5: 
                    evaluated->intVal = left >= right;
                break;                
                case 6: 
                    evaluated->intVal = left <= right;
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
            struct astList * astList = eval(ifStruct->cond)->intVal == 1 ? ifStruct->thenStmt : ifStruct->elseStmt;

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
            macroCallEval((struct macro *) a);
        break;
        

        case GET_CHANNEL_VALUE:
        {
            struct getChannelValue * g = (struct getChannelValue *)a;
            struct var * variable = g->lookup->var;
            if (g->lookup->fixtureType != NULL)
            {
                int address = getChannelAddress(g->lookup->fixtureType, g->channelName);
                evaluated = getEvaluatedFromInt(address);
                break;
            }
            else if (variable->varType == ARRAY_VAR && g->lookup->index != NULL) //It's a variable of an array
            {
                int myIndex = eval(g->lookup->index)->intVal;

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
            printf("%s\n", eval(p->a)->stringVal);
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