%{
    #include "parser.tab.h"
%}

%%
.+\..+    { fprintf(stdout, "ogetto\n");
        }
"_"     {
        }
"\n"    { yylval.str = "<br/>\n"; return NEW_LINE; }
.       { yylval.str = strdup(yytext); return ANYTHING; }

%%