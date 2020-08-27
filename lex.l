%{
    #include "parser.tab.h"
%}

%%
"**"    {
        }
"_"     {
        }
"\n"    { yylval.str = "<br/>\n"; return NEW_LINE; }
.       { yylval.str = strdup(yytext); return ANYTHING; }

%%