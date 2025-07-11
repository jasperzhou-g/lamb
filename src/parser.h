#ifndef LAMB_PARSER_H
#define LAMB_PARSER_H
#include "lexer.h"
#include "ast.h"
// recursive descent parser

struct ParserState {
    struct TokenList* tokens; 
    const char* src;
    int current;
};

struct AST* parse(struct ParserState* parser_state);
struct ParserState* cons_parser(struct TokenList* tv, const char* src);
void parser_free(struct ParserState* ps);

#endif