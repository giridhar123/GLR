#ifndef PARSER_H
#define PARSER_H

/*
 *  Headers
 */
#include "structs.h"

/*
 *  Methods
 */
//@TODO Servono? 
//struct varlist *newsymlist(struct var *sym, struct varlist *next);
//void symlistfree(struct varlist *sl);

void* startParser(void * params);

extern int yylex_destroy(void);
void yyerror(const char *s, ...);
unsigned int varhash(char * var);

double eval(struct ast *a);
struct var * lookupVar(char * name);
struct fixtureType * lookupFixtureType(char * name);
void parseFile(char * fileName);

void* fadeEval(void* params);
void* delayEval(void * params);
void sleepEval(struct sleep * s);

void setChannelValueEval(struct setChannelValue * setChannelValue);
void newFixtureEval(struct newFixture * newFixture);

int getChannelAddress(struct fixtureType * fixtureType, char * channelName);
int getNumberOfChannels(struct fixtureType * fixtureType);

void createArrayEval(struct createArray * createArray);

struct macro * lookupMacro(char * name);
void macroCallEval(struct macro * m);

int createFixture(struct fixtureType * fixtureType, int startAddress, struct var * fixture);


#endif