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

    freeAst(astList->this);
    freeAstList(astList->next);
    free(astList);
}

void freeAst(struct ast * ast)
{
    if (ast == NULL)
        return;

    switch (ast->nodetype)
    {
        case AST:
            freeAst(ast->l);
            freeAst(ast->r);
        break;

        case NUM: break;

        case FIXTURE_TYPE:
            freeFixtureType((struct fixtureType *) ast);
        break;

        case NEW_FIXTURE:
            freeNewFixture((struct newFixture *) ast);
        break;

        case SET_CHANNEL_VALUE:
            freeSetChannelValue((struct setChannelValue *) ast);
        break;

        case LOOP_TYPE:
            freeLoop((struct loop *) ast);
        break;

    }
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