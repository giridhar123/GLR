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

struct ast * newChannel(double address, char * name)
{ 
    //la funzione NewChannel serve per inserire un nuovo nale nell ast
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

    return (struct ast *) cl;
}

struct ast * newFixtureType(char * name, struct ast * cl)
{
    //la funzione newDefine serve per definire una nuova fixturetype da terminale
    struct ast * a = malloc(sizeof(struct ast));

    if(!a) 
    {
        yyerror("out of space");
        exit(0);
    }
    
    a->nodetype = FIXTURE_TYPE;

    struct fixtureType * ft = malloc(sizeof(struct fixtureType));
    ft->name = strdup(name);
    ft->cl = (struct channelList *) cl;
    addFixtureType(ft);
    return a;
}

struct ast * newFixture(char * fixtureTypeName, struct lookup * lookup, struct ast * address)
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

    nf->nodetype = NEW_FIXTURE; // il tipo di nodo
    nf->fixtureTypeName = fixtureTypeName; // la fixtureType
    nf->lookup = lookup; // la variabile
    nf->address = address; // l'indirizzo

    return (struct ast * ) nf;
}

struct ast * newSetChannelValue(struct lookup * lookup, char * channelName, struct ast * value)
{
    //La funzione setChannelValue imposta il valore di un determinato canale.
    struct setChannelValue * cv = malloc(sizeof(struct setChannelValue));

    if(!cv) {
        printf("out of memory");
        exit(0);
    }
    
    cv->nodetype = SET_CHANNEL_VALUE; // tipologia del nodo
    cv->lookup = lookup; // la fixture
    cv->channelName = channelName; // il nome del canale
    cv->value = value;

    return (struct ast *) cv;
}

struct astList * newAstList(struct ast * this, struct astList * next)
{
    //Creo una nuova astlist. next punta o a null oppure ad un'altra astlist passata come parametro
    struct astList * al = malloc(sizeof(struct astList));

    al->this = this;
    al->next = next;

    return al;
}

struct ast * newLoop(char * indexName, struct ast * start, struct ast * end, struct astList * stmtList)
{
    //@todo da capire prima di commentare
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
    
    //In base al segno devo fare un'operazione oppure l'altra
      /* comparisons */
   
    cmp->nodetype = COMPARE;
    cmp->left = left;
    cmp->right = right;
    cmp->cmp = cmptype;
    
    return (struct ast *) cmp ;
}

struct ast * newFade(struct lookup * lookup, char * channelName, struct ast * value, struct ast * time)
{
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

struct astList * AstToAstList(struct ast * a)
{
    //La funzione permette di ritornare un AstList passandogli un Ast, utilizzata negli IF. 
    struct astList *b = malloc(sizeof(struct astList));
    b->this = a ;
    b->next = NULL ;
    return b;

}

struct ast * newSleep(struct ast * seconds)
{
    struct sleep * s = malloc(sizeof(struct sleep));

    if(!s)
        printf("out of memory");
    
    s->nodetype = SLEEP_TYPE;
    s->seconds = seconds;

    return (struct ast *) s;
}

struct ast * newMacroDefine(char * name, struct astList * instructions)
{
    struct macro * m = malloc(sizeof(struct macro));

    if(!m)
        printf("out of memory");

    m->nodetype = MACRO_TYPE;
    m->macroName = strdup(name);
    m->instruction = instructions;

    //struct macro * existingMacro = &macrotab[varhash(name)%NHASH];
    
    //@TODO ma a che serve?	
    /*
    int scount = NHASH;	
    while(--scount >= 0)
    {
        if(++existingMacro >= macrotab+NHASH)
            existingMacro = macrotab; 
    }*/

    macrotab[varhash(m->macroName) % NHASH] = m;

    printf("Ho creato una macro di nome: %s\n",  macrotab[varhash(m->macroName) % NHASH]->macroName);

    return (struct ast *)m;
}

struct ast * newMacroCall(char * name)
{
    struct macro * m = malloc(sizeof(struct macro));

     if(!m)
        printf("out of memory");

    m->nodetype = MACRO_CALL;
    m->macroName = strdup(name);
    m->instruction = NULL;

    return (struct ast *)m;

}

struct lookup * newLookup(char * name)
{
    struct lookup * l = malloc(sizeof(struct lookup));

    if(!l)
    {
        printf("out of memory");
        exit(0);
    }

    l->nodetype = LOOKUP;
    l->fixtureType = lookupFixtureType(name);
    l->var = l->fixtureType == NULL ? lookupVar(name) : NULL;
    l->index = NULL;
    
    return l;
}

struct lookup * newLookupFromArray(char * arrayName, struct ast * index)
{
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
    struct string * s = malloc(sizeof(struct string));

    if(!s)
    {
        printf("out of memory");
        exit(0);
    }

    s->nodetype = STRING_TYPE;
    string[strlen(string) - 1] = 0;
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