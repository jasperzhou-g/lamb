#include <stdbool.h>
#include "lexer.h"
#include "parser.h"

static struct Token ps_peek(struct ParserState* ps) {
    return ps->tokens->t;
} 

static bool ps_is_done(struct ParserState* ps) {
    return ps_peek(ps).type == TOK_EOF;
}

static struct Token ps_advance(struct ParserState* ps) {
    struct Token tok = ps->tokens->t;
    ps->prev = ps->tokens;
    ps->tokens = ps->tokens->next;
    return tok;
}

static bool ps_check(struct ParserState* ps, enum TokenType t) {
    if (ps_is_done(ps)) return false;
    return ps_peek(ps).type == t;
}

static bool ps_match(struct ParserState* ps, enum TokenType t) {
    if (ps_check(ps, t)) {
        ps_advance(ps);
        return true;
    }
    return false;
}

static struct Token ps_prev(struct ParserState* ps) {
    return ps->prev->t;
}

static struct AST* parse_unary(struct ParserState* ps) {
    
}

static struct AST* parse_identifier(struct ParserState* ps) {

}

static struct AST* parse_abs(struct ParserState* ps) {

}

static struct AST* parse_app(struct ParserState *ps) {

}

struct ParserState* cons_parser(struct TokenList* tv, const char* src) {
    struct ParserState* ps = malloc(sizeof(struct ParserState));
    ps->src = src;
    ps->tokens = tv;
    ps->prev = NULL;
    return ps;
}

void parser_free(struct ParserState* ps) {
    free(ps);
}

struct AST* parse(struct ParserState* ps) {
    
}