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
            // TODO: this leaks strings, please fix
            value_free(to_free);
            free(to_free);
        }
    }
    free(hm->buckets);
    free(hm);
}

/*

ENV FUNCTIONS

*/

void rc_init(struct Rc* rc, void (*ref_free)(void*)) {
    rc->count = 0;
    rc->ref_free = ref_free;
}

void rc_use(struct Rc* rc) {
    rc->count++;
}

void rc_release(struct Rc* rc, void** obj) {
    rc->count--;
    if (rc->count == 0) {
        rc->ref_free(*obj);
        *obj = NULL;
    }
}

static void pprint_lo(struct LambObject* obj) {
    switch(obj->type) {
        case LOBJ_NUM:
            printf("%d", *(int*)obj->obj);
            break;
        case LOBJ_ERR:
            printf("error: %s\n", ((struct String*)obj->obj)->b);
            break;
        case LOBJ_CLOSURE:
            printf("Closure");
            break;
    }
}

struct Environment* env_create(struct Environment* enclosing) {
    struct Environment* env = malloc(sizeof(struct Environment));
    env->enclosing = enclosing;
    env->values = hashmap_create();
    rc_init(&env->rc, env_free);
    if (env->enclosing) rc_use(&env->enclosing->rc);
    return env;
}

void env_put(struct Environment* env, struct String key, struct LambObject* val) {
    rc_use(&val->rc);
    hashmap_put(env->values, key, val);
}

static void release_lo(void *lo) {
    struct LambObject* lobj = lo;
    if (!lobj) return;
    rc_release(&lobj->rc, (void**) &lobj);
}

void pprint_env(struct Environment* env) {
    if (!env) return;
    if (!getenv("DEBUG")) return;
    pprint_env(env->enclosing);
    printf("\t");
    for (int i = 0; i < env->values->len_buckets; i++) {
        struct HashMapBucket* curr = env->values->buckets[i];
        while (curr) {
            printf("(%s -> ", curr->key.b);
            struct LambObject* lo = curr->item;
            if (lo) {
                lo->print(lo);
                printf(")");
            } else {
                printf("NULL\n");
            }
            curr = curr->next;
        }
    }
    printf("\n");
}

void env_free(void* env) {
    struct Environment* env_obj = env;
    if (!env_obj) return;
    if (env_obj->enclosing) rc_release(&env_obj->enclosing->rc, (void**) &env_obj->enclosing);
    hashmap_free(env_obj->values, release_lo);
    free(env_obj);
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
    obj->print = pprint_lo;
    rc_init(&obj->rc, lamb_obj_free);
    return obj;
}

struct LambObject* make_lamb_err(struct String err) {
    struct LambObject* obj = malloc(sizeof(struct LambObject));
    struct String* err_ptr = malloc(sizeof(struct String));
    *err_ptr = err;
    obj->type = LOBJ_ERR;
    obj->obj = err_ptr;
    obj->print = pprint_lo;
    rc_init(&obj->rc, lamb_obj_free);
    return obj;
}

struct LambObject* make_lamb_closure(struct AST* abs, struct String param, struct Environment* env) { // ast live after interpretation
    struct LambObject* obj = malloc(sizeof(struct LambObject));
    struct LambClosure* LC = malloc(sizeof(struct LambClosure));
    LC->env = env;
    LC->code = abs;
    LC->param = param;
    obj->type = LOBJ_CLOSURE;
    obj->obj = LC;
    obj->print = pprint_lo;
    rc_use(&env->rc);
    rc_init(&obj->rc, lamb_obj_free);
    return obj;
}

void lamb_obj_free(void* lobj_ptr) {
    struct LambObject* lobj = lobj_ptr;
    if (!lobj) return;
    struct LambClosure* cl;
    switch (lobj->type) {
        case LOBJ_NUM:
            free(lobj->obj);
            break;
        case LOBJ_ERR:
            string_free(lobj->obj);
            free(lobj->obj);
            break;
        case LOBJ_CLOSURE:
            cl = lobj->obj;
            rc_release(&cl->env->rc, (void**) &cl->env);
            free(lobj->obj);
            break;
    }
    free(lobj_ptr);
}

/*
LAMB OBJECTS END
*/

/*
EVALUATION FUNCTIONS START
*/


static struct LambObject* eval_abs(struct Interpreter* state, struct AST* abs, struct Environment* env);
struct LambObject* eval_expr(struct Interpreter* state, struct AST* expr, struct Environment* env);

