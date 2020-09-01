#ifndef SHAREDVARIABLES_H
#define SHAREDVARIABLES_H

/*
 *  Constants
 */
#define DEBUG 1
#define NHASH 9997
#define NHASH 9997
#define DEBUG 1

/*
 *  Variables
 */
extern FILE *yyin;
extern int yylineno; /* from lexer */

extern struct fixtureType typetab[NHASH];
extern struct var vartab[NHASH];
extern unsigned char dmxUniverse[513];

#endif