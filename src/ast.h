#ifndef LAMB_AST_H
#define LAMB_AST_H

enum AST_Type {
    AST_ABS,
    AST_APP,
    AST_NUM,
};

struct Num {
    
};

struct App {

};

struct Abs {
    
};

union Expr {
    struct App app;
    struct Abs abs;
    struct Num num;
};

struct AST {
    union Expr e;
    enum AST_Type t;
};

#endif