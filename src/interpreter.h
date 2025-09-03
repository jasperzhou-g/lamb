#ifndef LAMB_INTERPRETER_H
#define LAMB_INTERPRETER_H
#include "ast.h"
#include "stringt.h"

struct Rc {
    int count;
    void (*ref_free)(void*);
};

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
    struct Rc rc;
    void (*print)(struct LambObject* lo);
};

struct LambClosure {
    struct Environment* env;
    struct String param;
    struct AST* code;
};

void rc_init(struct Rc* rc, void (*ref_free)(void*));
void rc_use(struct Rc* rc);
void rc_release(struct Rc* rc, void** obj);

struct Environment {
    struct Rc rc;
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
struct LambObject* make_lamb_closure(struct AST* abs, struct String param, struct Environment* env);
void lamb_obj_free(void* lobj_ptr);

struct Environment* env_create(struct Environment* enclosing);
struct LambObject* env_get(struct Environment* env, struct String key);
void env_put(struct Environment* env, struct String key, struct LambObject* val);
void env_free(void* env);
void env_pprint(struct Environment *env);

void interpret(struct Interpreter* state, struct AST* program);

#endif