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
            if (ast->u.app.alist) {
                pprint_ast_helper(ast->u.app.fn);
                printf("[");
                pprint_ast_helper(ast->u.app.alist);
                printf("]");
            } else {
                pprint_ast_helper(ast->u.app.fn);
            }
             break;
        case AST_NUM:
            printf("%d", ast->u.num.value);
            break;
        case AST_SUCC:
            printf("(+");
            pprint_ast_helper(ast->u.succ.arg);
            printf(")");
            break;
        case AST_NEG:
            printf("(>");
            pprint_ast_helper(ast->u.pos.arg);
            printf(")");
            break;
        case AST_POS:
            printf("(<");
            pprint_ast_helper(ast->u.pos.arg);
            printf(")");
            break;
        case AST_DEC:
            printf("(-");
            pprint_ast_helper(ast->u.succ.arg);
            printf(")");
            break;
        case AST_IDENTIFIER:
            printf("%s", ast->u.identifier.name.b);
            break;
        case AST_ERR:
            printf("%s", ast->u.err.error_message.b);
            break;
        case AST_ARGLIST:
            pprint_ast_helper(ast->u.app_list.arg);
            if (ast->u.app_list.next) {
                printf("[");
                pprint_ast_helper(ast->u.app_list.next);
                printf("]");
            } else {
                printf(" NIL");
            }
            break;
        case AST_LET_IN:
            printf("%s", ast->u.binding.id.b);
            printf("=");
            pprint_ast_helper(ast->u.binding.value);
            printf(" in (");
            pprint_ast_helper(ast->u.binding.expr);
            printf(")");
            break;
        case AST_IF_ELSE:
            printf("if ");
            pprint_ast_helper(ast->u.if_else.cond);
            printf(" then ");
            pprint_ast_helper(ast->u.if_else.then_branch);
            printf(" else ");
            pprint_ast_helper(ast->u.if_else.else_branch);
            break;
        case AST_LETREC:
            printf("def %s=", ast->u.letrec.id.b);
            pprint_ast_helper(ast->u.letrec.fn);
            printf(" in (");
            pprint_ast_helper(ast->u.letrec.expr);
            printf(")");
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

struct AST* make_app(struct AST* fn, struct AST* alist) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_APP;
    ast->u.app.fn = fn;
    ast->u.app.alist = alist;
    return ast;
}

struct AST* make_identifier(struct String name) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_IDENTIFIER;
    ast->u.identifier.name = name;
    return ast;
}

struct AST* make_cond(struct AST* cond, struct AST* then_branch, struct AST* else_branch) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_IF_ELSE;
    ast->u.if_else.cond = cond;
    ast->u.if_else.then_branch = then_branch;
    ast->u.if_else.else_branch = else_branch;
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

struct AST* make_pos(struct AST* arg) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_POS;
    ast->u.pos.arg = arg;
    return ast;
}

struct AST* make_neg(struct AST* arg) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_NEG;
    ast->u.neg.arg = arg;
    return ast;
}

struct AST* make_dec(struct AST* arg) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_DEC;
    ast->u.dec.arg = arg;
    return ast;
}

struct AST* make_err(struct String error_message) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_ERR;
    ast->u.err.error_message = error_message;
    return ast;
}

struct AST* make_binding(struct String id, struct AST* value, struct AST* expr) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_LET_IN;
    ast->u.binding.id = id;
    ast->u.binding.value = value;
    ast->u.binding.expr = expr;
    return ast;
}

struct AST* make_letrec(struct String id, struct AST* fn, struct AST* expr) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_LETREC;
    ast->u.letrec.id = id;
    ast->u.letrec.fn = fn;
    ast->u.letrec.expr = expr;
    return ast;
}

struct AST* cons_alist(struct AST* arg, struct AST* next) {
    struct AST* ast = malloc(sizeof(struct AST));
    ast->tag = AST_ARGLIST;
    ast->u.app_list.arg = arg;
    ast->u.app_list.next = next;
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
            free_ast(ast->u.app.alist);
            break;
        case AST_NUM:
            break;
        case AST_SUCC:
            free_ast(ast->u.succ.arg);
            break;
        case AST_POS:
            free_ast(ast->u.pos.arg);
            break;
        case AST_NEG:
            free_ast(ast->u.neg.arg);
            break;
        case AST_IDENTIFIER:
            string_free(&ast->u.identifier.name);
            break;
        case AST_ERR:
            string_free(&ast->u.err.error_message);
            break;
        case AST_ARGLIST:
            free_ast(ast->u.app_list.arg);
            free_ast(ast->u.app_list.next);
            break;
        case AST_DEC:
            free_ast(ast->u.dec.arg);
            break;
        case AST_LET_IN:
            string_free(&ast->u.binding.id);
            free_ast(ast->u.binding.value);
            free_ast(ast->u.binding.expr);
            break;
        case AST_IF_ELSE:
            free_ast(ast->u.if_else.cond);
            free_ast(ast->u.if_else.then_branch);
            free_ast(ast->u.if_else.else_branch);
            break;
        case AST_LETREC:
            string_free(&ast->u.letrec.id);
            free_ast(ast->u.letrec.fn);
            free_ast(ast->u.letrec.expr);
            break;
        default:
            fprintf(stderr, "lamb: err: [free_ast] Unknown AST type.\n");
    }
    free(ast);
}