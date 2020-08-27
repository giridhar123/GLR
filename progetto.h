struct ast {
  int nodetype;
  struct ast *l;
  struct ast *r;
};

struct numval {
  int nodetype;			/* type K */
  double number;
};

void yyerror(const char *s);    
void* startDMX(void * params);
void* startParser(void * params);

double eval(struct ast *a);
struct ast * newnum(double d);
struct ast *newast(int nodetype, struct ast *l, struct ast *r);