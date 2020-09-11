#include "headers/free.h"
#include "headers/sharedVariables.h"

void freeEverything()
{
    if (DEBUG)
        printf("\n\nInizio a liberare la memoria...\n");

    for (int i = 0; i < NHASH; ++i)
    {
        freeVariable(&vartab[i]);
        freeFixtureType(typetab[i]);
    }

    if (DEBUG)
        printf("Memoria liberata!\n");

    exit(0);
}

void freeExpr(struct ast * ast)
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
            freeExpr(ast->l);
            freeExpr(ast->r);
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
        default:
            printf("Nodetype has not valid free: %d", ast->nodetype);
        break;
    }

    printf("Free\n");
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

    free(g->channelName);
    freeLookup(g->lookup);
    free(g);
}

void freeLookup(struct lookup * l)
{
    if (l == NULL)
        return;

    freeExpr(l->index);
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
    
    if (var->array != NULL)
    {
        freeArrayList(var->array);
        var->array = NULL;
    }    
}

void freeArray(struct array * array)
{
    if (array == NULL)
        return;
    
    freeVariable(array->var);
    freeArray(array->next);
    myFree(array);
}

void freeSetChannelValue(struct setChannelValue * setChannelValue)
{
    myFree(setChannelValue);
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
    myFree(newFixture);
}

void freeLoop(struct loop * loop)
{
    if (loop == NULL)
        return;

    myFree(loop->indexName);
    freeAstList(loop->stmtList);
    myFree(loop);
}

void freeAstList(struct astList * astList)
{
    if (astList == NULL)
        return;

    freeExpr(astList->this);
    freeAstList(astList->next);
    free(astList);
}

void freeArrayList(struct array * al)
{
    if (al == NULL)
        return;

    freeVariable(al->var);
    freeArrayList(al->next);
    free(al);
}

void freeMacro(struct macro * m)
{
    if(m == NULL)
        return;
    
    myFree(m->macroName);
    freeAstList(m->instruction);
    free(m);
}