#ifndef LAMB_LEXER_H
#define LAMB_LEXER_H
#include <stdlib.h>
#include <stdio.h>

struct LexerState {
    const char* source;
    long len;
    int line;
    int start;
    int curr;
};

enum TokenType {
    // Operators
    TOK_PLUS, TOK_LEFT_PAREN, TOK_RIGHT_PAREN,

    TOK_IDENTIFIER,

    // Keywords
    TOK_FIX, TOK_LETREC, TOK_LET, TOK_IN, TOK_BE, TOK_FN, TOK_IF, TOK_ELSE,

    // Literals
    TOK_NUMBER,
    // etc.
    TOK_SOF, TOK_EOF, TOK_NONE
};

struct Token {
    const char *str_type;
    enum TokenType type;
    int str_start;
    int str_end;
    int line;
};

enum OptTokenTag { // Overengineered to perfection!
    OPTIONAL_TOKEN_YES,
    OPTIONAL_TOKEN_NO
};

struct OptionalToken {
    struct Token t;
    enum OptTokenTag e;
};

// Singly-linked list FTW! Totally no regrets!
struct TokenList {
    struct Token t;
    struct TokenList *next;
};

// why use lexer generators when you can reinvent the wheel
struct LexerState* cons_lexer(const char* source, int len);
void lexer_free(struct LexerState* ls);
struct TokenList* scan_source(struct LexerState* s);
void tl_free(struct TokenList* tl);

#endif