static struct LambObject* closure_call(struct Interpreter* state, struct LambObject* closure, struct LambObject* arg) {
    if (closure->type != LOBJ_CLOSURE) {
        return make_lamb_err(string_create("[run-time error]: tried to apply something that's not a function"));
    }
    struct LambClosure* cl = closure->obj;
    rc_use(&arg->rc);
    struct Environment* new_env = env_create(cl->env);
    rc_use(&new_env->rc);
    env_put(new_env, string_clone(cl->param), arg);
    struct LambObject* result = eval_expr(state, cl->code->u.abs.body, new_env);
    rc_release(&new_env->rc, (void**) &new_env);
    rc_release(&arg->rc, (void**) &arg);
    return result;
}

static struct LambObject* eval_letrec(struct Interpreter* state, struct AST* expr, struct Environment* env)  {
    if (getenv("DEBUG")) {
        printf("[eval_letrec] "); 
        pprint_ast(expr);
    }
    rc_use(&env->rc);
    struct LambObject* fn = eval_expr(state, expr->u.letrec.fn, env);
    struct LambClosure* fn_cl = fn->obj;
    if (fn->type == LOBJ_ERR) {
        return fn;
    } 
    rc_use(&fn->rc);
    if (fn->type != LOBJ_CLOSURE) {
        rc_release(&fn->rc, (void**) &fn);
        return make_lamb_err(string_create("[type error] Expected a function to be recursively defined in letrec expression"));
    }
    env_put(fn_cl->env, string_clone(expr->u.letrec.id), fn);
    env_put(env, string_clone(expr->u.letrec.id), fn);
    struct LambObject* result = eval_expr(state, expr->u.letrec.expr, env);
    rc_release(&fn->rc, (void**) &fn);
    rc_release(&env->rc, (void**) &env);
    return result;
}

static struct LambObject* eval_app(struct Interpreter* state, struct AST* expr, struct Environment* env) {
    // TODO: Fix quite severe memory leak (reference counting problem).
    // Otherwise,LGTM
    if (getenv("DEBUG")) {
        printf("[eval_app] "); 
        pprint_ast(expr);
    } 
    rc_use(&env->rc);
    if (!expr->u.app.alist) {
        rc_release(&env->rc, (void**) &env);
        return eval_expr(state, expr->u.app.fn, env);
    } else if (!expr->u.app.alist->u.app_list.next) { //single argument
        struct Environment* new_env = env_create(env);
        rc_release(&env->rc, (void**) &env);
        rc_use(&new_env->rc);
        struct LambObject* arg = eval_expr(state, expr->u.app.alist->u.app_list.arg, new_env);
        if (arg->type == LOBJ_ERR) {
            rc_release(&new_env->rc, (void**) &new_env);
            return arg;
        }
        rc_use(&arg->rc);
        struct LambObject* cl_obj = eval_expr(state, expr->u.app.fn, new_env);
        if (cl_obj->type == LOBJ_ERR) {
            rc_release(&arg->rc, (void**) &arg);
            rc_release(&new_env->rc, (void**) &new_env);
            return cl_obj;
        }
        rc_use(&cl_obj->rc);
        struct LambObject* result = closure_call(state, cl_obj, arg);
        rc_release(&cl_obj->rc, (void**) &cl_obj);
        rc_release(&arg->rc, (void**) &arg);
        rc_release(&new_env->rc, (void**) &new_env);
        return result;
    }
    struct AST* alist = expr->u.app.alist;
    struct Environment* new_env = env_create(env);
    rc_release(&env->rc, (void**) &env);
    struct LambObject* cl_obj = eval_expr(state, expr->u.app.fn, env);
    if (cl_obj->type == LOBJ_ERR) {
        rc_use(&new_env->rc);
        rc_release(&new_env->rc, (void**) &new_env);
        return cl_obj;
    } 
    for (;;) {
        rc_use(&new_env->rc);
        if (cl_obj->type != LOBJ_CLOSURE) {
            rc_release(&new_env->rc, (void**) &new_env);
            return make_lamb_err(string_create("[type error] Expected a function to be applied"));
        }
        struct LambObject* arg_obj = eval_expr(state, alist->u.app_list.arg, new_env);
        if (arg_obj->type == LOBJ_ERR) {
            rc_release(&new_env->rc, (void**) &new_env);
            return arg_obj;
        }
        rc_use(&arg_obj->rc);
        rc_use(&cl_obj->rc);
        struct LambObject* result = closure_call(state, cl_obj, arg_obj);
        rc_release(&arg_obj->rc, (void**) &arg_obj);
        rc_release(&cl_obj->rc, (void**) &cl_obj);
        if (result->type == LOBJ_ERR) {
            rc_release(&new_env->rc, (void**) &new_env);
            return result;
        }
        cl_obj = result;
        alist = alist->u.app_list.next;
        struct Environment* old_env = new_env;
        new_env = env_create(old_env);
        rc_release(&old_env->rc, (void**) &old_env);
        if (!alist) break;
    }
    rc_release(&new_env->rc, (void**)&new_env);
    return cl_obj;
}

