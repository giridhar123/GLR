#ifndef PASERUTILS_H
#define PASERUTILS_H

unsigned int varhash(char * var);
struct fixtureType * lookupFixtureType(char * name);
struct var * lookupVar(char * name);
void parseFile(char * fileName);
int getChannelAddress(struct fixtureType * fixtureType, char * channelName);
int getNumberOfChannels(struct fixtureType * fixtureType);

int addFixtureType(struct fixtureType * fixtureType);

struct macro * lookupMacro(char * name);
int createFixture(struct fixtureType * fixtureType, int startAddress, struct var * fixture);
struct evaluated * getEvaluatedFromDouble(double value);
struct evaluated * getEvaluatedFromString(char * value);
struct evaluated * getEvaluatedFromInt(int value);

#endif