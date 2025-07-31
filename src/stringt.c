#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringt.h"

struct String string_create(const char* cstr) {
    char* b = (char*)malloc(strlen(cstr) + 1);
    if (!b) {
        free(b);
        return (struct String) {.b = NULL, .length = 0};
    }
    strcpy(b, cstr);
    return (struct String) {.b = b, .length = strlen(cstr)};
}

struct String string_ncreate(const char* cstr, unsigned int len) {
    char* b = (char*)malloc(len + 1);
    if (!b) {
        free(b);
        return (struct String) {.b = NULL, .length = 0};
    }
    strncpy(b, cstr, len);
    b[len] = '\0';
    return (struct String) {.b = b, .length = len};
}

void string_free(struct String* s) {
    if (s && s->b) {
        free(s->b);
    }
}