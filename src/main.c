#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"

char *read_file_chars(FILE *f, long* len) {
    if (f == NULL) 
        return NULL;
    fseek(f, 0, SEEK_END);
    *len = ftell(f);
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
        fprintf(stderr, "err: unable to read file \"%s\".\n",  argv[1]);
        exit(1);
    }
    long len = 0;
    char *source = read_file_chars(file, &len);
    fclose(file);
    file = NULL;

    struct LexerState* lexer_state = cons_lexer(source, len);
    struct TokenList* tl = scan_source(lexer_state);
    assert(tl);
    lexer_free(lexer_state);
    lexer_state = NULL;
    
    struct ParserState* parser_state = cons_parser(tl, source);
    struct AST* ast = parse(parser_state);

    for (struct TokenList* curr = tl; curr; curr = curr->next) {
        printf("%s ", curr->t.str_type);
    }
    printf("\n");

    tl_free(tl);
    tl = NULL;

    parser_free(parser_state);
    free(source);
    parser_state = NULL;
    source = NULL;
    return 0;
}   