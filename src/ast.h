#ifndef LAMB_AST_H
#define LAMB_AST_H

struct AST;
struct Num;
struct App;
struct Abs;
union Expr;

enum AST_Type {
    AST_ABS,
    AST_APP,
    AST_NUM,
};

struct Num {
    unsigned int n;
};

struct Succ {
    struct Num* num;
};

struct Identifier {
    char* str;
};

struct Abs {
    struct Identifier* id;
    struct AST* body;
};

struct App {
    struct Abs* abs;
    struct AST* arg;
};

union Expr {
    struct App* app;
    struct Abs* abs;
    struct Num* num;
};

struct AST {
    union Expr e;
    enum AST_Type t;
};

void pprint_ast(struct AST* ast);

#endif