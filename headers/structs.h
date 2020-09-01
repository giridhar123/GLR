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

//Serial port
#include <termios.h>
#include <sys/ioctl.h>
//#include <IOKit/serial/ioss.h>

#include <signal.h> //SIGINT

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
    SLEEP_TYPE,
    MACRO_TYPE,
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

struct invoke {
    int nodetype;
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
    struct var * fixture;
    struct ast * address;
};

struct setChannelValue
{
    int nodetype;
    struct var * fixture;
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

struct var {		/* a variable name */
    char * name;
    double value;
    struct ast * func;	/* stmt for the function */
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

struct createArray
{
    int nodetype;
    struct fixtureType * fixtureType;
    struct var * array;
    struct ast * size;
    struct ast * startAddress;
};

#endif