#include "headers/free.h"
#include "headers/sharedVariables.h"

//Per ogni cella degli array globali, libera la memoria allocata 
void freeEverything()
{
    if (DEBUG)
        printf("\n\nInizio a liberare la memoria...\n");

    for (int i = 0; i < NHASH; ++i)
    {
        freeVariable(&vartab[i]);
        freeFixtureType(typetab[i]);
        freeMacro(macrotab[i]);
    }

    for (int i = 0; i < N_THREADS; ++i)
        myFree(DmxName[i]);

    if (DEBUG)
        printf("Memoria liberata!\n");

    exit(0);
}

void freeStmt(struct ast * ast)
{
    if(ast->nodetype != MACRO_TYPE &&
        ast->nodetype != FADE_TYPE &&
        ast->nodetype != DELAY_TYPE)
        freeAst(ast);
}

void freeAst(struct ast * ast)
{
    if (ast == NULL)
        return;

    switch (ast->nodetype)
    {
        case PLUS:
        case CONCAT:
        case MINUS:
        case MUL:
        case DIV:
        case MOD:
            freeAst(ast->l);
            freeAst(ast->r);
        break;
        case NUM:
        {
            struct numval * n = (struct numval *)ast;
            freeNumval(n);
        }
        break;
        case GET_CHANNEL_VALUE:
        {
            struct getChannelValue * g = (struct getChannelValue *) ast;
            freeGetChannelValue(g);
        }
        break;
        case LOOKUP:
        {
            struct lookup * l = (struct lookup *) ast;
            freeLookup(l);
        }
        break;
        case STRING_TYPE:
        {
            struct string * s = (struct string *) ast;
            freeString(s);
        }
        break;
        case INPUT_TYPE:
            free(ast);
        break;
        case PRINT_TYPE:
        {   
            struct print * p = (struct print *)ast;
            freePrint(p);
        }
        break;
        case SET_CHANNEL_VALUE:
        {
            struct setChannelValue * s = (struct setChannelValue *)ast;
            freeSetChannelValue(s);
        }
        break;
        case LOOP_TYPE:
        {
            struct loop * l = (struct loop *)ast;
            freeLoop(l);
        }
        break;
        case IF_TYPE:
        {
            struct ifStruct * ifStruct = (struct ifStruct *)ast;
            freeIf(ifStruct);
        }
        break;
        case COMPARE:
        {
            struct compare * c = (struct compare *)ast;
            freeCompare(c);
        }
        break;
        case NEW_ASGN:
        {
            struct asgn * asgn = (struct asgn *)ast;
            freeAsgn(asgn);
        }
        break;
        case FADE_TYPE:
        case DELAY_TYPE:
        {
            struct fade * f = (struct fade *)ast;
            freeFade(f);
        }
        break;
        case SLEEP_TYPE:
        {
            struct sleep * s = (struct sleep *)ast;
            freeSleep(s);
        }
        break;
        case MACRO_CALL:
        {
            struct macroCall * m = (struct macroCall *)ast;
            freeMacroCall(m);
        }
        break;
        case NEW_FIXTURE:
        {
            struct newFixture * nf = (struct newFixture *)ast;
            freeNewFixture(nf);
        }
        break;
        case CREATE_ARRAY:
        {
            struct createArray * ca = (struct createArray *)ast;
            freeCreateArray(ca);
        }
        break;
        default:
            printf("ERROR: Nodetype has not valid free: %d\n", ast->nodetype);
        break;
    }
}

void freeNumval(struct numval * n)
{
    if (n != NULL)
        free(n);
}

void freeGetChannelValue(struct getChannelValue * g)
{
    if (g == NULL)
        return;

    myFree(g->channelName);
    freeLookup(g->lookup);
    free(g);
}

void freeLookup(struct lookup * l)
{
    if (l == NULL)
        return;

    freeAst(l->index);
    free(l);
}

void freeString(struct string * s)
{
    if (s == NULL)
        return;

    free(s);
}

void freeFixtureType(struct fixtureType * fixtureType)
{
    if (fixtureType == NULL)
        return;

    myFree(fixtureType->name);
    freeChannelList(fixtureType->cl);
    free(fixtureType);
}

void freeChannelList(struct channelList * channelList)
{
    if (channelList == NULL)
        return;

    freeChannel(channelList->channel);
    freeChannelList(channelList->next);
    myFree(channelList);
}

void freeChannel(struct channel * channel)
{
    if (channel == NULL)
        return;

    myFree(channel->name);
    myFree(channel);
}

void freeVariable(struct var * var)
{
    if (var == NULL)
        return;

    var->nodetype = -1;
    var->varType = -1;
    myFree(var->name);
    var->intValue = 0;
    var->doubleValue = 0;
    myFree(var->stringValue);
    var->fixtureType = NULL;
    
    freeArray(var->array);
    var->array = NULL;
}

void freeArray(struct array * array)
{
    if (array == NULL)
        return;
    
    freeVariable(array->var);
    freeArray(array->next);
    myFree(array);
}

void freeSetChannelValue(struct setChannelValue * s)
{
    if (s == NULL)
        return;

    freeLookup(s->lookup);
    myFree(s->channelName);
    freeAst(s->value);
    free(s);
}

void myFree(void * pt)
{
    if(pt != NULL)
        free(pt);
}

void freeNewFixture(struct newFixture * newFixture)
{
    myFree(newFixture->fixtureTypeName);
    myFree(newFixture->lookup);
    myFree(newFixture->address);
    myFree(newFixture);
}

void freeLoop(struct loop * loop)
{
    if (loop == NULL)
        return;

    myFree(loop->indexName);
    freeAst(loop->start);
    freeAst(loop->end);
    freeAstList(loop->stmtList);
    free(loop);
}

void freeAstList(struct astList * astList)
{
    if (astList == NULL)
        return;

    freeAst(astList->this);
    freeAstList(astList->next);
    free(astList);
}

void freeMacro(struct macro * m)
{
    if(m == NULL)
        return;
    
    myFree(m->macroName);
    freeAstList(m->instruction);
    free(m);
}

void freeMacroCall(struct macroCall * m)
{
    if(m == NULL)
        return;

    myFree(m->name);
}

void freePrint(struct print * p)
{
    if (p == NULL)
        return;

    freeAst(p->a);
    free(p);
}

void freeIf(struct ifStruct * ifStruct)
{
    if (ifStruct == NULL)
        return;

    freeAst(ifStruct->cond);
    freeAstList(ifStruct->thenStmt);
    freeAstList(ifStruct->elseStmt);
    free(ifStruct);
}

void freeCompare(struct compare * c)
{
    if (c == NULL)
        return;

    freeAst(c->left);
    freeAst(c->right);
    free(c);
}

void freeAsgn(struct asgn * a)
{
    if (a == NULL)
        return;

    freeLookup(a->lookup);
    freeAst(a->value);
    free(a);
}

void freeFade(struct fade * f)
{
    if (f == NULL)
        return;

    myFree(f->channelName);
    freeAst(f->value);
    freeAst(f->time);
    free(f);
}

void freeSleep(struct sleep * s)
{
    if (s == NULL)
        return;

    freeAst(s->seconds);
    free(s);
}

void freeEvaluated(struct evaluated * evalu)
{
    if (evalu->type == STRING_VAR)
        myFree(evalu->stringVal);

    evalu->stringVal = NULL;
}

void freeCreateArray(struct createArray * ca)
{
    if (ca == NULL)
        return;

    freeLookup(ca->lookup);
    freeAstList(ca->values);
    free(ca);
}