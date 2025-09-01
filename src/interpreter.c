#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "interpreter.h"

const int INITIAL_BUCKET_COUNT = 16;

static unsigned int hash(struct HashMap* hm, struct String key) {
    unsigned int val = 59810393 + key.length;
    for (unsigned int i = 0; i < key.length; i++) {
        val = val * 7;
        val ^= key.b[i];
    }
    return val % hm->len_buckets;
}

struct HashMap* hashmap_create() {
    struct HashMap* hm = malloc(sizeof(struct HashMap));
    hm->buckets = malloc(sizeof(struct HashMapBucket*) * INITIAL_BUCKET_COUNT);
    hm->len_buckets = INITIAL_BUCKET_COUNT;
    hm->n_items = 0;
    for (int i = 0; i < INITIAL_BUCKET_COUNT; i++){
        hm->buckets[i] = NULL;
    }
    return hm;
}

void hashmap_put(struct HashMap* hm, struct String key, void* item) {
    unsigned int index = hash(hm, key);
    if (hm->buckets[index] == NULL) {
        struct HashMapBucket* bucket = malloc(sizeof(struct HashMapBucket));
        bucket->item = item;
        bucket->key = key;
        bucket->next = NULL;
        hm->buckets[index] = bucket;
        hm->n_items++;
        return;
    }
    struct HashMapBucket* curr = hm->buckets[index];
    while (curr) {
        if (string_compare(curr->key, key)) {
            curr->item = item;
            return;
        }
        curr = curr->next;
    }
    struct HashMapBucket* bucket = malloc(sizeof(struct HashMapBucket));
    bucket->item = item;
    bucket->key = key;
    bucket->next = hm->buckets[index];
    hm->buckets[index] = bucket;
    hm->n_items++;
}

int hashmap_contains(struct HashMap* hm, struct String key) {
    unsigned int index = hash(hm, key);
    struct HashMapBucket* curr = hm->buckets[index];
    while (curr) {
        if (string_compare(curr->key, key)) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

void* hashmap_get(struct HashMap* hm, struct String key) {
    unsigned int index = hash(hm, key);
    struct HashMapBucket* curr = hm->buckets[index];
    while (curr) {
        if (string_compare(curr->key, key)) {
            return curr->item;
        }
        curr = curr->next;
    }
    return NULL;
}

void hashmap_free(struct HashMap* hm, void (*value_free)(void*)) {
    if (!hm) return;
    for (int i = 0; i < hm->len_buckets; i++) {
        struct HashMapBucket* curr = hm->buckets[i];
        while (curr) {
            struct HashMapBucket* to_free = curr;
            curr = curr->next;
            string_free(&to_free->key);
            value_free(to_free->item);
            free(to_free);
        }
    }
    free(hm->buckets);
    free(hm);
}

/*

ENV FUNCTIONS

*/

struct Environment* env_create(struct Environment* enclosing) {
    struct Environment* env = malloc(sizeof(struct Environment));
    env->enclosing = enclosing;
    env->values = hashmap_create();
    return env;
}

void env_free(struct Environment* env) {
    if (!env) return;
    env_free(env->enclosing);
    hashmap_free(env->values, free);
    free(env);
}

struct LambObject* env_get(struct Environment* env, struct String key) {
    struct Environment* curr = env;
    while (curr) {
        if (hashmap_contains(curr->values, key)) {
            return hashmap_get(curr->values, key);
        }
        curr = curr->enclosing;
    }
    return NULL;
}

/*

ENV FUNCTIONS

*/

/*
LAMB OBJECTS START
*/

struct LambObject* make_lamb_num(int num) {
    struct LambObject* obj = malloc(sizeof(struct LambObject));
    int* num_ptr = malloc(sizeof(int));
    *num_ptr = num;
    obj->type = LOBJ_NUM;
    obj->obj = num_ptr;
    return obj;
}

struct LambObject* make_lamb_err(struct String err) {
    struct LambObject* obj = malloc(sizeof(struct LambObject));
    struct String* err_ptr = malloc(sizeof(struct String));
    *err_ptr = err;
    obj->type = LOBJ_ERR;
    obj->obj = err_ptr;
    return obj;
}

void lamb_obj_free(struct LambObject* lobj) {
    if (!lobj) return;
    switch (lobj->type) {
        case LOBJ_NUM:
            free(lobj->obj);
            break;
        case LOBJ_ERR:
            string_free(lobj->obj);
            break;
        case LOBJ_CLOSURE:
            // Free closure-specific resources
            break;
    }
    free(lobj);
}

/*
LAMB OBJECTS END
*/

/*
EVALUATION FUNCTIONS START
*/

static struct LambObject* eval_app(struct Interpreter* state, struct AST* expr) {
    if (!expr->u.app.alist) {
        return eval_expr(state, expr->u.app.fn);
    } else {
        return make_lamb_err(string_create("Application not supported yet."));
    }
    return NULL;
}


static struct LambObject* eval_num(struct Interpreter* state, struct AST* num) {
    struct LambObject* lamb_num = malloc(sizeof(struct LambObject));
    lamb_num->type = LOBJ_NUM;
    lamb_num->obj = malloc(sizeof(int));
    *(int*)lamb_num->obj = num->u.num.value;
    return lamb_num;
}

static struct LambObject* eval_succ(struct Interpreter* state, struct AST* succ) {
    struct LambObject* succ_num = eval_expr(state, succ->u.succ.arg);
    if (succ_num->type == LOBJ_NUM) {
        struct LambObject* result = malloc(sizeof(struct LambObject));
        result->type = LOBJ_NUM;
        result->obj = malloc(sizeof(int));
        *(int*)result->obj = *(int*)succ_num->obj + 1;
        lamb_obj_free(succ_num);
        return result;
    }
    lamb_obj_free(succ_num);
    return make_lamb_err(string_create("[type error] + applied to a non-Num argument."));
}

/*
EVALUATION FUNCTIONS END
*/

struct LambObject* eval_expr(struct Interpreter* state, struct AST* expr) {
    switch (expr->tag) {
        case AST_APP:
            return eval_app(state, expr);
        case AST_NUM:
            return eval_num(state, expr);
        case AST_SUCC:
            return eval_succ(state, expr);
        default:
            assert(0);
    }
}

void interpret(struct Interpreter* state, struct AST* program) {
    pprint_ast(program);
    struct LambObject* val = eval_expr(state, program);
    switch (val->type) {
        case LOBJ_NUM:
            printf("%d\n", *(int*)val->obj);
            break;
        case LOBJ_ERR:
            printf("You got an error bruh\n");
            break;
        case LOBJ_CLOSURE:
            printf("You got a closure bruh\n");
            break;
    }
    lamb_obj_free(val);
}