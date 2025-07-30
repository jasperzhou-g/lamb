#ifndef LAMB_STRING_H
#define LAMB_STRING_H
#include <stdlib.h>
#include <string.h>

struct String {
    char* b;
    int length;
};

struct String* string_create(const char* cstr);
void string_free(struct String* s); 

#endif