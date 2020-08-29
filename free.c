#include "free.h"

void freeEverything()
{
    if (DEBUG)
        printf("\n\nInizio a liberare la memoria...\n");

    for (int i = 0; i < NHASH; ++i)
    {
        freeVariable(&vartab[i]);
        freeFixtureType(&typetab[i]);
    }

    if (DEBUG)
        printf("Memoria liberata!\n");

    exit(0);
}

void freeFixtureType(struct fixtureType * fixtureType)
{
    if (fixtureType->name == NULL)
        return;

    free(fixtureType->name);
    freeChannelList(fixtureType->cl);
}

void freeChannelList(struct channelList * channelList)
{
    if (channelList == NULL)
        return;

    freeChannel(channelList->channel);
    freeChannelList(channelList->next);
    free(channelList);
}

void freeChannel(struct channel * channel)
{
    if (channel->name == NULL)
        return;

    free(channel->name);
    free(channel);
}

void freeVariable(struct var * variable)
{
    if (variable->name == NULL)
        return;

    free(variable->name);
    freeAst(variable->func);
    freeVarList(variable->vars);
}

void freeVarList(struct varlist * vars)
{
    if (vars == NULL)
        return;
    
    freeVariable(vars->var);
    freeVarList(vars->next);
    free(vars);
}

void freeAst(struct ast * ast)
{
    if (ast == NULL)
        return;

    switch (ast->nodetype)
    {
        
    }
}