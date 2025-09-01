#ifndef LAMB_INTERPRETER_H
#define LAMB_INTERPRETER_H
#include "ast.h"
#include "stringt.h"

struct HashMapBucket {
    void *item;
    struct String key;
    void *next;
};

struct HashMap {
    struct HashMapBucket** buckets;
    int len_buckets;
    int n_items;
};

enum LambObjectType {
    LOBJ_ERR,
    LOBJ_NUM,
    LOBJ_CLOSURE
};

struct LambObject {
    enum LambObjectType type;
    void *obj;
};

struct Environment {
    struct Environment* enclosing;
    struct HashMap* values;
};

struct Interpreter {
    void *empty;
};

void hashmap_put(struct HashMap* hm, struct String key, void* item);
int hashmap_contains(struct HashMap* hm, struct String key);
void* hashmap_get(struct HashMap* hm, struct String key);
struct HashMap* hashmap_create();
void hashmap_free(struct HashMap* hm, void (*value_free)(void*));

struct LambObject* make_lamb_num(int num);
struct LambObject* make_lamb_err(struct String err);
void lamb_obj_free(struct LambObject* lobj);
//struct LambObject* make_lamb_closure(struct AST* abs, struct HashMap* env);

struct Environment* env_create(struct Environment* enclosing);
void env_free(struct Environment* env);

struct LambObject* eval_expr(struct Interpreter* state, struct AST* expr);
void interpret(struct Interpreter* state, struct AST* program);

#endif