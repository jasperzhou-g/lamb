#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringt.h"

struct String* string_create(const char* cstr) {
    struct String* s = (struct String*)malloc(sizeof(struct String));
    if (!s)
        return NULL;
    s->length = strlen(cstr);
    s->b = (char*)malloc(s->length + 1);
    if (!s->b) {
        free(s);
        return NULL;
    }
    strcpy(s->b, cstr);
    return s;
}

void string_free(struct String* s) {
    if (s) {
        free(s->b);
        free(s);
    }
}