#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "progetto.h"
    
int main (int argc, char ** argv)
{ 
    pthread_t serialPortThread, parser;

    //pthread_create(&serialPortThread, NULL, &startDMX, NULL);
    pthread_create(&parser, NULL, &startParser, NULL);
    
    //Join solo sul parser, se quest'ultimo termina, termina anche la serial port
    pthread_join(parser, NULL);

    return 0;
}

void yyerror(const char *s)
{
	printf("ERROR: %s\n", s);
}

void* startDMX(void * params)
{
    for (int i = 0; i < 5; ++i)
    {
        fprintf(stdout, "Thread: %d\n", i);
        sleep(2);
    }

    return NULL;
}

void* startParser(void * params)
{
    if(!yyparse())
        printf("\nParsing complete\n");
    else
        printf("\nParsing failed\n");
}

struct ast * newast(int nodetype, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast * newnum(double d)
{
  struct numval *a = malloc(sizeof(struct numval));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'K';
  a->number = d;
  return (struct ast *)a;
}

double eval(struct ast *a)
{
  double v;

  if(!a) {
    yyerror("internal error, null eval");
    return 0.0;
  }

  switch(a->nodetype) {
    /* constant */
  case 'K': v = ((struct numval *)a)->number; break;

  
    /* name reference */
  //case 'N': v = ((struct symref *)a)->s->value; break;

    /* assignment */
/*
  case '=': v = ((struct symasgn *)a)->s->value =
      eval(((struct symasgn *)a)->v); break;
      */

    /* expressions */
  case '+': v = eval(a->l) + eval(a->r); break;
  case '-': v = eval(a->l) - eval(a->r); break;
  case '*': v = eval(a->l) * eval(a->r); break;
  case '/': v = eval(a->l) / eval(a->r); break;
  /*
  case '|': v = fabs(eval(a->l)); break;
  case 'M': v = -eval(a->l); break;
  */

    /* comparisons */
    /*
  case '1': v = (eval(a->l) > eval(a->r))? 1 : 0; break;
  case '2': v = (eval(a->l) < eval(a->r))? 1 : 0; break;
  case '3': v = (eval(a->l) != eval(a->r))? 1 : 0; break;
  case '4': v = (eval(a->l) == eval(a->r))? 1 : 0; break;
  case '5': v = (eval(a->l) >= eval(a->r))? 1 : 0; break;
  case '6': v = (eval(a->l) <= eval(a->r))? 1 : 0; break;
  */

  /* control flow */
  /* null if/else/do expressions allowed in the grammar, so check for them */
  /*
  case 'I': 
    if( eval( ((struct flow *)a)->cond) != 0) {
      if( ((struct flow *)a)->tl) {
	v = eval( ((struct flow *)a)->tl);
      } else
	v = 0.0;
    */
   	/* a default value */
    /*
    } else {
      if( ((struct flow *)a)->el) {
        v = eval(((struct flow *)a)->el);
      } else
	v = 0.0;
    */
    /* a default value */
    /*
    }
    break;

  case 'W':
    v = 0.0;
    */
   	/* a default value */
    /*
    if( ((struct flow *)a)->tl) {
      while( eval(((struct flow *)a)->cond) != 0)
	v = eval(((struct flow *)a)->tl);
    }
    break;
    */
    /* last value is value */
    /*         
  case 'L': eval(a->l); v = eval(a->r); break;

  case 'F': v = callbuiltin((struct fncall *)a); break;

  case 'C': v = calluser((struct ufncall *)a); break;
*/
  default: printf("internal error: bad node %c\n", a->nodetype);
  }
  return v;
}