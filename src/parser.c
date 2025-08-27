#include <stdbool.h>
#include <stdio.h>
#include "lexer.h"
#include "parser.h"

static void debug_tokens(struct ParserState* ps) {
    printf("tokens: ");
    for (struct TokenList* curr = ps->tokens; curr; curr = curr->next) {
        printf("%s ", curr->t.str_type);
    }
    printf("\n");
}

static struct Token ps_peek(struct ParserState* ps) {
    if (!ps->tokens) {
        fprintf(stderr, "Critical error: no tokens where EOF");
        exit(1);
    }
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

static struct AST* parse_unary(struct ParserState* ps);
static struct AST* parse_app(struct ParserState* ps);
static struct AST* parse_abs(struct ParserState* ps);
static struct AST* parse_expr(struct ParserState* ps);

static struct AST* parse_unary(struct ParserState* ps) {
    printf("[parse_unary]\n");
    debug_tokens(ps);
    if (ps_match(ps, TOK_IDENTIFIER)) {
        struct Token id_tok = ps_prev(ps);
        struct String name = string_ncreate(
            &ps->src[id_tok.str_start], id_tok.str_end - id_tok.str_start);
        return make_identifier(name); 
    } else if (ps_match(ps, TOK_NUMBER)) {
        struct Token num_tok = ps_prev(ps);
        (void)num_tok;
        return make_num(0); //TODO Read number
    } else if (ps_match(ps, TOK_LEFT_PAREN)) {
        struct AST* expr = parse_expr(ps);
        if (expr->tag == AST_ERR) return expr;
        if (!ps_match(ps, TOK_RIGHT_PAREN)) {
            free_ast(expr);
            return make_err(string_create("Expected ')' after expression."));
        }
        return expr;
    } else if (ps_match(ps, TOK_PLUS)) {
        struct AST* inner = parse_unary(ps);
        if (inner->tag == AST_ERR) return inner;
        return make_succ(inner);
    }
    return make_err(string_create("Unexpected token."));
}

static struct AST* parse_expr(struct ParserState* ps) {
    printf("[parse_expr]\n");
    debug_tokens(ps);
    if (!ps) return NULL;
    if (!ps->tokens) return NULL; //
    if (ps->tokens->t.type == TOK_FN) {
        return parse_abs(ps);
    }
    return parse_app(ps);
}

static struct AST* parse_abs(struct ParserState* ps) {
    printf("[parse_abs]\n");
    debug_tokens(ps);
    if (!ps_match(ps, TOK_FN)) {
        struct String err_msg = string_create("Expected 'fn' keyword.");
        return make_err(err_msg);
    }
    if (!ps_match(ps, TOK_IDENTIFIER)) {
        struct String err_msg = string_create("Expected identifier after 'fn'.");
        return make_err(err_msg);
    }
    struct Token id_tok = ps_prev(ps);
    struct String id_tok_str = string_ncreate(ps->src + id_tok.str_start, id_tok.str_end - id_tok.str_start);
    struct AST* body = parse_expr(ps);
    if (body->tag == AST_ERR) {
        string_free(&id_tok_str);
        return body; 
    }
    return make_abs(make_identifier(id_tok_str), body);
}

static struct AST* parse_app(struct ParserState *ps) {
    printf("[parse_app]\n");
    debug_tokens(ps);
    struct AST* unary = parse_unary(ps);
    if (unary->tag == AST_ERR) 
        return unary;
    struct AST* alist = NULL;
    while (ps_match(ps, TOK_LEFT_PAREN)) {
        struct AST* arg = parse_expr(ps);
        if (arg->tag == AST_ERR) {
            free_ast(unary);
            free_ast(alist);
            return arg;
        }
        if (!ps_match(ps, TOK_RIGHT_PAREN)) {
            free_ast(unary);
            free_ast(alist);
            struct String err_msg = string_create("Expected ')' after application.");
            return make_err(err_msg);
        }
        alist = cons_alist(arg, alist);
    }
    struct AST* app = make_app(unary, alist);
    return app;
}

struct ParserState* init_parser(struct TokenList* tv, const char* src) {
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
    ps_advance(ps);
    return parse_expr(ps);
}