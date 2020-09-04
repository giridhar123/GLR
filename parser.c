#include "headers/parser.h"
#include "headers/evalFunctions.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"

struct var * dmxOccupied[513];

void* startParser(void * param)
{
    //Inizio del parsing

    //La funzione yylex_destroy è chiamata per liberare le risorse usate dallo scanner. 
     //In modo tale che, una volta fatto il read file al posto di rimanere sul file posso
      // ritornare agli input standard.
    yylex_destroy();

    // inizializzo il puntatore input stream al parametro passato alla funzione che può essere
     // o un file oppure lo standard stdin passandogli stdin oppure il valore NULL
    yyin = (FILE *) param; 

    //inizio il parsing
    if(!yyparse())
    {
        printf("\nParsing complete\n");
        if (yyin != stdin)
        startParser(stdin);
    }    
    else
    {
        printf("\nParsing failed\n");
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

double eval(struct ast *a)
{
    double v;

    if(!a) {
        yyerror("internal error, null eval");
        return 0.0;
    }

    switch(a->nodetype)
    {
        // imposto v al (double) all'interno della struct numval
        case NUM:
            v = ((struct numval *)a)->number;
            break;

        /* Variable invocation */
        case LOOKUP:
        {
            struct lookup * l = (struct lookup *) a;
            if (l->var != NULL && l->index != NULL) //It's a variable of an array
            {
                struct array * array = l->var->array;
                int myIndex = (int) eval(l->index);

                while (array != NULL)
                {
                    if (array->index == myIndex)
                    {
                        v = array->var->value;
                        break;
                    }
                    array = array->next;
                }
            }
            else if (l->fixtureType != NULL) //It's a fixtureType
            {
                v = 0;
                struct channelList * cl = l->fixtureType->cl;
                while (cl != NULL)
                {
                    ++v;
                    cl = cl->next;
                }
            }
            else if (l->var != NULL && l->index == NULL) //it's a variable or an array
            {
                if (l->var->array == NULL) //IT's a variable
                    v = l->var->value;
                else //it's an array
                {
                    v = 0;
                    struct array * array = l->var->array;
                    while (array != NULL)
                    {
                        ++v;
                        array = array->next;
                    }
                }
            }
            else printf("\nERROR: Index out of bound!\n");
        }
        break;

        /* Fixture type - Define */
        case FIXTURE_TYPE:
        {
            printf("Hai definito un nuovo tipo: %s\n", ((struct fixtureType *)a)->name);
            v = 0;
        } 
        break;

        /* Variable Fixture */
        case NEW_FIXTURE: 
        {
            struct newFixture * nf = (struct newFixture *)a;
            newFixtureEval(nf);
            v = 0;
        }
        break;

        /* Set Channel Value */
        case SET_CHANNEL_VALUE:
        {
            struct setChannelValue * cv = (struct setChannelValue *) a;
            setChannelValueEval(cv);
            v = 0;
        }
        break;

        case LOOP_TYPE:
        {
            struct loop * l = (struct loop *) a;
            struct var * index = lookupVar(l->indexName);
            
            index->value = l->start;

            while(((int)index->value) <= l->end)
            {   
                struct astList * astList = l->assegnazioni;
               
                while(astList != NULL)
                {
                    struct ast * currentAst = astList->this; 
                    printf("%f\n",eval(currentAst));
                    astList = astList->next;
                }

                index->value++;
            }

            index->value = 0;
        }   
        break;

        case COMPARE:
        {
            struct compare * cmp = (struct compare *)a;
            int left = eval(cmp->left);
            int right = eval(cmp->right);
            switch(cmp->cmp) 
            {
                case 1: 
                    if(left > right)
                        v=1; 
                    else
                        v=0; 
                break;
                case 2: 
                    if (left < right)
                        v=1;
                    else
                        v=0;
                break;
                case 3: 
                    if (left != right)
                        v=1;
                    else
                        v=0;
                break;
                case 4: 
                    if (left == right)
                        v=1;
                    else
                        v=0;
                break;
                case 5: 
                    if (left >= right)
                        v=1;
                    else
                        v=0;
                break;                
                case 6: 
                    if (left <= right)
                        v=1 ;
                    else
                        v=0;
                break;
            }
        } 
        break;

        // caso espressioni 
        case '+':
            v = eval(a->l) + eval(a->r);
        break;
        case '-':
            v = eval(a->l) - eval(a->r);
        break;
        case '*':
            v = eval(a->l) * eval(a->r);
        break;
        case '/':
            v = eval(a->l) / eval(a->r);
        break;

        case FADE_TYPE:
        {
            struct fade * fadeStruct = (struct fade *) a;
            pthread_t fadeThread;

            pthread_create(&fadeThread, NULL, &fadeEval, fadeStruct);
            v = 0;
        }
        break;

        case DELAY_TYPE:
        {
            struct fade * delayStruct = (struct fade *) a;
            pthread_t delayThread;

            pthread_create(&delayThread, NULL, &delayEval, delayStruct);
            v = 0;
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
                double currentValue = eval(currentAst);
                if(currentValue != 0 )
                {
                    printf("= %4.4g\n", currentValue);
                }
                astList = astList->next;
            }
            break;
        }
        break;

        case SLEEP_TYPE:
        {
            struct sleep * s = (struct sleep *)a;
            sleepEval(s);
            v = 0;
        }
        break;

        case CREATE_ARRAY:
        {
            struct createArray * c = (struct createArray *)a;
            createArrayEval(c);
            v = 0;
        }
        break;

        case MACRO_CALL:
        {
            struct macro * m = (struct macro *)a;
            macroCallEval(m);
            v = 0;
        }
        break;
        
        case GET_CHANNEL_VALUE:
        {
            struct getChannelValue * g = (struct getChannelValue *)a;

            if (g->lookup->var == NULL && g->lookup->fixtureType == NULL)
            {
                printf("Variabile inesistente\n");
                v = 0;
            }
            else if (g->lookup->fixtureType != NULL)
                v = getChannelAddress(g->lookup->fixtureType, g->channelName);
            else if (g->lookup->var != NULL) //it's a variable or an array
            {
                struct var * variable = g->lookup->var;
                if (g->lookup->index != NULL) //It's a variable of an array
                {
                    int myIndex = eval(g->lookup->index);
                    struct array * array = g->lookup->var->array;
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
                
                int address = getChannelAddress(variable->fixtureType, g->channelName);
                if (address == -1)
                {
                    printf("Canale inesistente.\n");
                    v = 0;
                }
                else
                {
                    address += variable->value - 1;
                    v = (double) dmxUniverse[address];
                }
            }
        }
        break;

        default:
        {
            //Non è stato riscontrato un nodetype valido, stampo il nome del nodetype
            printf("Nodetype non valido: %d\n", a->nodetype);
        }
    }
    return v;
}

