#include "error.h"
#include <stdio.h>

void report(int line, const char* where, const char* message) {
    fprintf(stderr, "[line %d] Error %s: %s", line, where, message);
}