#ifndef ASTBUILDER_H
#define ASTBUILDER_H

/*
 *  Headers
 */
#include "structs.h"

/*
 *  Methods
 */

struct ast * newInvoke(char * name);
struct ast * newast(int nodetype, struct ast *l, struct ast *r);
struct ast * newnum(double d);

struct ast * newChannel(double address, char * name);
struct ast * newChannelList (struct ast * c, struct ast * otherList);
struct ast * newDefine(char * name, struct ast * cl);

struct ast * newFixture(char * fixtureTypeName, char * fixtureName, struct ast * address);
struct ast * setChannelValue(char * fixtureName, char * channelName, struct ast * value);

struct astList * newAstList(struct ast * this, struct astList * next);
struct ast * newLoop(char * indexName, double start, double end, struct astList * al);

struct ast * newCompare(int cmptype, struct ast * left, struct ast * right);
struct ast * newFade(char * variableName, char * channelName, double value, double time);
struct ast * newDelay(char * variableName, char * channelName, double value, double time);
struct ast * newIf(struct ast * cond, struct astList * thenStmt, struct astList * elseStmt);

#endif