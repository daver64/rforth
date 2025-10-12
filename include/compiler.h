#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include "dict.h"

/* Forward declarations */
typedef struct rforth_ctx rforth_ctx_t;

/* Compiler context */
typedef struct {
    FILE *output;               /* Output C file */
    dict_t *dict;              /* Dictionary for word lookup */
    int word_count;            /* Generated word counter */
    bool in_main;              /* Whether we're in main function */
} compiler_ctx_t;

/* Code generation functions */
compiler_ctx_t* compiler_create(const char *output_file);
void compiler_destroy(compiler_ctx_t *compiler);
bool compiler_compile_file(compiler_ctx_t *compiler, const char *input_file);
bool compiler_generate_header(compiler_ctx_t *compiler);
bool compiler_generate_footer(compiler_ctx_t *compiler);
bool compiler_generate_word(compiler_ctx_t *compiler, const char *name, const char *definition);
bool compiler_generate_main(compiler_ctx_t *compiler, const char *main_code);

/* Compilation utilities */
bool compile_forth_to_c(const char *input_file, const char *output_file);
bool invoke_c_compiler(const char *c_file, const char *output_file);

#endif /* COMPILER_H */