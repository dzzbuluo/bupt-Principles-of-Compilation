%{
#include "y.tab.h"
#include <stdlib.h>
%}

%%

[0-9]+      {
                yylval = atoi(yytext);
                return NUM;
            }

[ \t\r\n]+  ;

"("         return '(';
")"         return ')';
"+"         return '+';
"-"         return '-';
"*"         return '*';
"/"         return '/';

.           return yytext[0];

%%

int yywrap(void) {
    return 1;
}