static struct LambObject* eval_abs(struct Interpreter* state, struct AST* abs, struct Environment* env) {
    if (getenv("DEBUG")) {
        printf("[eval_abs] "); 
        pprint_ast(abs);
    }
    if (abs->tag != AST_ABS) {
        return make_lamb_err(string_create("[run-time error] expected a function expression."));
    }
    rc_use(&env->rc);
    struct Environment* new_env = env_create(env);
    struct LambObject* closure = make_lamb_closure(abs, abs->u.abs.id->u.identifier.name, new_env);
    rc_release(&env->rc, (void**) &env);
    return closure;
}

static struct LambObject* eval_num(struct Interpreter* state, struct AST* num, struct Environment* env) {
    if (getenv("DEBUG")) {
        printf("[eval_num] "); 
        pprint_ast(num);
    }
    return make_lamb_num(num->u.num.value);
}

static struct LambObject* eval_succ(struct Interpreter* state, struct AST* succ, struct Environment* env) {
    if (getenv("DEBUG")) {
        printf("[eval_succ] "); 
        pprint_ast(succ);
    }
    struct LambObject* succ_num = eval_expr(state, succ->u.succ.arg, env);
    rc_use(&succ_num->rc);
    if (succ_num->type == LOBJ_NUM) {
        int n = *(int*)succ_num->obj;
        rc_release(&succ_num->rc, (void**) &succ_num);
        return make_lamb_num(n+1);
    }
    rc_release(&succ_num->rc, (void**) &succ_num);
    return make_lamb_err(string_create("[type error] + applied to a non-Num argument."));
}

static struct LambObject* eval_is_pos(struct Interpreter* state, struct AST* succ, struct Environment* env) {
    if (getenv("DEBUG")) {
        printf("[eval_is_pos] "); 
        pprint_ast(succ);
    }
    struct LambObject* succ_num = eval_expr(state, succ->u.succ.arg, env);
    rc_use(&succ_num->rc);
    if (succ_num->type == LOBJ_NUM) {
        int n = *(int*)succ_num->obj;
        rc_release(&succ_num->rc, (void**) &succ_num);
        return make_lamb_num(n>0);
    }
    rc_release(&succ_num->rc, (void**) &succ_num);
    return make_lamb_err(string_create("[type error] + applied to a non-Num argument."));
}

static struct LambObject* eval_is_neg(struct Interpreter* state, struct AST* succ, struct Environment* env) {
    if (getenv("DEBUG")) {
        printf("[eval_is_neg] "); 
        pprint_ast(succ);
    }
    struct LambObject* succ_num = eval_expr(state, succ->u.succ.arg, env);
    rc_use(&succ_num->rc);
    if (succ_num->type == LOBJ_NUM) {
        int n = *(int*)succ_num->obj;
        rc_release(&succ_num->rc, (void**) &succ_num);
        return make_lamb_num(n<0);
    }
    rc_release(&succ_num->rc, (void**) &succ_num);
    return make_lamb_err(string_create("[type error] + applied to a non-Num argument."));
}

static struct LambObject* eval_dec(struct Interpreter* state, struct AST* succ, struct Environment* env) {
    if (getenv("DEBUG")) {
        printf("[eval_dec] "); 
        pprint_ast(succ);
    }
    struct LambObject* dec_num = eval_expr(state, succ->u.succ.arg, env);
    rc_use(&dec_num->rc);
    if (dec_num->type == LOBJ_NUM) {
        int n = *(int*)dec_num->obj;
        rc_release(&dec_num->rc, (void**) &dec_num);
        return make_lamb_num(n-1);
    }
    rc_release(&dec_num->rc, (void**) &dec_num);
    return make_lamb_err(string_create("[type error] - applied to a non-Num argument."));
}

