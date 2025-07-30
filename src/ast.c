#include <stdio.h>
#include "ast.h"

static void pprint_num(struct Num* n);
static void pprint_abs(struct Abs* abs);
static void pprint_app(struct App* app);
static void pprint_ast_helper(struct AST* ast);

static void pprint_num(struct Num* n) {
    printf("%u", n->n);
}

static void pprint_abs(struct Abs* abs) {
    printf("(\\%s ", abs->id->str->b);
    pprint_ast(abs->body);
    printf(")");
}

static void pprint_app(struct App* app) {
    printf("(");
    pprint_abs(app->abs);
    printf(" ");
    pprint_ast(app->arg);
    printf(")");
}

static void pprint_ast_helper(struct AST* ast) {
    switch (ast->t) {
        case AST_ABS: {
            pprint_abs(ast->e.abs);
            break;
        }
        case AST_APP: {
            pprint_app(ast->e.app);
            break;
        }
        case AST_NUM: {
            pprint_num(ast->e.num);
            break;
        }
    }
}

void pprint_ast(struct AST* ast) {
    pprint_ast_helper(ast);
    printf("\n");
}