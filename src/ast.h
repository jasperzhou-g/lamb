#ifndef LAMB_AST_H
#define LAMB_AST_H
#include "stringt.h"

enum ASTType {
    AST_ABS,
    AST_APP,
    AST_ARGLIST,
    AST_IDENTIFIER,
    AST_NUM,
    AST_SUCC,
    AST_ERR
};

struct AST;

struct AST {
    enum ASTType tag;
    union {
        struct {struct AST* fn; struct AST* alist; } app;
        struct {struct AST* arg; struct AST* next; } app_list;
        struct {struct AST* id; struct AST* body; } abs;
        struct {struct String name; } identifier;
        struct {int value; } num;
        struct {struct AST* arg; } succ;
        struct {struct String error_message; } err;
    } u;
};
void pprint_ast(struct AST* ast);
struct AST* make_abs(struct AST* id, struct AST* body);
struct AST* make_app(struct AST* fn, struct AST* alist);
struct AST* cons_alist(struct AST* arg, struct AST* next);
struct AST* make_identifier(struct String name);
struct AST* make_num(int value);
struct AST* make_succ(struct AST* arg);
struct AST* make_err(struct String error_message);
void free_ast(struct AST* ast);

#endif