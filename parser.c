#include "headers/parser.h"
#include "headers/evalFunctions.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"

struct var * dmxOccupied[513];
struct fileList * fileList;

void* startParser(void * param)
{
    //Inizio del parsing

    //La funzione yylex_destroy è chiamata per liberare le risorse usate dallo scanner. 
     //In modo tale che, una volta fatto il read file al posto di rimanere sul file posso
      // ritornare agli input standard.
    //yylex_destroy();

    // inizializzo il puntatore input stream al parametro passato alla funzione che può essere
     // o un file oppure lo standard stdin passandogli stdin oppure il valore NULL
    //yyin = (FILE *) param; 
    if(fileList->this == stdin)
        yylex_destroy();

    yyin = fileList->this;

    //inizio il parsing
    if(!yyparse())
        printf("\nParsing complete\n");   
    else
        printf("\nParsing failed\n");

    //Restart parser
    if(fileList->this != stdin && fileList->next != NULL)
    {
        struct fileList * toFree = fileList;
        fileList = fileList->next;
        fclose(toFree->this);
        free(toFree);
        startParser(stdin);
    }
    else
    {
        fileList->this = stdin;
        startParser(stdin);
    }
        
    return NULL;
}

void yyerror(const char *s, ...)
{
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

struct evaluated * eval(struct ast *a)
{
    struct evaluated * evaluated = malloc(sizeof(struct evaluated));

    if(!a)
    {
        yyerror("internal error, null eval");
        return NULL;
    }

    if (!evaluated)
    {
        yyerror("internal error, no free memory");
        return NULL;
    }

    switch(a->nodetype)
    {
        // imposto v al (double) all'interno della struct numval
        case NUM:
            evaluated = getEvaluatedFromDouble(((struct numval *) a)->number);
        break;

        /* Variable invocation */
        case LOOKUP:
            evaluated = lookupEval((struct lookup *) a);
        break;

        /* Variable Fixture */
        case NEW_FIXTURE:
            newFixtureEval((struct newFixture *)a);
        break;

        /* Set Channel Value */
        case SET_CHANNEL_VALUE:
            setChannelValueEval((struct setChannelValue *) a);
        break;

        case LOOP_TYPE:
        {
            struct loop * l = (struct loop *) a;
            struct var * index = lookupVar(l->indexName);
            
            index->varType = INT_VAR;
            index->intValue = l->start;

            while(index->intValue <= l->end)
            {   
                struct astList * astList = l->stmtList;
               
                while(astList != NULL)
                {
                    struct ast * currentAst = astList->this; 
                    eval(currentAst);
                    astList = astList->next;
                }

                index->intValue++;
            }

            index->intValue = 0;
        }   
        break;

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

        // caso espressioni 
        case PLUS:
        case MINUS:
        case MUL:
        case DIV:
        case CONCAT:
            evaluated = evalExpr(a);
        break;

        case FADE_TYPE:
        {
            struct fade * fadeStruct = (struct fade *) a;
            pthread_t fadeThread;

            pthread_create(&fadeThread, NULL, &fadeEval, fadeStruct);
        }
        break;

        case DELAY_TYPE:
        {
            struct fade * delayStruct = (struct fade *) a;
            pthread_t delayThread;

            pthread_create(&delayThread, NULL, &delayEval, delayStruct);
        }
        break;
        
        case IF_TYPE: 
        {
            struct ifStruct * ifStruct = (struct ifStruct *) a;
            //faccio l'eval della condition, se mi viene 0 faccio l'expressione 1 (dopo then) | altrimenti faccio l'espressione2 (dopo else)
            struct astList * astList = ((int) eval(ifStruct->cond)) == 1 ? ifStruct->thenStmt : ifStruct->elseStmt;
            
            while(astList != NULL)
            {
                struct ast * currentAst = astList->this; 
                evaluated = eval(currentAst);
                if(evaluated != NULL )
                {
                    printf("= %4.4g\n", evaluated->doubleVal);
                }
                astList = astList->next;
            }
            break;
        }
        break;

        case SLEEP_TYPE:
            sleepEval((struct sleep *)a);
        break;

        case CREATE_ARRAY:
            createArrayEval((struct createArray *) a);
        break;

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

        case STRING_TYPE:
            evaluated = getEvaluatedFromString(((struct string *)a)->value);
        break;
        
        case PRINT_TYPE:
        {
            struct print * p = (struct print *)a;
            printf("%s\n", eval(p->a)->stringVal);
        }
        break;

        case NEW_ASGN:
            newAsgnEval((struct asgn *) a);
        break;

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

        default:
        {
            //Non è stato riscontrato un nodetype valido, stampo il nome del nodetype
            printf("Nodetype non valido: %d\n", a->nodetype);
        }
    }
    return evaluated;
}