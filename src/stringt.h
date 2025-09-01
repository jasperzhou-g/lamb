#ifndef LAMB_STRING_H
#define LAMB_STRING_H
#include <stdlib.h>
#include <string.h>

struct String {
    int length;
    char* b;
};

struct String string_create(const char* cstr);
struct String string_ncreate(const char* cstr, unsigned int len);
struct String string_concat(struct String a, struct String b);
struct String string_from_n(unsigned int n);
int string_compare(struct String a, struct String b);
void string_free(struct String* s); 

#endif