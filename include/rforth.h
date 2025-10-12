#ifndef RFORTH_H
#define RFORTH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Version information */
#define RFORTH_VERSION_MAJOR 1
#define RFORTH_VERSION_MINOR 0
#define RFORTH_VERSION_PATCH 0

/* Configuration constants */
#define MAX_WORD_LENGTH 64
#define MAX_INPUT_LENGTH 1024
#define DEFAULT_STACK_SIZE 256
#define DEFAULT_RETURN_STACK_SIZE 256

/* Include other headers */
#include "stack.h"
#include "dict.h"
#include "parser.h"
#include "compiler.h"

/* Error codes */
typedef enum {
    RFORTH_OK = 0,
    RFORTH_ERROR_STACK_UNDERFLOW,
    RFORTH_ERROR_STACK_OVERFLOW,
    RFORTH_ERROR_WORD_NOT_FOUND,
    RFORTH_ERROR_PARSE_ERROR,
    RFORTH_ERROR_FILE_ERROR,
    RFORTH_ERROR_COMPILE_ERROR,
    RFORTH_ERROR_MEMORY
} rforth_error_t;

/* Forward declarations */
typedef struct rforth_ctx rforth_ctx_t;

/* Main context structure */
struct rforth_ctx {
    stack_t *data_stack;        /* Data stack */
    stack_t *return_stack;      /* Return stack */
    dict_t *dict;              /* Word dictionary */
    parser_t *parser;          /* Parser state */
    parse_state_t state;       /* Interpreter state */
    char *compile_word_name;   /* Word being compiled */
    bool running;              /* Interpreter running flag */
    rforth_error_t last_error; /* Last error code */
};

/* Main API functions */
rforth_ctx_t* rforth_init(void);
void rforth_cleanup(rforth_ctx_t *ctx);
int rforth_repl(rforth_ctx_t *ctx);
int rforth_interpret_file(rforth_ctx_t *ctx, const char *filename);
int rforth_interpret_string(rforth_ctx_t *ctx, const char *input);
int rforth_compile_file(rforth_ctx_t *ctx, const char *input_file, const char *output_file);

/* Builtin word registration */
bool builtins_register(dict_t *dict);

/* Utility functions for builtins */
void builtin_dot_s(rforth_ctx_t *ctx);

#endif /* RFORTH_H */