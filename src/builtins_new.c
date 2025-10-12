#include "rforth.h"
#include "turnkey.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

// Simplified error setting for builtins
static void set_error(rforth_ctx_t *ctx, rforth_error_t code, const char *message) {
    rforth_set_error(ctx, code, message, __func__, __FILE__, __LINE__, 0);
}

/* Control flow operations */
static void builtin_if(rforth_ctx_t *ctx) {
    if (ctx->cf_sp >= 32) {
        set_error(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Control flow stack overflow");
        return;
    }
    
    ctx->cf_stack[ctx->cf_sp].type = CF_IF;
    ctx->cf_stack[ctx->cf_sp].address = 0;  /* Will be patched later */
    
    cell_t condition;
    if (!stack_pop(ctx->data_stack, &condition)) {
        set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "IF requires condition on stack");
        return;
    }
    
    /* Evaluate condition (non-zero is true) */
    bool is_true = false;
    if (condition.type == CELL_INT) {
        is_true = (condition.value.i != 0);
    } else {
        is_true = (condition.value.f != 0.0);
    }
    
    ctx->cf_stack[ctx->cf_sp].condition_met = is_true;
    ctx->cf_sp++;
}

static void builtin_then(rforth_ctx_t *ctx) {
    if (ctx->cf_sp <= 0) {
        set_error(ctx, RFORTH_ERROR_CONTROL_FLOW, "THEN without matching IF");
        return;
    }
    
    ctx->cf_sp--;
    if (ctx->cf_stack[ctx->cf_sp].type != CF_IF) {
        set_error(ctx, RFORTH_ERROR_CONTROL_FLOW, "THEN without matching IF");
        return;
    }
    
    /* THEN just marks end of IF block - no action needed in immediate mode */
}

static void builtin_else(rforth_ctx_t *ctx) {
    if (ctx->cf_sp <= 0) {
        set_error(ctx, RFORTH_ERROR_CONTROL_FLOW, "ELSE without matching IF");
        return;
    }
    
    /* Toggle condition for the IF block */
    ctx->cf_stack[ctx->cf_sp - 1].condition_met = !ctx->cf_stack[ctx->cf_sp - 1].condition_met;
}

/* Control flow words to add to builtin table */
static const builtin_word_t control_flow_words[] = {
    {"if", builtin_if},
    {"then", builtin_then},
    {"else", builtin_else},
    {NULL, NULL}
};