static struct LambObject* eval_let(struct Interpreter* state, struct AST* expr, struct Environment* env) {
    // let x = y in z === (fn x z)(y)
    if (getenv("DEBUG")) {
        printf("[eval_let] "); 
        pprint_ast(expr);
    }
    rc_use(&env->rc);
    struct LambObject* val = eval_expr(state, expr->u.binding.value, env);
    if (val->type == LOBJ_ERR) {
        rc_release(&env->rc, (void**) &env);
        return val;
    }
    rc_use(&val->rc);
    struct Environment* new_env = env_create(env);
    env_put(new_env, string_clone(expr->u.binding.id), val);
    rc_release(&val->rc, (void**) &val);
    rc_release(&env->rc, (void**) &env);
    return eval_expr(state, expr->u.binding.expr, new_env);
}

static struct LambObject* eval_if_else(struct Interpreter* state, struct AST* expr, struct Environment* env) {
    if (getenv("DEBUG")) {
        printf("[eval_if_else] "); 
        pprint_ast(expr);
    }    
    struct LambObject* cond = eval_expr(state, expr->u.if_else.cond, env);
    rc_use(&cond->rc);
    if (cond->type == LOBJ_ERR) { 
        return cond;
    } else if (cond->type != LOBJ_NUM) {
        return make_lamb_err(string_create("[type error] - tried to use a non-Num condition in if-else expression."));
    }
    if (*(int*)cond->obj) {
        rc_release(&cond->rc, (void**)&cond);
        return eval_expr(state, expr->u.if_else.then_branch, env);
    }
    rc_release(&cond->rc, (void**)&cond);
    return eval_expr(state, expr->u.if_else.else_branch, env);
}

struct LambObject* eval_expr(struct Interpreter* state, struct AST* expr, struct Environment* env) {
    pprint_env(env);
    switch (expr->tag) { //function table?
        struct LambObject* lo;
        case AST_APP:
            return eval_app(state, expr, env);
        case AST_NUM:
            return eval_num(state, expr, env);
        case AST_SUCC:
            return eval_succ(state, expr, env);
        case AST_DEC:
            return eval_dec(state, expr, env);
        case AST_NEG:
            return eval_is_neg(state, expr, env);
        case AST_POS:
            return eval_is_pos(state, expr, env);
        case AST_ABS:
            return eval_abs(state, expr, env);
        case AST_IDENTIFIER:
            lo = env_get(env, expr->u.identifier.name);
            if (lo) return lo;
            return make_lamb_err(string_concat(string_create("[run-time error] attempted to use an undefined name: "), string_clone(expr->u.identifier.name)));
        case AST_LET_IN:
            return eval_let(state, expr, env);
        case AST_IF_ELSE:
            return eval_if_else(state, expr, env);
        case AST_LETREC:
            return eval_letrec(state, expr, env);
        case AST_ERR:
            printf("%s\n", expr->u.err.error_message.b);
            return NULL;
        default:
            assert(0);
    }
}

/*
EVALUATION FUNCTIONS END
*/

void interpret(struct Interpreter* state, struct AST* program) {
    if (!getenv("DEBUG")) {
        printf("DEBUG env not set; skipping evaluation and stack frame logging.\n");
    } else {
        printf("DEBUG env set; skipping debug and stack frame logging.\n");
    }
    if (program->tag != AST_ERR) { printf("program repr:\n"); pprint_ast(program);}
    struct Environment *global = env_create(NULL);
    rc_use(&global->rc);
    struct LambObject* val = eval_expr(state, program, global);
    if (!val) {
        rc_release(&global->rc, (void**) &global);
        return;
    }
    rc_use(&val->rc);
    printf("> ");
    switch (val->type) {
        case LOBJ_NUM:
            printf("%d\n", *(int*)val->obj);
            break;
        case LOBJ_ERR:
            printf("%s\n", (*(struct String*)val->obj).b);
            break;
        case LOBJ_CLOSURE:
            printf("Closure (pretty printed): ");
            pprint_ast(((struct LambClosure*)val->obj)->code);
            break;
    }
    rc_release(&val->rc, (void**) &val);
    rc_release(&global->rc, (void**) &global);
}