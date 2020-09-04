#ifndef ASTBUILDER_H
#define ASTBUILDER_H

/*
 *  Headers
 */
#include "structs.h"

/*
 *  Methods
 */

struct ast * newast(int nodetype, struct ast *l, struct ast *r);
struct ast * newnum(double d);

struct ast * newChannel(double address, char * name);
struct ast * newChannelList (struct ast * c, struct ast * otherList);
struct ast * newFixtureType(char * name, struct ast * cl);

struct ast * newFixture(char * fixtureTypeName, struct lookup * lookup, struct ast * address);
struct ast * newSetChannelValue(struct lookup * fixture, char * channelName, struct ast * value);

struct astList * newAstList(struct ast * this, struct astList * next);
struct ast * newLoop(char * indexName, double start, double end, struct astList * al);

struct ast * newCompare(int cmptype, struct ast * left, struct ast * right);
struct ast * newFade(struct lookup * fixture, char * channelName, struct ast * value, struct ast * time);
struct ast * newDelay(struct lookup * fixture, char * channelName, struct ast * value, struct ast * time);
struct ast * newIf(struct ast * cond, struct astList * thenStmt, struct astList * elseStmt);
struct ast * newSleep(struct ast * seconds);
struct ast * newMacroDefine(char * name, struct astList * instructions);
struct ast * newMacroCall(char * name);

struct ast * newCreateArray(struct fixtureType * fixtureType, struct var * array, struct ast * size, struct ast * startAddress);

struct lookup * newLookup(char * name);
struct lookup * newLookupFromArray(char * arrayName, struct ast * index);

struct astList * AstToAstList(struct ast * a);

struct ast * newGetChannelValue(struct lookup * lookup, char * channelName);

struct ast * newString(char * string);
struct ast * newStringList(struct ast * this, struct ast * next);
struct ast * newPrint(struct ast * a);

#endif