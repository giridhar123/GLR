#ifndef PARSER_H
#define PARSER_H

#define NHASH 9997
#define DEBUG 1

/*
 *  Headers
 */
#include <pthread.h>
#include <stdarg.h>
#include <math.h>

#include "astbuilder.h"

/*
 *  Structs
 */ 
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

/*
 *  Variables
 */
FILE *yyin;
extern int yylineno; /* from lexer */

struct var vartab[NHASH];


/*
 *  Methods
 */
void* startParser(void * params);

extern int yylex_destroy(void);
void yyerror(char *s, ...);

double eval(struct ast *a);
static unsigned int varhash(char * var);
struct var * lookupVar(char * name);
struct fixtureType * lookupFixtureType(char * name);
void parseFile(char * fileName);

void* fadeEval(void* params);
void* delayEval(void * params);

void setChannelValueEval(struct setChannelValue * setChannelValue);
void newFixtureEval(struct newFixture * newFixture);

#endif