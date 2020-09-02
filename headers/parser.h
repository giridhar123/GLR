#ifndef PARSER_H
#define PARSER_H

/*
 *  Headers
 */
#include "structs.h"

/*
 *  Methods
 */
//@TODO Servono? 
//struct varlist *newsymlist(struct var *sym, struct varlist *next);
//void symlistfree(struct varlist *sl);

void* startParser(void * params);

extern int yylex_destroy(void);
void yyerror(const char *s, ...);
int yyparse();
double eval(struct ast *a);



#endif