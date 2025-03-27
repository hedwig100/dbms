%code requires {

#include "sql.h"
#include <string>
#include <stdio.h>
#include <iostream>

/* Flex-related declaration */
typedef void* yyscan_t;
}

%define api.pure full

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
extern int yylex(YYSTYPE *, yyscan_t);
extern void yyerror(sql::ParseResult *, yyscan_t, char const *);
}

/**********
** Grammer Definitions
**********/
%%

input
    : select_statement
    ;

select_statement
    : SELECT column FROM table ';' { std::cerr << "SELECT!!\n"; }
    ;

column
    : INTEGER_VAL { std::cerr << "integer!!\n"; }
    | IDENTIFIER { std::cerr << "ident!!\n"; }
    ;

table
    : IDENTIFIER { std::cerr << "ident!!\n"; }
    ;

%%




