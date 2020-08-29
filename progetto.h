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

//Serial port
#include <termios.h>
#include <sys/ioctl.h>
//#include <IOKit/serial/ioss.h>

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
    int nodetype;			/* type K */
    double number;
};

struct invoke {
    int nodetype;			/* type I */
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
    int nodetype;   /* type F */
    char * name;
    struct channelList * cl;
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

void newFixture(char * fixtureTypeName, char * fixtureName, double address);
void setChannelValue(char * fixtureName, char * channelName, double value);

/*
 * METHODS
 */
extern int yylex_destroy(void);

void yyerror(char *s, ...);
extern int yylineno; /* from lexer */

void* startDMX(void * params);
void* startParser(void * params);
void parseFile(char * fileName);