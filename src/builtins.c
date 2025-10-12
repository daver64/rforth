#include "rforth.h"
#include "turnkey.h"
#include <stdio.h>

/* Forward declarations */
static void builtin_add(rforth_ctx_t *ctx);
static void builtin_sub(rforth_ctx_t *ctx);
static void builtin_mul(rforth_ctx_t *ctx);
static void builtin_div(rforth_ctx_t *ctx);
static void builtin_mod(rforth_ctx_t *ctx);
static void builtin_dup(rforth_ctx_t *ctx);
static void builtin_drop(rforth_ctx_t *ctx);
static void builtin_swap(rforth_ctx_t *ctx);
static void builtin_over(rforth_ctx_t *ctx);
static void builtin_rot(rforth_ctx_t *ctx);
static void builtin_dot(rforth_ctx_t *ctx);
static void builtin_emit(rforth_ctx_t *ctx);
static void builtin_cr(rforth_ctx_t *ctx);
static void builtin_space(rforth_ctx_t *ctx);
static void builtin_words_cmd(rforth_ctx_t *ctx);
static void builtin_bye(rforth_ctx_t *ctx);
static void builtin_equal(rforth_ctx_t *ctx);
static void builtin_less(rforth_ctx_t *ctx);
static void builtin_greater(rforth_ctx_t *ctx);
static void builtin_and(rforth_ctx_t *ctx);
static void builtin_or(rforth_ctx_t *ctx);
static void builtin_not(rforth_ctx_t *ctx);
static void builtin_negate(rforth_ctx_t *ctx);

/* Structure to hold builtin word definitions */
typedef struct {
    const char *name;
    void (*func)(rforth_ctx_t *ctx);
} builtin_word_t;

/* Table of builtin words */
static const builtin_word_t builtin_words[] = {
    /* Arithmetic */
    {"+", builtin_add},
    {"-", builtin_sub},
    {"*", builtin_mul},
    {"/", builtin_div},
    {"MOD", builtin_mod},
    {"NEGATE", builtin_negate},
    
    /* Stack manipulation */
    {"DUP", builtin_dup},
    {"DROP", builtin_drop},
    {"SWAP", builtin_swap},
    {"OVER", builtin_over},
    {"ROT", builtin_rot},
    
    /* I/O */
    {".", builtin_dot},
    {"EMIT", builtin_emit},
    {"CR", builtin_cr},
    {"SPACE", builtin_space},
    
    /* Comparison */
    {"=", builtin_equal},
    {"<", builtin_less},
    {">", builtin_greater},
    
    /* Logic */
    {"AND", builtin_and},
    {"OR", builtin_or},
    {"NOT", builtin_not},
    
    /* System */
    {".S", builtin_dot_s},
    {"WORDS", builtin_words_cmd},
    {"BYE", builtin_bye},
    {"TURNKEY", builtin_turnkey},
    
    /* End marker */
    {NULL, NULL}
};

bool builtins_register(dict_t *dict) {
    if (!dict) return false;
    
    for (int i = 0; builtin_words[i].name; i++) {
        if (!dict_add_builtin(dict, builtin_words[i].name, builtin_words[i].func)) {
            return false;
        }
    }
    
    return true;
}

/* Builtin implementations */

static void builtin_add(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, a + b);
}

static void builtin_sub(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, a - b);
}

static void builtin_mul(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, a * b);
}

static void builtin_div(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    if (b == 0) {
        ctx->last_error = RFORTH_ERROR_PARSE_ERROR; /* Division by zero */
        return;
    }
    stack_push(ctx->data_stack, a / b);
}

static void builtin_mod(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    if (b == 0) {
        ctx->last_error = RFORTH_ERROR_PARSE_ERROR; /* Division by zero */
        return;
    }
    stack_push(ctx->data_stack, a % b);
}

static void builtin_negate(rforth_ctx_t *ctx) {
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, -a);
}

static void builtin_dup(rforth_ctx_t *ctx) {
    if (!stack_dup(ctx->data_stack)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
    }
}

static void builtin_drop(rforth_ctx_t *ctx) {
    if (!stack_drop(ctx->data_stack)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
    }
}

static void builtin_swap(rforth_ctx_t *ctx) {
    if (!stack_swap(ctx->data_stack)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
    }
}

static void builtin_over(rforth_ctx_t *ctx) {
    if (!stack_over(ctx->data_stack)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
    }
}

static void builtin_rot(rforth_ctx_t *ctx) {
    if (!stack_rot(ctx->data_stack)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
    }
}

static void builtin_dot(rforth_ctx_t *ctx) {
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    printf("%ld ", (long)value);
}

static void builtin_emit(rforth_ctx_t *ctx) {
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    printf("%c", (char)value);
}

static void builtin_cr(rforth_ctx_t *ctx) {
    (void)ctx; /* Unused */
    printf("\n");
}

static void builtin_space(rforth_ctx_t *ctx) {
    (void)ctx; /* Unused */
    printf(" ");
}

void builtin_dot_s(rforth_ctx_t *ctx) {
    printf("<%d> ", stack_depth(ctx->data_stack));
    if (!stack_is_empty(ctx->data_stack)) {
        stack_print(ctx->data_stack);
    } else {
        printf("\n");
    }
}

static void builtin_words_cmd(rforth_ctx_t *ctx) {
    dict_print(ctx->dict);
}

static void builtin_bye(rforth_ctx_t *ctx) {
    ctx->running = false;
}

static void builtin_equal(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, (a == b) ? -1 : 0);
}

static void builtin_less(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, (a < b) ? -1 : 0);
}

static void builtin_greater(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, (a > b) ? -1 : 0);
}

static void builtin_and(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, a & b);
}

static void builtin_or(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, a | b);
}

static void builtin_not(rforth_ctx_t *ctx) {
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        ctx->last_error = RFORTH_ERROR_STACK_UNDERFLOW;
        return;
    }
    stack_push(ctx->data_stack, ~a);
}