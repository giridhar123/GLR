#ifndef FREE_H
#define FREE_H

#include "structs.h"


void freeEverything();

void freeAst(struct ast * ast);
void freeNumval(struct numval * n);
void freeGetChannelValue(struct getChannelValue * g);
void freeLookup(struct lookup * l);
void freeString(struct string * s);


void freeFixtureType(struct fixtureType * fixtureType);
void freeChannelList(struct channelList * channelList);
void freeChannel(struct channel * channel);
void freeSetChannelValue(struct setChannelValue * setChannelValue);
void freeNewFixture(struct newFixture * newFixture);
void freeLoop(struct loop * loop);
void freeAstList(struct astList * astList);

void freeVariable(struct var * variable);
void freeArray(struct array * array);



void myFree(void * pt);
void freeMacro(struct macro * m);
void freePrint(struct print * p);
void freeIf(struct ifStruct * ifStruct);
void freeCompare(struct compare * c);
void freeAsgn(struct asgn * a);
void freeFade(struct fade * f);
void freeSleep(struct sleep * s);

#endif