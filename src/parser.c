#include <stdbool.h>
#include "lexer.h"
#include "parser.h"

static struct Token ps_peek(struct ParserState* ps) {
} 

static struct Token ps_advance(struct ParserState* ps) {

}

static bool ps_check(struct ParserState* ps, enum TokenType t) {
}

static bool ps_match(struct ParserState* ps, enum TokenType t) {
    bool b = ps_check(ps, t);
    // advance
    return b;
}

static struct AST* parse_num(struct ParserState* ps) {

}

static struct AST* parse_identifier(struct ParserState* ps) {

}

static struct AST* parse_abs(struct ParserState* ps) {

}

static struct AST* parse_app(struct ParserState *ps) {

}

struct ParserState* cons_parser(struct TokenList* tv, const char* src) {
    struct ParserState* ps = malloc(sizeof(struct ParserState));
    ps->current = 0;
    ps->src = src;
    ps->tokens = tv;
    return ps;
}

void parser_free(struct ParserState* ps) {
    free(ps);
}

struct AST* parse(struct ParserState* ps) {
    
}