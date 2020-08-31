#ifndef PROGETTO_H
#define PROGETTO_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <string.h>
#include <errno.h> // Error integer and strerror() function
#include <unistd.h> // write(), read(), close()

#include <math.h>

//Serial port
#include <termios.h>
#include <sys/ioctl.h>
//#include <IOKit/serial/ioss.h>

#include <signal.h> //SIGINT
/*
 * CONSTANTS
 */
#define DEBUG 1
#define NHASH 9997

/*
 * VARIABLES
 */
FILE *yyin ;
unsigned char dmxUniverse[513];

/* var table */
struct var {		/* a variable name */
    char *name;
    double value;
    struct ast *func;	/* stmt for the function */
    struct varlist *vars; /* list of dummy args */
    struct fixtureType *fixtureType; /* a fixture */
};

/* list of var, for an argument list */
struct varlist {
    struct var *var;
    struct varlist *next;
};

struct varlist *newsymlist(struct var *sym, struct varlist *next);
void symlistfree(struct varlist *sl);

struct ast {
    int nodetype;
    struct ast *l;
    struct ast *r;
};

struct numval {
    int nodetype;
    double number;
};

struct invoke {
    int nodetype;
    struct fixtureType * ft;
    struct var * v;
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
    int nodetype;
    char * name;
    struct channelList * cl;
};
 
struct newFixture
{
    int nodetype;
    char * fixtureTypeName;
    char * fixtureName;
    double address;
};

struct setChannelValue
{
    int nodetype;
    char * fixtureName;
    char * channelName;
    struct ast * value;
};

struct astList
{
    struct ast * this;
    struct astList * next;
};

struct loop
{
    int nodetype;   /* type L */
    int start;
    int end;
    char * indexName;
    struct astList * assegnazioni;
};

struct fade
{
    int nodetype;
    char * variableName;
    char * channelName;
    int value;
    double time;
};

enum nodetype
{
    AST = 1,
    NUM,
    INVOKE, 
    FIXTURE_TYPE,
    NEW_FIXTURE,
    SET_CHANNEL_VALUE,
    LOOP_TYPE,
    FADE_TYPE,
    DELAY_TYPE,
    COMPARE,
    SEQ,
};

struct compare
{
    int nodetype;
    double value1;
    double value2;
    int cmp;
};

struct seq {
  int nodetype;			/* type I or W */
  struct ast *cond;		/* condition */
  struct ast *th;		/* then istruction */
  struct ast *el;		/* else instruction */
};

/* simple vartab of fixed size */
struct var vartab[NHASH];
struct fixtureType typetab[NHASH];

static unsigned varhash(char * var);
struct var * lookupVar(char * name);
struct fixtureType * lookupFixtureType(char * name);
struct ast * newInvoke(char * name);

double eval(struct ast *a);
struct ast * newast(int nodetype, struct ast *l, struct ast *r);
struct ast * newnum(double d);

struct ast * newChannel(double address, char * name);
struct ast * newChannelList (struct ast * c, struct ast * otherList);
struct ast * newDefine(char * name, struct ast * cl);

struct ast * newFixture(char * fixtureTypeName, char * fixtureName, double address);
void newFixtureEval(struct newFixture * newFixture);
struct ast * setChannelValue(char * fixtureName, char * channelName, struct ast * value);
void setChannelValueEval(struct setChannelValue * setChannelValue);
/*
 * METHODS
 */
extern int yylex_destroy(void);

void yyerror(char *s, ...);
extern int yylineno; /* from lexer */

void* startDMX(void * params);
void* startParser(void * params);
void parseFile(char * fileName);

struct astList * newAstList(struct ast * this, struct astList * next);
struct ast * newLoop(char * indexName, double start, double end, struct astList * al);

struct ast * ifcase(int cmptype, double number1, double number2);
struct ast * newFade(char * variableName, char * channelName, double value, double time);
void* fadeEval(void* params);

struct ast * newDelay(char * variableName, char * channelName, double value, double time);
void* delayEval(void * params);

struct ast * newSeq(struct ast *cond, struct ast *th, struct ast *el);

#endif