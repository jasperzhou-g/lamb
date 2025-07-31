#include <stdio.h>
#include "ast.h"

static void pprint_ast_helper(struct AST* ast) {
    if (!ast) return;
    switch (ast->tag) {
        case AST_ABS:
            printf("(\\");
            pprint_ast_helper(ast->u.abs.id);
            printf(" ");
            pprint_ast_helper(ast->u.abs.body);
            printf(")");
            break;
        case AST_APP:
            printf("(");
            pprint_ast_helper(ast->u.app.fn);
            printf(" ");
            pprint_ast_helper(ast->u.app.arg);
            printf(")");
            break;
        case AST_NUM:
            printf("%d", ast->u.num.value);
            break;
        case AST_SUCC:
            printf("(+");
            pprint_ast_helper(ast->u.succ.arg);
            printf(")");
            break;
        case AST_IDENTIFIER:
            printf("%s", ast->u.identifier.name.b);
            break;
        default:
            fprintf(stderr, "lamb: err: [pprint_ast_helper] Unknown AST type.\n");
    }
}

void pprint_ast(struct AST* ast) {
    pprint_ast_helper(ast);
    printf("\n");
}

struct AST* make_abs(struct AST* id, struct AST* body) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_ABS;
    ast->u.abs.id = id;
    ast->u.abs.body = body;
    return ast;
}

struct AST* make_app(struct AST* fn, struct AST* arg) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_APP;
    ast->u.app.fn = fn;
    ast->u.app.arg = arg;
    return ast;
}

struct AST* make_identifier(struct String name) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_IDENTIFIER;
    ast->u.identifier.name = name;
    return ast;
}

struct AST* make_num(int value) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_NUM;
    ast->u.num.value = value;
    return ast;
}

struct AST* make_succ(struct AST* arg) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_SUCC;
    ast->u.succ.arg = arg;
    return ast;
}

void free_ast(struct AST* ast) {
    if (!ast) return;
    switch (ast->tag) {
        case AST_ABS:
            free_ast(ast->u.abs.id);
            free_ast(ast->u.abs.body);
            break;
        case AST_APP:
            free_ast(ast->u.app.fn);
            free_ast(ast->u.app.arg);
            break;
        case AST_NUM:
            break;
        case AST_SUCC:
            free_ast(ast->u.succ.arg);
            break;
        case AST_IDENTIFIER:
            string_free(&ast->u.identifier.name);
            break;
        default:
            fprintf(stderr, "lamb: err: [free_ast] Unknown AST type.\n");
    }
    free(ast);
}