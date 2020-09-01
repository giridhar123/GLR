#include "headers/astbuilder.h"
#include "headers/sharedVariables.h"
#include "headers/parser.h"

struct fixtureType typetab[NHASH];
struct var vartab[NHASH];
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

struct ast * newFixture(char * fixtureTypeName, char * fixtureName, struct ast * address)
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

struct ast * setChannelValue(char * fixtureName, char * channelName, struct ast * value)
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
    //cv->value = value; // l'indirizzo
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

struct ast * newLoop(char * indexName, double start, double end, struct astList * al)
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
    l->indexName = indexName;
    l->assegnazioni = al;

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

struct ast * newSleep(struct ast * seconds)
{
    struct sleep * s = malloc(sizeof(struct sleep));

    if(!s)
        printf("out of memory");
    
    s->nodetype = SLEEP_TYPE;
    s->seconds = seconds;

    return (struct ast *) s;
}

