#ifndef ASTBUILDER_H
#define ASTBUILDER_H


/*
 *  Headers
 */
#include <stdlib.h>
/*
 *  Variables
 */
struct fixtureType typetab[NHASH];

/*
 *  Structs
 */ 

enum nodetype
{
    AST = 1,
    NUM,
    INVOKE, 
    FIXTURE_TYPE,
    NEW_FIXTURE,
    SET_CHANNEL_VALUE,
    LOOP_TYPE,
    FADE_TYPE,
    DELAY_TYPE,
    COMPARE,
    IF_TYPE,
};

struct ast {
    int nodetype;
    struct ast *l;
    struct ast *r;
};

struct numval {
    int nodetype;
    double number;
};

struct invoke {
    int nodetype;
    struct fixtureType * ft;
    struct var * v;
};

struct channel
{
    char * name;
    int address;
};

struct channelList
{
    struct channel * channel;
    struct channelList * next;
};

struct fixtureType
{
    int nodetype;
    char * name;
    struct channelList * cl;
};
 
struct newFixture
{
    int nodetype;
    char * fixtureTypeName;
    char * fixtureName;
    double address;
};

struct setChannelValue
{
    int nodetype;
    char * fixtureName;
    char * channelName;
    struct ast * value;
};

struct astList
{
    struct ast * this;
    struct astList * next;
};

struct loop
{
    int nodetype;   /* type L */
    int start;
    int end;
    char * indexName;
    struct astList * assegnazioni;
};

struct fade
{
    int nodetype;
    char * variableName;
    char * channelName;
    int value;
    double time;
};

struct compare
{
    int nodetype;
    struct ast * left;
    struct ast * right;
    int cmp;
};

struct ifStruct {
  int nodetype;			/* type I or W */
  struct ast * cond;		/* condition */
  struct astList * thenStmt;		/* then istruction */
  struct astList * elseStmt;		/* else instruction */
};




//////////////


struct ast * newast(int nodetype, struct ast *l, struct ast *r);
struct ast * newnum(double d);

struct ast * newChannel(double address, char * name);
struct ast * newChannelList (struct ast * c, struct ast * otherList);
struct ast * newDefine(char * name, struct ast * cl);

struct ast * newFixture(char * fixtureTypeName, char * fixtureName, double address);
struct ast * setChannelValue(char * fixtureName, char * channelName, struct ast * value);

struct astList * newAstList(struct ast * this, struct astList * next);
struct ast * newLoop(char * indexName, double start, double end, struct astList * al);

struct ast * newCompare(int cmptype, struct ast * left, struct ast * right);
struct ast * newFade(char * variableName, char * channelName, double value, double time);
struct ast * newDelay(char * variableName, char * channelName, double value, double time);
struct ast * newIf(struct ast * cond, struct astList * thenStmt, struct astList * elseStmt);

#endif