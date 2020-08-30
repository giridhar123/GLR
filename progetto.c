#include "progetto.h"
#include "free.h"
int main (int argc, char ** argv)
{
    signal(SIGINT, freeEverything); 

    pthread_t serialPortThread, parser;

    FILE * source = stdin;
    if (argc > 1)
        source = fopen(argv[1], "r");

    pthread_create(&parser, NULL, &startParser, source);
    pthread_create(&serialPortThread, NULL, &startDMX, NULL);

    //Join solo sul parser, se quest'ultimo termina, termina anche la serial port
    pthread_join(parser, NULL);

    freeEverything();
    return 0;
}

void yyerror(char *s, ...)
{
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

void* startDMX(void * params)
{
    // Iniziliazzazione del vettore universare
    for (int i = 0; i < 513; ++i)
        dmxUniverse[i] = 0;

    printf("Opening serial port...\n");
    // Apertura porta seriale
    int serial_port = open("/dev/cu.usbserial-A50285BI", O_WRONLY);

    // Controllo errori
    if (serial_port < 0) {
        printf("Error %i from open: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    printf("Opened port...\n");

    // Create new termios struc, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    
    // Creazione di una termios struct, ci scriveremo la configurazione esistente.
    struct termios tty;

    // Read in existing settings, and handle any error
    // Lettura di configurazioni già esistenti.
    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag |= CSTOPB;  // Set stop field, two stop bits used in communication
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Save tty settings, also checking for error
    // Salvataggio impostazioni tty.
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    //Imposto il baud rate a basso livello perchè la liberia OSX è differente da quella linux.
    speed_t speed = (speed_t)250000;
    //ioctl(serial_port, IOSSIOSPEED, &speed); //@TODO Su windows il termine IOSSISPEED non funziona poiché viene dalla libreria IOKit/serial/ioss.h (bisogna scaricare la libreria online)
                                                //https://developer.apple.com/documentation/iokit
    ioctl(serial_port, TIOCSBRK); //Start break
    while(1)
    {
        usleep(100);
        ioctl(serial_port, TIOCCBRK); //Stop break

        write(serial_port, dmxUniverse, sizeof(dmxUniverse));

        tcflush(serial_port, TCIOFLUSH);

        ioctl(serial_port, TIOCSBRK); //Start break

        usleep(18000);
    }

    printf("Serial port closing...\n");
    close(serial_port);
    printf("Serial port closed...\n");

    return NULL;
}

/* Inutilizzata
void * startParserFromFile(void * param)
{
    yyin = (FILE *) param;

    //inizio il parsing
    if(!yyparse())
    {
        printf("\nParsing complete\n");
        yylex_destroy();
        startParser(NULL);
    }    
    else
    {
        printf("\nParsing failed\n");
    }
}
*/

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

/* var table */
/* hash a var */
static unsigned varhash(char *var)
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

    if(!var->name) {
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

struct ast * newInvoke(char * name)
{
    //La funzione newInvoke server per definire una variabile oppure una fixturetype
    struct invoke * i = malloc(sizeof(struct invoke));

    if(!i) {
        yyerror("out of space");
        exit(0);
    }

    i->nodetype = INVOKE;
    //Cerco, se esiste, la fixture type. 
     //Nel caso in cui non riesco a trovarla lookupFixtureType ritorna null e definisco il nome inserito come variabile.
    struct fixtureType * fixtureType = lookupFixtureType(name);
    if (fixtureType !=  NULL)
        i->ft = fixtureType; 
    else
        i->v = lookupVar(name);

    return (struct ast *)i;
}

struct ast * newast(int nodetype, struct ast *l, struct ast *r)
{
    //La funzione newast serve per svolgere le operazioni matematiche di base ( + - * / )
    struct ast *a = malloc(sizeof(struct ast));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}


struct ast * newnum(double d)
{
    //la funzione newnum inserisce un numero all'ast 
    struct numval *a = malloc(sizeof(struct numval));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = NUM;
    a->number = d;
    return (struct ast *)a;
}

double eval(struct ast *a)
{
    double v;

    if(!a) {
        yyerror("internal error, null eval");
        return 0.0;
    }

    switch(a->nodetype) {
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
            printf("Hai definito un nuovo tipo: %s\n", ((struct fixtureType *)a)->name);
            v = 0;
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

            int index = l->start;
            int end = l->end;

            while(index <= end)
            {
                struct astList * astList = l->assegnazioni;
               
                while(astList != NULL)
                {
                    struct ast * currentAst = astList->this; 
                    eval(currentAst);
                    astList = astList->next;
                }

                index++;
            }
        }   
        break;

        /* assignment */
        /*
        case '=': v = ((struct symasgn *)a)->s->value =
            eval(((struct symasgn *)a)->v); break;
            */

        /* expressions */
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
        
        /*
        case '|': v = fabs(eval(a->l)); break;
        case 'M': v = -eval(a->l); break;
        */

        /* comparisons */
        /*
        case '1': v = (eval(a->l) > eval(a->r))? 1 : 0; break;
        case '2': v = (eval(a->l) < eval(a->r))? 1 : 0; break;
        case '3': v = (eval(a->l) != eval(a->r))? 1 : 0; break;
        case '4': v = (eval(a->l) == eval(a->r))? 1 : 0; break;
        case '5': v = (eval(a->l) >= eval(a->r))? 1 : 0; break;
        case '6': v = (eval(a->l) <= eval(a->r))? 1 : 0; break;
        */

        /* control flow */
        /* null if/else/do expressions allowed in the grammar, so check for them */
        /*
        case 'I': 
        if( eval( ((struct flow *)a)->cond) != 0) {
            if( ((struct flow *)a)->tl) {
        v = eval( ((struct flow *)a)->tl);
            } else
        v = 0.0;
        */
        /* a default value */
        /*
        } else {
            if( ((struct flow *)a)->el) {
            v = eval(((struct flow *)a)->el);
            } else
        v = 0.0;
        */
        /* a default value */
        /*
        }
        break;

        case 'W':
        v = 0.0;
        */
        /* a default value */
        /*
        if( ((struct flow *)a)->tl) {
            while( eval(((struct flow *)a)->cond) != 0)
        v = eval(((struct flow *)a)->tl);
        }
        break;
        */
        /* last value is value */
        /*         
        case 'L': eval(a->l); v = eval(a->r); break;

        case 'F': v = callbuiltin((struct fncall *)a); break;

        case 'C': v = calluser((struct ufncall *)a); break;
        */
        default:
            printf("internal error: bad node %d\n", a->nodetype);
    }
    return v;
}

struct ast * newChannel(double address, char * name)
{ 
    //la funzione NewChannel serve per inserire un nuovo canale nell ast
    struct channel *c = malloc(sizeof(struct channel));

        if(!c) 
        {
            yyerror("out of space");
            exit(0);
        }

    c->name = name; // nome
    c->address = (int) address; //indirizzo

    return (struct ast *)c;
}

struct ast * newChannelList (struct ast * c, struct ast * otherList)
{
     //la funzione NewChannel serve per inserire una nuova lista di canali nell ast
    struct channelList * cl = malloc(sizeof(struct channelList));

        if(!cl) 
         {
          yyerror("out of space");
          exit(0);
         }

    cl->channel = (struct channel *) c; //valore del canale
    cl->next = (struct channelList *) otherList; //@todo

    struct channelList * tmp = cl;

    return (struct ast *) cl;
}

struct ast * newDefine(char * name, struct ast * cl)
{
    //la funzione newDefine serve per definire una nuova fixturetype da terminale
    struct fixtureType * f = malloc(sizeof(struct fixtureType));

        if(!f) 
        {
            yyerror("out of space");
            exit(0);
        }

    struct fixtureType *ft = &typetab[varhash(name)%NHASH];
        int scount = NHASH;		
        while(--scount >= 0)
            {
                if(++ft >= typetab+NHASH)
                ft = typetab; 
            }


    f->nodetype = FIXTURE_TYPE;
    f->name = name;
    f->cl = (struct channelList *) cl;
    
    //Aggiunge il tipo alla lookup table dei tipi
    typetab[varhash(f->name) % NHASH] = *f;

    return (struct ast *) f;
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

struct ast * newFixture(char * fixtureTypeName, char * fixtureName, double address)
{
    //La funzione newFixture serve per inizializzare una fixture.
     //Una volta inizializzata una FixtureType precedentemente ( nome della strumentazione)
      //la pongo uguale ad una variabile ( FixtureName ) e ad un indirizzo
       //in modo tale che posso cambiare i valori di quella specifico strumento. 
        //con comandi semplici come fixtureName.red = 255
    struct newFixture *nf = malloc(sizeof(struct newFixture));

    if(!nf) {
        printf("out of memory");
        exit(0);
    }

    nf->nodetype = NEW_FIXTURE; // il tipo di nodo.
    nf->fixtureTypeName = fixtureTypeName; // la fixtureType.
    nf->fixtureName = fixtureName; // il nome della variabile.
    nf->address = address; // l'indirizzo

    return (struct ast * ) nf;
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

    //Se l'indirizzo non è corretto
    if (newFixture->address < 1 || newFixture->address > 512)
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
    variable->value = newFixture->address;

    if (DEBUG)
        printf("Fixture dichiarata\n Nome variabile: %s\nNome tipo: %s\nIndirizzo: %lf\n", variable->name, variable->fixtureType->name, variable->value);
}

struct ast * setChannelValue(char * fixtureName, char * channelName, double value)
{
    //La funzione setChannelValue imposta il valore di un determinato canale.
    struct setChannelValue * cv = malloc(sizeof(struct setChannelValue));

    if(!cv) {
        printf("out of memory");
        exit(0);
    }
    
    cv->nodetype = SET_CHANNEL_VALUE; // tipologia del nodo
    cv->fixtureName = fixtureName; // il nome della fixture
    cv->channelName = channelName; // il nome del canale
    cv->value = value; // l'indirizzo

    return (struct ast *) cv;
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

    //Se l'indirizzo non è corretto
    if (setChannelValue->value < 0 || setChannelValue->value > 255)
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
    dmxUniverse[address] = setChannelValue->value;
}

void parseFile(char * fileName) 
{
    //Apre il file in lettura e starta il parsing tramite il file.
    FILE * file = fopen(fileName, "r");
    startParser(file);
}

struct astList * newAstList(struct ast * this, struct astList * next)
{
    //Creo una nuova astlist. next punta o a null oppure ad un'altra astlist passata come parametro
    struct astList * al = malloc(sizeof(struct astList));

    al->this = this;
    al->next = next;

    return al;
}

struct ast * newLoop(char * varName, double start, double end, struct astList * al)
{
    //@todo da capire prima di commentare
    struct loop *l = malloc(sizeof(struct loop));

    if(!l) {
        printf("out of memory");
        exit(0);
    }

    l->nodetype = LOOP_TYPE;
    l->start = (int) start;
    l->end = (int) end;
    l->varName = varName;
    l->assegnazioni = al;

    return (struct ast *) l;
}

struct ast * newFade(char * variableName, char * channelName, double value, double time)
{
    struct fade * fade = malloc(sizeof(struct fade));

    if(!fade) {
        printf("out of memory");
        exit(0);
    }

    fade->nodetype = FADE_TYPE;
    fade->variableName = variableName;
    fade->channelName = channelName;
    fade->value = (int) value;
    fade->time = time;

    return (struct ast *) fade;
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

struct ast * newDelay(char * variableName, char * channelName, double value, double time)
{
    struct fade * delay = malloc(sizeof(struct fade));

    if(!delay) {
        printf("out of memory");
        exit(0);
    }

    delay->nodetype = DELAY_TYPE;
    delay->variableName = variableName;
    delay->channelName = channelName;
    delay->value = (int) value;
    delay->time = time;

    return (struct ast *) delay;
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