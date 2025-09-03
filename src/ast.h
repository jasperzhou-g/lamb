#ifndef LAMB_AST_H
#define LAMB_AST_H
#include "stringt.h"

enum ASTType {
    AST_ABS,
    AST_APP,
    AST_ARGLIST,
    AST_IDENTIFIER,
    AST_NUM,
    AST_SUCC, AST_DEC, // operators
    AST_LET_IN,
    AST_LETREC,
    AST_IF_ELSE,
    AST_POS,
    AST_NEG,
    AST_ERR,
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
        struct {struct AST* arg; } dec;
        struct {struct AST* arg; } neg;
        struct {struct AST* arg; } pos;
        struct {struct String error_message; } err;
        struct {struct String id; struct AST* value; struct AST* expr; } binding; //syntactic sugar for (fn id expr)(value)
        struct {struct String id; struct AST* fn; struct AST* expr; } letrec;        
        struct {struct AST* cond; struct AST* then_branch; struct AST* else_branch;} if_else;
    } u;
};
void pprint_ast(struct AST* ast);
void pprint_ast_helper(struct AST* ast);
struct AST* make_abs(struct AST* id, struct AST* body);
struct AST* make_app(struct AST* fn, struct AST* alist);
struct AST* cons_alist(struct AST* arg, struct AST* next); //bruh
struct AST* make_identifier(struct String name);
struct AST* make_num(int value);
struct AST* make_cond(struct AST* cond, struct AST* then_branch, struct AST* else_branch);
struct AST* make_succ(struct AST* arg);
struct AST* make_dec(struct AST* arg);
struct AST* make_pos(struct AST* arg);
struct AST* make_neg(struct AST* arg);
struct AST* make_err(struct String error_message);
struct AST* make_binding(struct String id, struct AST* value, struct AST* expr);
struct AST* make_letrec(struct String id, struct AST* fn, struct AST* expr);
void free_ast(struct AST* ast);

#endif