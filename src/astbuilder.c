#include "headers/astbuilder.h"
#include "headers/sharedVariables.h"
#include "headers/parserUtils.h"
#include "headers/parser.h"

struct fixtureType * typetab[NHASH];
struct var vartab[NHASH];
struct macro * macrotab[NHASH];
unsigned char dmxUniverse[513];

struct ast * newast(int nodetype, struct ast *l, struct ast *r)
{
    // La funzione newast serve per svolgere le operazioni matematiche di base ( + - * / )
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
    // La funzione newnum inserisce un numero all'ast 
    struct numval *a = malloc(sizeof(struct numval));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = NUM;
    a->number = d;
    return (struct ast *)a;
}

struct channel * newChannel(double address, char * name)
{ 
    // La funzione NewChannel serve per inserire un nuovo canale nell ast
    struct channel *c = malloc(sizeof(struct channel));

    if(!c) 
    {
        yyerror("out of space");
        exit(0);
    }

    c->name = name; // nome
    c->address = (int) address; //indirizzo
    return c;
}

struct channelList * newChannelList (struct channel * c, struct channelList * otherList)
{
     // La funzione NewChannel serve per inserire una nuova lista di canali nell ast
    struct channelList * cl = malloc(sizeof(struct channelList));

    if(!cl) 
    {
        yyerror("out of space");
        exit(0);
    }

    cl->channel = c; //valore del canale
    cl->next = otherList; //mette in coda la lista precedente
    return cl;
}

struct ast * newFixture(char * fixtureTypeName, struct lookup * lookup, struct ast * address)
{
    // La funzione newFixture serve per inizializzare una fixture.
     // Una volta inizializzata una FixtureType precedentemente ( nome della strumentazione)
      // La pongo uguale ad una variabile ( FixtureName ) e ad un indirizzo
       // In modo tale che posso cambiare i valori di quella specifico strumento. 
        // Con comandi semplici come fixtureName.red = 255
    struct newFixture *nf = malloc(sizeof(struct newFixture));

    if(!nf) {
        printf("out of memory");
        exit(0);
    }

    nf->nodetype = NEW_FIXTURE; // il tipo di nodo
    nf->fixtureTypeName = fixtureTypeName; // la fixtureType
    nf->lookup = lookup; // la variabile
    nf->address = address; // l'indirizzo

    return (struct ast * ) nf;
}

struct ast * newSetChannelValue(struct lookup * lookup, char * channelName, struct ast * value)
{
    // La funzione setChannelValue imposta il valore di un determinato canale.
    struct setChannelValue * cv = malloc(sizeof(struct setChannelValue));

    if(!cv) {
        printf("out of memory");
        exit(0);
    }
    
    cv->nodetype = SET_CHANNEL_VALUE; // tipologia del nodo
    cv->lookup = lookup; // la fixture
    cv->channelName = channelName; // il nome del canale
    cv->value = value; //Il valore da impostare

    return (struct ast *) cv;
}

struct astList * newAstList(struct ast * this, struct astList * next)
{
    // Creo una nuova astlist. next punta o a null oppure ad un'altra astlist passata come parametro
    struct astList * al = malloc(sizeof(struct astList));

    al->this = this;
    al->next = next;

    return al;
}

struct ast * newLoop(char * indexName, struct ast * start, struct ast * end, struct astList * stmtList)
{
    // Imposto una struct loop con tutti i vari valori, inizio, fine, nome dell'indice, istruzioni
    struct loop *l = malloc(sizeof(struct loop));

    if(!l) {
        printf("out of memory");
        exit(0);
    }

    l->nodetype = LOOP_TYPE;
    l->start = start;
    l->end = end;
    l->indexName = indexName;
    l->stmtList = stmtList;

    return (struct ast *) l;
}

struct ast * newCompare(int cmptype, struct ast * left, struct ast * right)
{
    struct compare * cmp = malloc(sizeof(struct compare));
    
    cmp->nodetype = COMPARE;
    cmp->left = left;
    cmp->right = right;
    cmp->cmp = cmptype;
    
    return (struct ast *) cmp ;
}

struct ast * newFade(struct lookup * lookup, char * channelName, struct ast * value, struct ast * time)
{
    // La funzione newFade mi permette di inizializazre una struct di tipo fade con tutti i vari valori:
    // fixture, il nome del canale, il tempo, etc...
    struct fade * fade = malloc(sizeof(struct fade));

    if(!fade) {
        printf("out of memory");
        exit(0);
    }

    fade->nodetype = FADE_TYPE;
    fade->fixture = lookup->var;
    fade->channelName = channelName;
    fade->value = value;
    fade->time = time;

    return (struct ast *) fade;
}

struct ast * newDelay(struct lookup * lookup, char * channelName, struct ast * value, struct ast * time)
{
    // La funzione newDelay mi permette di inizializzare una struct simile a quella del fade descritta sopra.
    struct fade * delay = malloc(sizeof(struct fade));

    if(!delay) {
        printf("out of memory");
        exit(0);
    }

    delay->nodetype = DELAY_TYPE;
    delay->fixture = lookup->var;
    delay->channelName = channelName;
    delay->value = value;
    delay->time = time;

    return (struct ast *) delay;
}

struct ast * newIf(struct ast * cond, struct astList * thenStmt, struct astList * elseStmt)
{
    // La funzione newIf mi permette di inizializare una struct per lo statement IF-ELSE. 
     // L'else statement può essere anche null.
    struct ifStruct *a = malloc(sizeof(struct ifStruct));
    
