#ifndef FREE_H
#define FREE_H

#include "structs.h"


void freeEverything();

void freeExpr(struct ast * ast);
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
void freeArrayList(struct array * al);
void freeMacro(struct macro * m);

#endif