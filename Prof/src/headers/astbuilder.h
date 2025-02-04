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
struct channel * newChannel(double address, char * name);
struct channelList * newChannelList (struct channel * c, struct channelList * otherList);
struct ast * newFixture(char * fixtureTypeName, struct lookup * lookup, struct ast * address);
struct ast * newSetChannelValue(struct lookup * fixture, char * channelName, struct ast * value);
struct astList * newAstList(struct ast * this, struct astList * next);
struct ast * newLoop(char * indexName, struct ast * start, struct ast * end, struct astList * al);
struct ast * newCompare(int cmptype, struct ast * left, struct ast * right);
struct ast * newFade(struct lookup * fixture, char * channelName, struct ast * value, struct ast * time);
struct ast * newDelay(struct lookup * fixture, char * channelName, struct ast * value, struct ast * time);
struct ast * newIf(struct ast * cond, struct astList * thenStmt, struct astList * elseStmt);
struct ast * newSleep(struct ast * seconds);
struct ast * newMacroCall(char * name);
struct lookup * newLookup(char * name);
struct lookup * newLookupFromArray(char * arrayName, struct ast * index);
struct astList * astToAstList(struct ast * a);
struct ast * newGetChannelValue(struct lookup * lookup, char * channelName);
struct ast * newAsgn(struct lookup *l, struct ast *v);
struct ast * newString(char * string);
struct ast * newStringList(struct ast * this, struct ast * next);
struct ast * newPrint(struct ast * a);
struct ast * newInput();
struct ast * newCreateArray(struct lookup * l, struct astList * al);

#endif