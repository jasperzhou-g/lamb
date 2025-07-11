#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "lexer.h"
#include "error.h"

static struct TokenList* tl_cons(struct Token t, struct TokenList* next) {
    struct TokenList* l = malloc(sizeof(struct TokenList));
    l->t = t;
    l->next = next;
    return l;
}

static void tl_add_token(struct TokenList* l, struct Token t) {
    if (!l)
        return;
    struct TokenList* curr = l;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = tl_cons(t, NULL);
}

static struct OptionalToken create_token(
    struct LexerState* s, const char* str_t, enum TokenType t) {
    struct Token tok = {
        .str_type = str_t, .type = t,
        .line = s->line, .str_start = s->start, .str_end = s->curr
    };
    return (struct OptionalToken) {tok, OPTIONAL_TOKEN_YES};
}

static struct OptionalToken create_none_token(
    struct LexerState* s) {
    struct Token tok = {
        .str_type = "NONE", .type = TOK_NONE,
        .line = s->line, .str_start = s->start, .str_end = s->curr
    };
    return (struct OptionalToken) {tok, OPTIONAL_TOKEN_NO};
}

static int lexer_match(struct LexerState* s, char expected) {
    if (s->curr >= s->len)
        return 0;
    if (s->source[s->curr] != expected)
        return 0;
    s->curr++;
    return 1;
}

static char lexer_advance(struct LexerState* s) {
    return s->source[(s->curr)++];
}

static char lexer_peek(struct LexerState* s) {
    if (s->curr >= s->len)
        return '\0';
    return s->source[s->curr];
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') 
        || (c >= 'A' && c <= 'Z')
        || (c == '_');
}

static int is_alphanumeric(char c) {
    return is_digit(c) || is_alpha(c);
}

static struct OptionalToken number(struct LexerState* s) {
    while (is_digit(lexer_peek(s))) lexer_advance(s);
    struct Token tok = {"NUMBER", TOK_NONE, s->start, s->curr, s->line};
    return (struct OptionalToken) {tok, OPTIONAL_TOKEN_YES};
}

static struct OptionalToken identifier(struct LexerState* s) {
    while (is_alphanumeric(lexer_peek(s))) lexer_advance(s);
    enum TokenType token_type;
    const char* tok_str = "keyword";
    if (!strncmp(&s->source[s->start], "if", 2)) {
        token_type = TOK_IF;
        tok_str = "IF";
    } else if (!strncmp(&s->source[s->start], "else", 4)) {
        tok_str = "ELSE";
        token_type = TOK_ELSE;
    } else if (!strncmp(&s->source[s->start], "let", 3)) {
        tok_str = "LET";
        token_type = TOK_LET;
    } else if (!strncmp(&s->source[s->start], "fix", 3)) {
        tok_str = "FIX";
        token_type = TOK_FIX;
    } else if (!strncmp(&s->source[s->start], "letrec", 6)) {
        tok_str = "LETREC";
        token_type = TOK_LETREC;
    } else if (!strncmp(&s->source[s->start], "in", 2)) {
        tok_str = "IN";
        token_type = TOK_IN;
    } else if (!strncmp(&s->source[s->start], "be", 2)) {
        tok_str = "BE";
        token_type = TOK_BE;
    } else if (!strncmp(&s->source[s->start], "fn", 2)) {
        tok_str = "FN";
        token_type = TOK_FN;
    } else {
        token_type = TOK_IDENTIFIER;
        tok_str = "identifier";
    }
    return create_token(s, tok_str, token_type);
}

static struct OptionalToken scan_token(struct LexerState* s) {
    char c = lexer_advance(s);
    struct OptionalToken ot = create_none_token(s);
    switch (c) {
        case '+': {
            return create_token(s, "+", TOK_PLUS);
        }
        case '(': {
            return create_token(s, "(", TOK_LEFT_PAREN);
        }
        case ')': {
            return create_token(s, ")", TOK_RIGHT_PAREN);
        }
        case '#': {
            while (lexer_peek(s) != '\n' && s->curr < s->len) lexer_advance(s);
            return create_none_token(s);
        }
        case '\n':
            s->line++;
        case ' ':
        case '\r':
        case '\t':
            return create_none_token(s);
        default: {
            if (is_digit(c))
                return number(s);
            else if (is_alpha(c))
                return identifier(s);
            report(s->line, "(unimplemented)", "Unexpected character.\n");
        }
    }
    return create_none_token(s);
}

struct LexerState cons_lexer(const char* source, int len) {
    return (struct LexerState) {source, len, 1, 0, 0};
}

struct TokenList* scan_source(struct LexerState* s) {
    struct OptionalToken sof = create_token(s, "SOF", TOK_SOF);
    struct TokenList* tokens = tl_cons(sof.t, NULL);
    int len_tokens = 0;
    while (s->curr < s->len) {
        s->start = s->curr;
        struct OptionalToken token = scan_token(s);
        if (token.e == OPTIONAL_TOKEN_YES)
            tl_add_token(tokens, token.t);
            len_tokens++;
    }
    struct OptionalToken eof = create_token(s, "EOF", TOK_EOF);
    tl_add_token(tokens, eof.t);
    return tokens;
}

void tl_delete(struct TokenList* tl) {
    struct TokenList* curr = tl;
    while (curr) {
        struct TokenList* old = curr;
        curr = curr->next;
        free(old);
    }
}