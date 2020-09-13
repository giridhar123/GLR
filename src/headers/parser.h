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
struct evaluated eval(struct ast *a);
struct evaluated evalExpr(struct ast * a);


#endif