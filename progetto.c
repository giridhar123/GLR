#include "progetto.h"
#include "free.h"
    
int main (int argc, char ** argv)
{
    /*
        Set the SIGINT (Ctrl-C) signal handler to sigintHandler  
        Refer http://en.cppreference.com/w/c/program/signal
    */
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
    for (int i = 0; i < 513; ++i)
        dmxUniverse[i] = 0;

    printf("Opening serial port...\n");
    int serial_port = open("/dev/cu.usbserial-A50285BI", O_WRONLY);

    // Check for errors
    if (serial_port < 0) {
        printf("Error %i from open: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    printf("Opened port...\n");

    // Create new termios struc, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    
    struct termios tty;

    // Read in existing settings, and handle any error
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
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    //Imposto il baud rate a basso livello perchè la liberia OSX è differente da quella linux
    speed_t speed = (speed_t)250000;
    //ioctl(serial_port, IOSSIOSPEED, &speed);
    
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

void* startParser(void * param)
{
    yylex_destroy();
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
    unsigned int hash = 0;
    unsigned c;

    while (c = *var++)
    hash = hash*9 ^ c;

    return hash;
}

struct var * lookupVar(char * name)
{
    struct var *var = &vartab[varhash(name)%NHASH];
    int scount = NHASH;		/* how many have we looked at */

    while(--scount >= 0) {
    if (var->name && !strcmp(var->name, name))
    { 
        return var;
    }

    if(!var->name) {
        /* new entry */
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
    struct invoke * i = malloc(sizeof(struct invoke));

    if(!i) {
        yyerror("out of space");
        exit(0);
    }

    i->nodetype = INVOKE;

    struct fixtureType * fixtureType = lookupFixtureType(name);
    if (fixtureType !=  NULL)
        i->ft = fixtureType;
    else
        i->v = lookupVar(name);

    return (struct ast *)i;
}

struct ast * newast(int nodetype, struct ast *l, struct ast *r)
{
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
    struct channel *c = malloc(sizeof(struct channel));

    if(!c) {
        yyerror("out of space");
        exit(0);
    }

    c->name = name;
    c->address = (int) address;

    return (struct ast *)c;
}

struct ast * newChannelList (struct ast * c, struct ast * otherList)
{
    struct channelList * cl = malloc(sizeof(struct channelList));

    if(!cl) {
        yyerror("out of space");
        exit(0);
    }

    cl->channel = (struct channel *) c;
    cl->next = (struct channelList *) otherList;

    struct channelList * tmp = cl;

    return (struct ast *) cl;
}

struct ast * newDefine(char * name, struct ast * cl)
{
    struct fixtureType * f = malloc(sizeof(struct fixtureType));

    if(!f) {
        yyerror("out of space");
        exit(0);
    }

    //vedo in typetab se c'è la cl name che sto inserendo, in caso chiedo conferma
     struct fixtureType *ft = &typetab[varhash(name)%NHASH];
         int scount = NHASH;		/* how many have we looked at */
          while(--scount >= 0)
            {
                    if(ft->name )
                        {
                            printf("%s", ft->name);
                            printf("%s", name);
                        }
                if(++ft >= typetab+NHASH)
                ft = typetab; /* try the next entry */
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
    int scount = NHASH;		/* how many have we looked at */

    while(--scount >= 0)
    {
        if (ft->name && !strcmp(ft->name, name))
            return ft;

        if(++ft >= typetab+NHASH)
            ft = typetab; /* try the next entry */
    }

    return NULL;

    yyerror("symbol table overflow\n");
    abort(); /* tried them all, table is full */
}

struct ast * newFixture(char * fixtureTypeName, char * fixtureName, double address)
{
    struct newFixture *nf = malloc(sizeof(struct newFixture));

    if(!nf)
        printf("out of memory");

    nf->nodetype = NEW_FIXTURE;
    nf->fixtureTypeName = fixtureTypeName;
    nf->fixtureName = fixtureName;
    nf->address = address;

    return (struct ast * ) nf;
}

void newFixtureEval(struct newFixture * newFixture)
{
    struct fixtureType * fixtureType = lookupFixtureType(newFixture->fixtureTypeName);

    if (fixtureType == NULL)
    {
        //Il tipo non esiste
        printf("Il tipo non esiste!\n");
        return;
    }

    if (newFixture->address < 1 || newFixture->address > 512)
    {
        printf("Indirizzo non valido\n");
        return;
    }

    struct var * variable = lookupVar(newFixture->fixtureName);

    if (variable->fixtureType != NULL)
    {
        printf("Variabile già dichiarata\n");
        return;
    }

    variable->fixtureType = fixtureType;
    variable->value = newFixture->address;

    if (DEBUG)
        printf("Fixture dichiarata\n Nome variabile: %s\nNome tipo: %s\nIndirizzo: %lf\n", variable->name, variable->fixtureType->name, variable->value);
}

struct ast * setChannelValue(char * fixtureName, char * channelName, double value)
{
    struct setChannelValue * cv = malloc(sizeof(struct setChannelValue));

    if(!cv)
        printf("out of memory");
    
    cv->nodetype = SET_CHANNEL_VALUE;
    cv->fixtureName = fixtureName;
    cv->channelName = channelName;
    cv->value = value;

    return (struct ast *) cv;
}

void setChannelValueEval(struct setChannelValue * setChannelValue)
{
    struct var * variable = lookupVar(setChannelValue->fixtureName);

    if (variable == NULL)
    {
        printf("La variabile non esiste!\n");
        return;
    }

    if (setChannelValue->value < 0 || setChannelValue->value > 255)
    {
        printf("Valore non consentito\n");
        return;
    }

    struct channelList * channelList = variable->fixtureType->cl;

    int address = variable->value;

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

    dmxUniverse[address] = setChannelValue->value;
}

void parseFile(char * fileName) {
    FILE * file = fopen(fileName, "r");
    startParser(file);
}

struct astList * newAstList(struct ast * this, struct astList * next)
{
    struct astList * al = malloc(sizeof(struct astList));

    al->this = this;
    al->next = next;

    return al;
}

struct ast * newLoop(char * varName, double start, double end, struct astList * al)
{
    struct loop *l = malloc(sizeof(struct loop));

    if(!l)
        printf("out of memory");

    l->nodetype = LOOP_TYPE;
    l->start = (int) start;
    l->end = (int) end;
    l->varName = varName;
    l->assegnazioni = al;

    return (struct ast *) l;
}