    if(!a)
    {
        yyerror("out of space");
        exit(0);
    }

    a->nodetype = IF_TYPE;
    a->cond = cond;
    a->thenStmt = thenStmt;
    a->elseStmt = elseStmt;

    return (struct ast *)a;
}

struct astList * astToAstList(struct ast * a)
{
    // La funzione permette di ritornare un AstList passandogli un Ast.
    struct astList *b = malloc(sizeof(struct astList));
    b->this = a ;
    b->next = NULL ;
    return b;

}

struct ast * newSleep(struct ast * seconds)
{
    // La funzione newSleep mi permette di inizializzare una struct di tipo sleep. 
    struct sleep * s = malloc(sizeof(struct sleep));

    if(!s)
        printf("out of memory");
    
    s->nodetype = SLEEP_TYPE;
    s->seconds = seconds;

    return (struct ast *) s;
}

struct ast * newMacroCall(char * name)
{
    // La funzione newMacroCall mi permette di inizializzare una nuova macro, porto i valori di istruction a null per poi modificarli dopo. Qui semplicemente la dichiaro.
    struct macroCall * m = malloc(sizeof(struct macroCall));

     if(!m)
        printf("out of memory");

    m->nodetype = MACRO_CALL;
    m->name = strdup(name);

    return (struct ast *)m;
}

struct lookup * newLookup(char * name)
{
    // La funzione newLookup mi permette di inizializzare una lookup. @da commentare
    struct lookup * l = malloc(sizeof(struct lookup));

    if(!l)
    {
        printf("out of memory");
        exit(0);
    }

    l->nodetype = LOOKUP;
    struct fixtureType * ft = lookupFixtureType(name);

    l->fixtureType = ft->cl == NULL ? NULL : ft;
    l->var = l->fixtureType == NULL ? lookupVar(name) : NULL;
    l->index = NULL;
    
    return l;
}

struct lookup * newLookupFromArray(char * arrayName, struct ast * index)
{
    // La funzioe newLookUpFromArray ha la stessa utilità di newLookup descritta sopra, solo che è per gli array e quindi contiene anche l'indice e la fixture type è settata a null di default
    struct lookup * l = malloc(sizeof(struct lookup));

    if(!l)
    {
        printf("out of memory");
        exit(0);
    }

    l->nodetype = LOOKUP;
    l->fixtureType = NULL;
    l->var = lookupVar(arrayName);
    l->index = index;
    
    return l;
}

struct ast * newGetChannelValue(struct lookup * lookup, char * channelName)
{
    // La funzione mi permette di inizializzare una struct utilizzata in seguito per prendere i valori del canale. Vi è il nome del canale e la look up di quel specifico canale.
    struct getChannelValue * g = malloc(sizeof(struct getChannelValue));

    if(!g)
    {
        printf("out of memory");
        exit(0);
    }

    g->nodetype = GET_CHANNEL_VALUE;
    g->lookup = lookup;
    g->channelName = channelName;

    return (struct ast *)g;
}

struct ast * newAsgn(struct lookup *l, struct ast *v)
{
    // La funzione newAsgn mi permette di inizializzare una struct per l'assignment.
    struct asgn * a = malloc(sizeof(struct asgn));

    if(!a)
    {
        printf("out of memory");
        exit(0);
    }

    a->nodetype = NEW_ASGN;
    a->lookup = l;
    a->value = v;

    return (struct ast *)a;
}

struct ast * newString(char * string)
{
    // La funzione newString mi permette di inizializzare una struct per la creazione della stringa.
    // Nodetype string e il campo value è uguale alla lunghezza stringa senza il carattere \0
    struct string * s = malloc(sizeof(struct string));

    if(!s)
    {
        printf("out of memory");
        exit(0);
    }

    s->nodetype = STRING_TYPE;
    string[strlen(string) - 1] = '\0';
    s->value = ++string;

    return (struct ast *)s;
}

struct ast * newStringList(struct ast * this, struct ast * next)
{
    struct stringList * sl = malloc(sizeof(struct stringList));

    if(!sl)
    {
        printf("out of memory");
        exit(0);
    }

    sl->this = this;
    sl->next = next == NULL ? NULL : (struct stringList *)next;

    return (struct ast *)sl;
}

struct ast * newPrint(struct ast * a)
{
    // La funzione newPrint mi permette di inizializzare una struct per la creazione della stampa
    struct print * p = malloc(sizeof(struct print));

    if(!p)
    {
        printf("out of memory");
        exit(0);
    }

    p->nodetype = PRINT_TYPE;
    p->a = a;

    return (struct ast *)p;
}

struct ast * newInput()
{
    // La funzione newInput mi permette di inizializzare una struct per gli input.
    // Viene inserito solo il nodetype perchè poi viene svolto tutto durante l'eval
    struct ast * a = malloc(sizeof(struct ast));

    if(!a)
    {
        printf("out of memory");
        exit(0);
    }

    a->nodetype = INPUT_TYPE;

    return a;
}

struct ast * newCreateArray(struct lookup * l, struct astList * al)
{
    // La funzione newCreateArray mi permette di inizializzare una struct per la creazione degli array.
    // Passo la lookup e l'astlist passata come parametro
    struct createArray * ca = malloc(sizeof(struct createArray));

    if(!ca)
    {
        printf("out of memory");
        exit(0);
    }

    ca->nodetype = CREATE_ARRAY;
    ca->lookup = l;
    ca->values = al;

    return (struct ast *)ca;
}