#ifndef LAMB_PARSER_H
#define LAMB_PARSER_H
#include "lexer.h"
#include "ast.h"
// recursive descent parser

struct ParserState {
    struct TokenList* tokens; 
    struct TokenList* prev; // what happens if you use a singly-linked list
    const char* src;
};

struct AST* parse(struct ParserState* parser_state);
struct ParserState* cons_parser(struct TokenList* tv, const char* src);
void parser_free(struct ParserState* ps);

#endif