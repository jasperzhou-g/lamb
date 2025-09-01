#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "stringt.h"
#include "interpreter.h"

char *read_file_chars(FILE *f, long* len) {
    if (f == NULL) 
        return NULL;
    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    if (*len < 0) {
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    char *buffer = (char *) malloc(*len + 1);
    fread(buffer, *len, 1, f);
    buffer[*len] = '\0'; 
    return buffer;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "lamb: error: cannot find \"%s\"; No such file.\n",  argv[1]);
        exit(1);
    }
    long len = 0;
    char *source = read_file_chars(file, &len);
    if (!source) {
        fprintf(stderr, "lamb: err: cannot read \"%s\"; Error reading files.\n", argv[1]);
        fclose(file);
        exit(1);
    }
    fclose(file);
    file = NULL;

    struct Lexer* lexer_state = lexer_init(source, len);
    struct TokenList* tl = scan_source(lexer_state);
    assert(tl); // EOF is included
    lexer_free(lexer_state);
    lexer_state = NULL;
    
    struct Parser* parser_state = parser_init(tl, source);
    struct AST* ast = parse(parser_state);
    pprint_ast(ast);

    struct Interpreter lambterpreter = {
        .empty = NULL
    };
    interpret(&lambterpreter, ast);

    tl_free(tl);
    tl = NULL;
    free_ast(ast);
    ast = NULL;
    parser_free(parser_state);
    free(source);
    parser_state = NULL;
    source = NULL;
    return 0;
}   