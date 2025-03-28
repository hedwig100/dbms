%code requires {

#include "sql.h"
#include <string>
#include <stdio.h>
#include <iostream>

/* Flex-related declaration */
typedef void* yyscan_t;

#define YY_USER_ACTION                        \
  yylloc->first_line = yylloc->last_line;     \
  yylloc->first_column = yylloc->last_column; \
  for (int i = 0; yytext[i] != '\0'; i++) {   \
    if (yytext[i] == '\n') {                  \
      yylloc->last_line++;                    \
      yylloc->last_column = 0;                \
    } else {                                  \
      yylloc->last_column++;                  \
    }                                         \
  }
}

%define api.pure full

// (https://www.gnu.org/software/bison/manual/html_node/Location-Type.html)
%locations
%initial-action {
  // Initialize
  @$.first_column = 0;
  @$.last_column = 0;
  @$.first_line = 0;
  @$.last_line = 0;
};

// Define additional parameters for yylex (http://www.gnu.org/software/bison/manual/html_node/Pure-Calling.html)
%lex-param   { yyscan_t scanner }

// Define additional parameters for yyparse
%parse-param { sql::ParseResult* result }
%parse-param { yyscan_t scanner }

/************
** Semantic values (https://www.gnu.org/software/bison/manual/html_node/Union-Decl.html)
*************/

%union {
    int ival;
    char *identifier;

    sql::SelectStatement *select_statement;
}


// Destructor (https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html)
%destructor {} <ival>
%destructor { delete($$); } <*>


/************
** Semantic Value type and the corresponding token or nterm
*************/
%define api.token.prefix {TOKEN_}

/* Terminal types (https://www.gnu.org/software/bison/manual/html_node/Token-Decl.html) */
%token <ival> INTEGER_VAL
%token <identifier> IDENTIFIER

%token SELECT FROM

/* Non-terminal symbols (https://www.gnu.org/software/bison/manual/html_node/Type-Decl.html) */
%type <select_statement> select_statement

%code provides {
/* Flex-related declaration */
extern int yylex(YYSTYPE *, YYLTYPE *, yyscan_t);
extern void yyerror(YYLTYPE *, sql::ParseResult *, yyscan_t, char const *);
}

/**********
** Grammer Definitions
**********/
%%

input
    : select_statement
    ;

select_statement
    : SELECT column FROM table ';'
    ;

column
    : INTEGER_VAL
    | IDENTIFIER
    ;

table
    : IDENTIFIER
    ;

%%




