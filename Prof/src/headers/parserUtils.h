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
void createFixtureArray(struct fixtureType * fixtureType, int startAddress, struct lookup * lookup);

struct evaluated getEvaluatedFromDouble(double value);
struct evaluated getEvaluatedFromString(char * value);
struct evaluated getEvaluatedFromInt(int value);

void printAllFixtures();
void setColor(char * color);

void connectDmx(char * port);
void disconnectDmx(char * port);

void newMacroDefine(char * name, struct astList * instructions);
void newFixtureType(char * name, struct channelList * cl, char * parentName);

char * intToString(int value);
char * doubleToString(double value);

#endif