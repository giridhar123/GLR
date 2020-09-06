#ifndef EVALFUNCTIONS_H
#define EVALFUNCTIONS_H


struct evaluated * eval(struct ast *a);

void* fadeEval(void* params);
void* delayEval(void * params);
void sleepEval(struct sleep * s);

void setChannelValueEval(struct setChannelValue * setChannelValue);
void newFixtureEval(struct newFixture * newFixture);

void createArrayEval(struct createArray * createArray);

void macroCallEval(struct macro * m);

struct evaluated * lookupEval(struct lookup * l);

void newAsgnEval(struct asgn * asg);
void deleteVar(struct lookup *variable );
void deleteMacro(struct macro *macro);

#endif