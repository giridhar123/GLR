#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <string.h>
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include "progetto.h"
    
int main (int argc, char ** argv)
{ 
    pthread_t serialPortThread, parser;

    pthread_create(&serialPortThread, NULL, &startDMX, NULL);
    pthread_create(&parser, NULL, &startParser, NULL);
    
    //Join solo sul parser, se quest'ultimo termina, termina anche la serial port
    pthread_join(parser, NULL);

    return 0;
}

void yyerror(char *s, ...)
{
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

void* startDMX(void * params)
{
    int serial_port = open("/dev/ttyUSB0", O_RDWR);

    // Check for errors
    if (serial_port < 0) {
        printf("Error %i from open: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    // Create new termios struc, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    struct termios tty;

    // Read in existing settings, and handle any error
    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag |= CSTOPB;  // Set stop field, two stop bits used in communication
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    
    // Set in/out baud rate to be 250000
    cfsetispeed(&tty, 250000);
    cfsetospeed(&tty, 250000);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    unsigned char msg[513];
    for (int i = 0; i < 513; ++i)
      msg[i] = 0;

    msg[4] = msg[6] = 255;

    for (int i = 0; i < 500; ++i)
      write(serial_port, msg, sizeof(msg));

    close(serial_port);
    return NULL;
}

void* startParser(void * params)
{
    if(!yyparse())
        printf("\nParsing complete\n");
    else
        printf("\nParsing failed\n");
}

/* symbol table */
/* hash a symbol */
static unsigned symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while (c = *sym++)
    hash = hash*9 ^ c;

  return hash;
}

struct symbol * lookup(char* sym)
{
  struct symbol *sp = &symtab[symhash(sym)%NHASH];
  int scount = NHASH;		/* how many have we looked at */

  while(--scount >= 0) {
    if (sp->name && !strcmp(sp->name, sym))
    { 
      return sp;
    }

    if(!sp->name) {		/* new entry */
      sp->name = strdup(sym);
      sp->value = 0;
      sp->func = NULL;
      sp->syms = NULL;
      return sp;
    }

    if(++sp >= symtab+NHASH) sp = symtab; /* try the next entry */
  }
  yyerror("symbol table overflow\n");
  abort(); /* tried them all, table is full */

}

struct ast * newref(struct symbol *s)
{
  struct symref *a = malloc(sizeof(struct symref));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'N';
  a->s = s;
  return (struct ast *)a;
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
    case 'K':
      v = ((struct numval *)a)->number;
      break;
    
    /* name reference */
    case 'N':
      v = ((struct symref *)a)->s->value;
      break;

    /* Fixture type - Define */
    case 'F':
      printf("Hai definito un nuovo tipo: %s\n", ((struct fixtureType *)a)->name);
      v = 0;
    break;

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

struct ast * newChannel(double address, char * name)
{ 
  struct channel *c = malloc(sizeof(struct channel));
  
  if(!c) {
    yyerror("out of space");
    exit(0);
  }

  c->name = name;
  c->address = (int) address;

  return (struct ast *)c;
}

struct ast * newChannelList (struct ast * c, struct ast * otherList)
{
  struct channelList * cl = malloc(sizeof(struct channelList));

  if(!cl) {
    yyerror("out of space");
    exit(0);
  }

  cl->channel = (struct channel *) c;
  cl->next = (struct channelList *) otherList;

  struct channelList * tmp = cl;

  return (struct ast *) cl;
}

struct ast * newDefine(char * name, struct ast * cl)
{
  struct fixtureType * f = malloc(sizeof(struct fixtureType));
  
  if(!f) {
    yyerror("out of space");
    exit(0);
  }

  f->nodetype = 'F';
  f->name = name;
  f->cl = (struct channelList *) cl;

  return (struct ast *) f;
}