#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringt.h"

struct String string_create(const char* cstr) {
    char* b = (char*)malloc(strlen(cstr) + 1);
    if (!b) {
        return (struct String) {.b = NULL, .length = 0};
    }
    strcpy(b, cstr);
    return (struct String) {.b = b, .length = strlen(cstr)};
}

struct String string_ncreate(const char* cstr, unsigned int len) {
    char* b = (char*)malloc(len + 1);
    if (!b) {
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

int string_compare(struct String a, struct String b) {
    if (a.length != b.length) return 0;
    for (int i = 0; i < a.length; i++) {
        if (a.b[i] != b.b[i]) return 0;
    }
    return 1;
}

struct String string_concat(struct String a, struct String b) {
    int new_length = a.length + b.length;
    char* new_b = (char*)malloc(new_length + 1);
    if (!new_b) {
        return (struct String) {.b = NULL, .length = 0};
    }
    strcpy(new_b, a.b);
    strcat(new_b, b.b);
    new_b[new_length] = '\0';
    string_free(&a);
    string_free(&b);
    return (struct String) {.b = new_b, .length = new_length};
}

struct String string_from_n(unsigned int n) {
    char buffer[20];
    if (n > 1000) {
        printf("hey, isn't that kinda too big?\n");
    }
    snprintf(buffer, sizeof(buffer), "%u", n);
    return string_create(buffer);
}