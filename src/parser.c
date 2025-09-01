#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "lexer.h"
#include "parser.h"

static void debug_tokens(struct Parser* ps) {
    printf("tokens: ");
    for (struct TokenList* curr = ps->tokens; curr; curr = curr->next) {
        printf("%s ", curr->t.str_type);
    }
    printf("\n");
}

static struct String err_line_pref(unsigned int line, struct String msg) {
    return string_concat(
        string_concat(
            string_create("error [line "),
            string_concat(
                string_from_n(line),
                string_create("]: ")
            )
        ),
        msg
    );
}

static struct Token ps_peek(struct Parser* ps) {
    if (!ps->tokens) {
        fprintf(stderr, "Critical error: no tokens where EOF");
        exit(1);
    }
    return ps->tokens->t;
} 

static bool ps_is_done(struct Parser* ps) {
    return ps_peek(ps).type == TOK_EOF;
}

static struct Token ps_advance(struct Parser* ps) {
    struct Token tok = ps->tokens->t;
    ps->prev = ps->tokens;
    ps->tokens = ps->tokens->next;
    return tok;
}

static bool ps_check(struct Parser* ps, enum TokenType t) {
    if (ps_is_done(ps)) return false;
    return ps_peek(ps).type == t;
}

static bool ps_match(struct Parser* ps, enum TokenType t) {
    if (ps_check(ps, t)) {
        ps_advance(ps);
        return true;
    }
    return false;
}

static struct Token ps_prev(struct Parser* ps) {
    return ps->prev->t;
}

// Lambda calculus application, abstraction + successor 
static struct AST* parse_unary(struct Parser* ps);
static struct AST* parse_app(struct Parser* ps);
static struct AST* parse_abs(struct Parser* ps);
static struct AST* parse_expr(struct Parser* ps);

static struct AST* parse_unary(struct Parser* ps) {
    if (ps_match(ps, TOK_IDENTIFIER)) {
        struct Token id_tok = ps_prev(ps);
        struct String name = string_ncreate(
            &ps->src[id_tok.str_start], id_tok.str_end - id_tok.str_start);
        return make_identifier(name); 
    } else if (ps_match(ps, TOK_NUMBER)) {
        struct Token num_tok = ps_prev(ps);
        (void)num_tok;
        struct String num_str = string_ncreate(
            &ps->src[ps_prev(ps).str_start], ps_prev(ps).str_end - ps_prev(ps).str_start);
        char* endptr;
        long val = strtol(num_str.b, &endptr, 10);
        if (*endptr != '\0' || val > INT_MAX) {
            string_free(&num_str);
            return make_err(
                err_line_pref(
                    ps_prev(ps).line,
                    string_create("Invalid number literal.")
                )
            );
        }
        return make_num(val); //TODO Read number
    } else if (ps_match(ps, TOK_LEFT_PAREN)) {
        struct AST* expr = parse_expr(ps);
        if (expr->tag == AST_ERR) return expr;
        if (!ps_match(ps, TOK_RIGHT_PAREN)) {
            free_ast(expr);

            return make_err(
                err_line_pref(
                    ps->tokens->t.line, 
                    string_create("Expected ')' after expression.")
                )
            );
        }
        return expr;
    } else if (ps_match(ps, TOK_PLUS)) {
        struct AST* inner = parse_unary(ps);
        if (inner->tag == AST_ERR) return inner;
        return make_succ(inner);
    } else if (ps_match(ps, TOK_MINUS)) {
        struct AST* inner = parse_unary(ps);
        if (inner->tag == AST_ERR) return inner;
        return make_dec(inner);
    }
    return make_err(
        err_line_pref(
            ps->tokens->t.line,
            string_concat(
                string_create("Unexpected token "), 
                string_create(ps->tokens->t.str_type)
            )
        )
    );
}
static struct AST* parse_expr(struct Parser* ps) {
    if (!ps) return NULL;
    if (!ps->tokens) return NULL; //
    static struct AST* expr = NULL;
    if (ps->tokens->t.type == TOK_FN) {
        expr = parse_abs(ps);
    } else {
        expr = parse_app(ps);
    }
    return expr;
}

static struct AST* parse_abs(struct Parser* ps) {
    if (!ps_match(ps, TOK_FN)) {
        return make_err(
            err_line_pref(
                ps->tokens->t.line, 
                string_create("Expected 'fn' keyword.")
            )
        );
    }
    if (!ps_match(ps, TOK_IDENTIFIER)) {
        return make_err(
            err_line_pref(
                ps->prev->t.line, 
                string_create("Expected identifier after 'fn'.")
            )
        );

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

static struct AST* parse_app(struct Parser *ps) {
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
            return make_err(
                err_line_pref(
                    ps->prev->t.line, 
                    string_create("Expected ')' after application")
                )
            );
        }
        alist = cons_alist(arg, alist);
    }
    return make_app(unary, alist);
}

struct Parser* parser_init(struct TokenList* tv, const char* src) {
    struct Parser* ps = malloc(sizeof(struct Parser));
    ps->src = src;
    ps->tokens = tv;
    ps->prev = NULL;
    return ps;
}

void parser_free(struct Parser* ps) {
    free(ps);
}

struct AST* parse(struct Parser* ps) {
    ps_advance(ps);
    struct AST* program = parse_expr(ps);
    if (!ps_is_done(ps)) {
        free_ast(program);
        return make_err(
            err_line_pref(
                ps->tokens->t.line, 
                string_create("Expected EOF, but received tokens after program end.")
            )
        );
    }
    return program;
}