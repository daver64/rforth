#ifndef RFORTH_H
#define RFORTH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Configuration constants */
#include "config.h"

/* Include other headers */
#include "stack.h"
#include "dict.h"
#include "parser.h"
#include "compiler.h"
#include "io.h"

/* Error codes */
typedef enum {
    RFORTH_OK = 0,
    
    /* Stack errors */
    RFORTH_ERROR_STACK_UNDERFLOW,
    RFORTH_ERROR_STACK_OVERFLOW,
    RFORTH_ERROR_RETURN_STACK_UNDERFLOW,
    RFORTH_ERROR_RETURN_STACK_OVERFLOW,
    
    /* Dictionary errors */
    RFORTH_ERROR_WORD_NOT_FOUND,
    RFORTH_ERROR_WORD_REDEFINED,
    RFORTH_ERROR_DICT_FULL,
    
    /* Parser errors */
    RFORTH_ERROR_PARSE,
    RFORTH_ERROR_PARSE_ERROR,
    RFORTH_ERROR_CONTROL_FLOW,
    RFORTH_ERROR_SYNTAX_ERROR,
    RFORTH_ERROR_UNEXPECTED_EOF,
    RFORTH_ERROR_INVALID_NUMBER,
    RFORTH_ERROR_UNTERMINATED_STRING,
    
    /* File errors */
    RFORTH_ERROR_FILE_NOT_FOUND,
    RFORTH_ERROR_FILE_READ_ERROR,
    RFORTH_ERROR_FILE_WRITE_ERROR,
    RFORTH_ERROR_FILE_PERMISSION,
    
    /* Compiler errors */
    RFORTH_ERROR_COMPILE_ERROR,
    RFORTH_ERROR_COMPILER_NOT_FOUND,
    RFORTH_ERROR_LINK_ERROR,
    
    /* Memory errors */
    RFORTH_ERROR_MEMORY,
    RFORTH_ERROR_NULL_POINTER,
    RFORTH_ERROR_BUFFER_OVERFLOW,
    
    /* I/O errors */
    RFORTH_ERROR_IO_TIMEOUT,
    RFORTH_ERROR_IO_FAILURE,
    RFORTH_ERROR_IO_NOT_AVAILABLE,
    
    /* Runtime errors */
    RFORTH_ERROR_DIVISION_BY_ZERO,
    RFORTH_ERROR_INVALID_ADDRESS,
    RFORTH_ERROR_DUPLICATE_WORD,
    RFORTH_ERROR_TYPE_MISMATCH,
    RFORTH_ERROR_INVALID_OPERATION,
    RFORTH_ERROR_EXECUTION_ERROR,
    RFORTH_ERROR_ABORT,
    RFORTH_ERROR_NOT_IMPLEMENTED,
    
    /* Generic error */
    RFORTH_ERROR_UNKNOWN
} rforth_error_t;

/* Error context for detailed error reporting */
typedef struct {
    rforth_error_t code;
    const char *message;
    const char *function;
    const char *filename;
    int line;
    int column;
} rforth_error_context_t;

/* Forward declarations */
typedef struct rforth_ctx rforth_ctx_t;

/* Control flow types */
typedef enum {
    CF_IF,
    CF_BEGIN,
    CF_DO
} control_flow_type_t;

/* Control flow stack entry */
typedef struct {
    control_flow_type_t type;
    int64_t address;        /* Address or flag value */
    bool condition_met;     /* For conditional constructs */
} control_flow_entry_t;

/* Variable entry */
typedef struct variable_entry {
    char name[MAX_WORD_LENGTH];
    cell_t value;
    struct variable_entry *next;
} variable_entry_t;

/* Main context structure */
struct rforth_ctx {
    rforth_stack_t *data_stack;        /* Data stack */
    rforth_stack_t *return_stack;      /* Return stack */
    dict_t *dict;              /* Word dictionary */
    parser_t *parser;          /* Parser state */
    parse_state_t state;       /* Interpreter state */
    char *compile_word_name;   /* Word being compiled */
    bool running;              /* Interpreter running flag */
    rforth_error_context_t last_error; /* Last error with context */
    
    /* Control flow stack */
    control_flow_entry_t cf_stack[32];  /* Control flow stack */
    int cf_sp;                           /* Control flow stack pointer */
    
    /* Skip control for true control flow */
    bool skip_mode;                      /* True when skipping execution */
    int skip_depth;                      /* Nesting depth during skip */
    
    /* Loop control */
    const char *loop_start[32];          /* Loop start positions */
    int loop_sp;                         /* Loop stack pointer */
    
    /* DO/LOOP support */
    int64_t loop_index[32];              /* Current loop indices */
    int64_t loop_limit[32];              /* Loop limits */
    int do_loop_sp;                      /* DO/LOOP stack pointer */
    
    /* Variables */
    variable_entry_t *variables;         /* Variable linked list */
    
    /* System variables for ANSI compliance */
    char *here_ptr;                      /* Dictionary HERE pointer */
    int64_t numeric_base;                /* Current numeric base (default 10) */
    int64_t state_var;                   /* STATE variable (0=interpret, -1=compile) */
    
    /* Numeric formatting buffer for ANSI words */
    char format_buffer[256];             /* Buffer for numeric formatting */
    int format_pos;                      /* Current position in format buffer */
};

/* Main API functions */
rforth_ctx_t* rforth_init(void);
void rforth_cleanup(rforth_ctx_t *ctx);
int rforth_repl(rforth_ctx_t *ctx);
int rforth_interpret_file(rforth_ctx_t *ctx, const char *filename);
int rforth_interpret_string(rforth_ctx_t *ctx, const char *input);
int rforth_compile_file(rforth_ctx_t *ctx, const char *input_file, const char *output_file);

/* Error handling functions */
void rforth_set_error(rforth_ctx_t *ctx, rforth_error_t code, const char *message, 
                      const char *function, const char *filename, int line, int column);
void rforth_clear_error(rforth_ctx_t *ctx);
const char* rforth_error_string(rforth_error_t code);
void rforth_print_error(rforth_ctx_t *ctx);

/* Error handling macros */
#define RFORTH_SET_ERROR(ctx, code, message) \
    rforth_set_error(ctx, code, message, __func__, __FILE__, __LINE__, 0)

#define RFORTH_SET_PARSE_ERROR(ctx, code, message, line, col) \
    rforth_set_error(ctx, code, message, __func__, __FILE__, line, col)

/* Builtin word registration */
bool builtins_register(dict_t *dict);

/* Utility functions for builtins */
void builtin_dot_s(rforth_ctx_t *ctx);

#endif /* RFORTH_H */