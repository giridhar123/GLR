#define DEBUG 1

/* symbol table */
struct symbol {		/* a variable name */
  char *name;
  double value;
  struct ast *func;	/* stmt for the function */
  struct symlist *syms; /* list of dummy args */
  struct fixtureType *fixtureType; /* a fixture */
};

/* list of symbols, for an argument list */
struct symlist {
  struct symbol *sym;
  struct symlist *next;
};

struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);

struct ast {
  int nodetype;
  struct ast *l;
  struct ast *r;
};

struct numval {
  int nodetype;			/* type K */
  double number;
};

struct symref {
  int nodetype;			/* type N */
  struct symbol *s;
};

struct channel
{
  char * name;
  int address;
};

struct channelList
{
  struct channel * channel;
  struct channelList * next;
};

struct fixtureType
{
  int nodetype;   /* type F */
  char * name;
  struct channelList * cl;
};

/* simple symtab of fixed size */
#define NHASH 9997
struct symbol symtab[NHASH];
struct fixtureType typetab[NHASH];

void yyerror(char *s, ...);
extern int yylineno; /* from lexer */

void* startDMX(void * params);
void* startParser(void * params);

static unsigned symhash(char * sym);
struct symbol * lookupSymbol(char * name);
struct fixtureType * lookupFixtureType(char * name);
struct ast * newref(struct symbol *s);

double eval(struct ast *a);
struct ast * newast(int nodetype, struct ast *l, struct ast *r);
struct ast * newnum(double d);

struct ast * newChannel(double address, char * name);
struct ast * newChannelList (struct ast * c, struct ast * otherList);
struct ast * newDefine(char * name, struct ast * cl);

void newFixture(char * fixtureTypeName, char * fixtureName, double address);