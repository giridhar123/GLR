#include "headers/parser.h"
#include "headers/astbuilder.h"
#include "headers/sharedVariables.h"

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
        /* constant */
        case NUM:
            v = ((struct numval *)a)->number;
            break;

        /* Variable invocation */
        case INVOKE:
        {
            struct invoke * i = (struct invoke *) a;
            if (i->ft != NULL)
            {
                struct channelList * cl = i->ft->cl;

                v = 0;
                while (cl != NULL)
                {
                struct channel * ch = cl->channel;
                printf("%s: %d\n", ch->name, ch->address);
                ++v;
                cl = cl->next;
                }
            }
            else
                v = i->v->value;
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
                    eval(currentAst);
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
                printf("= %4.4g\n", eval(currentAst));
                astList = astList->next;
            }
            break;
        }
        default:
            printf("internal error: bad node %d\n", a->nodetype);
    }


    return v;
}

unsigned int varhash(char *var)
{
    //Funzione per fare l hash
    unsigned int hash = 0;
    unsigned c;

    while (c = *var++)
    hash = hash*9 ^ c;

    return hash;
}

struct var * lookupVar(char * name)
{
    //La funzione lookupVar controlla all'interno della tabella vartab se c'è o meno il nome di una variabile.
     //Se la trova la ritorna
     //Se NON la trova inizializza una nuova variabile 

    //inizializzo il puntatore all'indirizzo di memoria della vartab alla posizione che viene dall'hash%nhash
    struct var *var = &vartab[varhash(name)%NHASH];
    int scount = NHASH;		/* contatore, lo inizializzo alla dimensione massima possibile (9997) */

    //Inizio ciclo, finisco appena lo scorro tutto oppure trovo un valore in tabella e ritorno il valore trovato
    while(--scount >= 0) 
     
    {
        //Se trovo la variabile inserita come parametro della funzione all'interno della tabella, la ritorno.
        if (var->name && !strcmp(var->name, name))
        { 
            return var;
        }

        if(!var->name) 
        {
            /* inizializzo una nuova variabile */
            var->name = strdup(name);
            var->value = 0;
            var->func = NULL;
            var->vars = NULL;
            var->fixtureType = NULL;
            return var;
        }

        if(++var >= vartab+NHASH)
            var = vartab; /* try the next entry */
    }
    yyerror("symbol table overflow\n");
    abort(); /* tried them all, table is full */
}

void parseFile(char * fileName) 
{
    //Apre il file in lettura e starta il parsing tramite il file.
    FILE * file = fopen(fileName, "r");
    startParser(file);
}

void* fadeEval(void * params)
{
    struct fade * fadeStruct = (struct fade *)params;

    struct var * fixture = lookupVar(fadeStruct->variableName);
    struct fixtureType * fixtureType = fixture->fixtureType;

    int channel = -1;
    struct channelList * channelList = fixtureType->cl;

    while(channelList != NULL)
    {
        if (!strcmp(fadeStruct->channelName, channelList->channel->name))
        {
            channel = channelList->channel->address;
            break;
        }
        channelList = channelList->next;
    }

    if (channel == -1)
        return NULL;

    channel += fixture->value - 1;

    unsigned char currentValue = dmxUniverse[channel];
    double difference = fadeStruct->value - currentValue;
    int step = difference > 0 ? 1 : -1;
    int time = fabs((fadeStruct->time * 1000 * 1000) / difference);
    
    while (dmxUniverse[channel] != fadeStruct->value)
    {
        dmxUniverse[channel] = dmxUniverse[channel] + step;
        usleep(time);
    }
}


void newFixtureEval(struct newFixture * newFixture)
{
    //La funzione newFixtureEval fa l'evaluate delle fixture

    //Inizializzo il valore della fixturetype con quella contenuta all'interno della typetab
    struct fixtureType * fixtureType = lookupFixtureType(newFixture->fixtureTypeName);

    //Se non è presente lookupFixtureType ritorna null
    if (fixtureType == NULL)
    {
        //
        printf("Il tipo non esiste!\n");
        return;
    }

    int address = (int) eval(newFixture->address);

    //Se l'indirizzo non è corretto
    if (address < 1 || address > 512)
    {
        printf("Indirizzo non valido\n");
        return;
    }

    //Nel caso in cui trovo il fixturetype, faccio lo stesso discorso con la lookupVar.
    struct var * variable = lookupVar(newFixture->fixtureName);

    //se la variabile è già dichiarata
    if (variable->fixtureType != NULL)
    {
        printf("Variabile già dichiarata\n");
        return;
    }

    //Setto la fixturetype della variabile e l'indirizzo della variabile con quelli trovati con la struct fixtureType
    variable->fixtureType = fixtureType;
    variable->value = address;

    if (DEBUG)
        printf("Fixture dichiarata\n Nome variabile: %s\nNome tipo: %s\nIndirizzo: %d\n", variable->name, variable->fixtureType->name, (int) variable->value);
}

void setChannelValueEval(struct setChannelValue * setChannelValue)
{
    //La funzione setChannelValueEval fa l'evaluate del canale

    //Inizializzo il valore della variabile con quella contenuta all'interno della vartab
    struct var * variable = lookupVar(setChannelValue->fixtureName);

    //Se non è presente lookupFixtureType ritorna null
    if (variable == NULL)
    {
        printf("La variabile non esiste!\n");
        return;
    }

    int value = eval(setChannelValue->value);

    //Se l'indirizzo non è corretto
    if (value < 0 || value > 255)
    {
        printf("Valore non consentito\n");
        return;
    }

    //Se var esiste ed è corretta, prendo la channel list della variabile
    struct channelList * channelList = variable->fixtureType->cl;

    // Prendo l'indirizzo della variabile
    int address = variable->value;

    //Cerco l'indirizzo del canale in base al nome
    while (channelList != NULL)
    {
        if (!strcmp(channelList->channel->name, setChannelValue->channelName))
        {
            address += channelList->channel->address - 1;
            break;
        }
        channelList = channelList->next;
    }

    if(channelList == NULL)
    {
        printf("Canale inesistente\n");
        return;
    } 

    //Imposto il valore del canale     
    dmxUniverse[address] = value;

    printf("Valore settato: %d\n", dmxUniverse[address]);
}

void* delayEval(void * params)
{
    struct fade * delayStruct = (struct fade *)params;

    struct var * fixture = lookupVar(delayStruct->variableName);
    struct fixtureType * fixtureType = fixture->fixtureType;

    int channel = -1;
    struct channelList * channelList = fixtureType->cl;

    while(channelList != NULL)
    {
        if (!strcmp(delayStruct->channelName, channelList->channel->name))
        {
            channel = channelList->channel->address;
            break;
        }
        channelList = channelList->next;
    }

    if (channel == -1)
        return NULL;

    channel += fixture->value - 1;

    usleep(delayStruct->time * 1000 * 1000);
    dmxUniverse[channel] = delayStruct->value;
}

struct fixtureType * lookupFixtureType(char * name)
{
    struct fixtureType *ft = &typetab[varhash(name)%NHASH];
    int scount = NHASH;		

    while(--scount >= 0)
    {
        if (ft->name && !strcmp(ft->name, name))
            return ft;

        if(++ft >= typetab+NHASH)
            ft = typetab; 
    }

    return NULL;

    yyerror("symbol table overflow\n");
    abort(); 
}