#ifndef FREE_H
#define FREE_H

#include "structs.h"


void freeEverything();
void freeFixtureType(struct fixtureType * fixtureType);
void freeChannelList(struct channelList * channelList);
void freeChannel(struct channel * channel);
void freeSetChannelValue(struct setChannelValue * setChannelValue);
void freeNewFixture(struct newFixture * newFixture);
void freeLoop(struct loop * loop);
void freeAstList(struct astList * astList);

void freeVariable(struct var * variable);
void freeVarList(struct varlist * vars);

void freeAst(struct ast * ast);

void myFree(void * pt);

#endif