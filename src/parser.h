#ifndef LAMB_PARSER_H
#define LAMB_PARSER_H
#include "lexer.h"
#include "ast.h"
// recursive descent parser

struct Parser {
    struct TokenList* tokens; 
    struct TokenList* prev; // what happens if you use a singly-linked list
    const char* src;
};

struct AST* parse(struct Parser* parser_state);
struct Parser* parser_init(struct TokenList* tv, const char* src);
void parser_free(struct Parser* ps);

#endif