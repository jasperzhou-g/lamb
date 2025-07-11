#ifndef LAMB_PARSER_H
#define LAMB_PARSER_H
#include "lexer.h"
#include "ast.h"
// recursive descent parser

struct ParserState {
    struct TokenList* tokens; 
    int current;
};

struct AST parse(struct TokenList tl);

#endif