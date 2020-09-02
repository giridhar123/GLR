#ifndef EVALFUNCTIONS_H
#define EVALFUNCTIONS_H


double eval(struct ast *a);

void* fadeEval(void* params);
void* delayEval(void * params);
void sleepEval(struct sleep * s);

void setChannelValueEval(struct setChannelValue * setChannelValue);
void newFixtureEval(struct newFixture * newFixture);

void createArrayEval(struct createArray * createArray);

void macroCallEval(struct macro * m);

#endif