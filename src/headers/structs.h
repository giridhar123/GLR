#ifndef STRUCTS_H
#define STRUCTS_H

/*
 *  Headers
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <string.h>
#include <errno.h> // Error integer and strerror() function
#include <unistd.h> // write(), read(), close(), sleep()

#include <math.h>

#include <signal.h> //SIGINT

/*
 *  Structs
 */ 

enum nodetype
{
    PLUS = 1,
    MINUS,
    MUL,
    DIV,
    MOD,
    CONCAT,
    AST,
    VARIABLE,
    NUM,
    FIXTURE_TYPE,
    NEW_FIXTURE,
    SET_CHANNEL_VALUE,
    LOOP_TYPE,
    FADE_TYPE,
    DELAY_TYPE,
    COMPARE,
    IF_TYPE,
    SLEEP_TYPE,
    MACRO_TYPE,
    MACRO_CALL,
    LOOKUP,
    GET_CHANNEL_VALUE,
    STRING_TYPE,
    PRINT_TYPE,
    PRINT_STRING_TYPE,
    NEW_ASGN,
    INPUT_TYPE,
    CREATE_ARRAY
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
    char * parentName;
    struct channelList * cl;
};
 
struct newFixture
{
    int nodetype;
    char * fixtureTypeName;
    struct lookup * lookup;
    struct ast * address;
};

struct setChannelValue
{
    int nodetype;
    struct lookup * lookup;
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
    struct ast * start;
    struct ast * end;
    char * indexName;
    struct astList * stmtList;
};

struct fade
{
    int nodetype;
    struct var * fixture;
    char * channelName;
    struct ast * value;
    struct ast * time;
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

enum varType
{
    NONE,
    INT_VAR,
    DOUBLE_VAR,
    STRING_VAR,
    ARRAY_VAR,
    FIXTURE_VAR,
    FIXTURE_TYPE_VAR
};

struct var {		/* a variable name */
    int nodetype;
    char * name;
    int varType;
    double doubleValue;
    int intValue;
    char * stringValue;
    struct fixtureType * fixtureType; /* a fixture */
    struct array * array;
};

struct array /* BUT IT'S NOT AN ARRAY ;-) */ {
    struct var * var;
    struct array * next;
    int index;
};

struct sleep {
    int nodetype;
    struct ast * seconds;
};

struct macro {
    int nodetype;
    char * macroName;
    struct astList * instruction;
};

struct lookup
{
    int nodetype;
    struct fixtureType * fixtureType;
    struct var * var;
    struct ast * index;
};

struct getChannelValue
{
    int nodetype;
    struct lookup * lookup;
    char * channelName;
};

struct string
{
    int nodetype;
    char * value;
    int size;
};

struct stringList
{
    struct ast * this;
    struct stringList * next;
};

struct print
{
    int nodetype;
    struct ast * a;
};

struct evaluated
{
    int type;
    double doubleVal;
    int intVal;
    char * stringVal;
};

struct asgn
{
    int nodetype;
    struct lookup * lookup;
    struct ast * value;
};

struct createArray
{
    int nodetype;
    struct lookup * lookup;
    struct astList * values;
};

#endif