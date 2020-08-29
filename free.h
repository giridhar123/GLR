#ifndef FREE_H
#define FREE_H

#include "progetto.h"

void freeEverything();
void freeFixtureType(struct fixtureType * fixtureType);
void freeChannelList(struct channelList * channelList);
void freeChannel(struct channel * channel);

void freeVariable(struct var * variable);
void freeVarList(struct varlist * vars);

void freeAst(struct ast * ast);

#endif