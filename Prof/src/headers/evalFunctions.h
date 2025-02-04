#ifndef EVALFUNCTIONS_H
#define EVALFUNCTIONS_H

void* fadeEval(void* params);
void* delayEval(void * params);
void sleepEval(struct sleep * s);
void setChannelValueEval(struct setChannelValue * setChannelValue);
void newFixtureEval(struct newFixture * newFixture);
void macroCallEval(struct macroCall * m);
void loopEval(struct loop * l);
struct evaluated lookupEval(struct lookup * l);
void newAsgnEval(struct asgn * asg);
void createArrayEval(struct createArray * createArray);
void deleteVar(struct var * var); 
void deleteMac(struct lookup *lookup );

#endif