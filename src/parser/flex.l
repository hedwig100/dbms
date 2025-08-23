
%top {

#include "execute/sql.h"
#include "bison.h"
#include <stdio.h>

}

/****************
** Options (https://westes.github.io/flex/manual/Scanner-Options.html#Scanner-Options)
*****************/

%option noyywrap
%option reentrant
%option warn
%option yylineno
%option case-insensitive
%option never-interactive

%option bison-bridge
%option bison-locations

/****************
** Declarations
****************/

DIGIT [0-9]
ID [_a-zA-Z][_a-zA-Z0-9]*

/****************
** Rules
*****************/
%%

[ \t\n]+

SELECT {return TOKEN_SELECT;}
FROM {return TOKEN_FROM;}
WHERE {return TOKEN_WHERE;}
AS {return TOKEN_AS;}

[<>+=*,;] { return yytext[0]; }

{DIGIT}+ {
    yylval->ival = strtoll(yytext, nullptr, 0);
    return TOKEN_INTEGER_VAL;
}

{ID} {
    yylval->identifier = strdup(yytext);
    return TOKEN_IDENTIFIER;
}

%%

void yyerror(YYLTYPE *llocp, sql::ParseResult* result, yyscan_t scanner, char const *msg) {
    char buffer[100];
    sprintf(buffer, "Failed to parse at L%d,C%d -> L%d,C%d (near ' %s '): %s",
            llocp->first_line, llocp->first_column,
            llocp->last_line, llocp->last_column, yyget_text(scanner), msg);
    result->SetError(buffer);
}


