#include "rforth.h"
#include "turnkey.h"
#include <stdio.h>
#include <math.h>

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

/* Floating point specific words */
static void builtin_f_dot(rforth_ctx_t *ctx);
static void builtin_int_to_float(rforth_ctx_t *ctx);
static void builtin_float_to_int(rforth_ctx_t *ctx);
static void builtin_sqrt(rforth_ctx_t *ctx);
static void builtin_abs(rforth_ctx_t *ctx);

/* Control flow words */
static void builtin_if(rforth_ctx_t *ctx);
static void builtin_then(rforth_ctx_t *ctx);
static void builtin_else(rforth_ctx_t *ctx);
static void builtin_begin(rforth_ctx_t *ctx);
static void builtin_until(rforth_ctx_t *ctx);
static void builtin_while(rforth_ctx_t *ctx);
static void builtin_repeat(rforth_ctx_t *ctx);

/* Variable and memory operations */
static void builtin_variable(rforth_ctx_t *ctx);
static void builtin_constant(rforth_ctx_t *ctx);
static void builtin_fetch(rforth_ctx_t *ctx);
static void builtin_store(rforth_ctx_t *ctx);
static void builtin_plus_store(rforth_ctx_t *ctx);
static void builtin_question(rforth_ctx_t *ctx);

/* Return stack operations */
static void builtin_to_r(rforth_ctx_t *ctx);
static void builtin_from_r(rforth_ctx_t *ctx);
static void builtin_r_fetch(rforth_ctx_t *ctx);

/* Advanced stack operations */
static void builtin_pick(rforth_ctx_t *ctx);
static void builtin_roll(rforth_ctx_t *ctx);
static void builtin_two_dup(rforth_ctx_t *ctx);
static void builtin_two_drop(rforth_ctx_t *ctx);
static void builtin_depth(rforth_ctx_t *ctx);

/* Extended arithmetic */
static void builtin_max(rforth_ctx_t *ctx);
static void builtin_min(rforth_ctx_t *ctx);
static void builtin_one_plus(rforth_ctx_t *ctx);
static void builtin_one_minus(rforth_ctx_t *ctx);
static void builtin_two_star(rforth_ctx_t *ctx);
static void builtin_two_slash(rforth_ctx_t *ctx);

/* Structure to hold builtin word definitions */
typedef struct {
    const char *name;
    void (*func)(rforth_ctx_t *ctx);
} builtin_word_t;

/* Simplified error setting for builtins */
static void set_error_simple(rforth_ctx_t *ctx, rforth_error_t code, const char *message) {
    rforth_set_error(ctx, code, message, "builtin", __FILE__, __LINE__, 0);
}

/* Control flow operations */
static void builtin_if(rforth_ctx_t *ctx) {
    if (ctx->cf_sp >= 32) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Control flow stack overflow");
        return;
    }
    
    ctx->cf_stack[ctx->cf_sp].type = CF_IF;
    ctx->cf_stack[ctx->cf_sp].address = 0;  /* Will be patched later */
    
    cell_t condition;
    if (!stack_pop(ctx->data_stack, &condition)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "IF requires condition on stack");
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
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "THEN without matching IF");
        return;
    }
    
    ctx->cf_sp--;
    if (ctx->cf_stack[ctx->cf_sp].type != CF_IF) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "THEN without matching IF");
        return;
    }
    
    /* THEN just marks end of IF block - no action needed in immediate mode */
}

static void builtin_else(rforth_ctx_t *ctx) {
    if (ctx->cf_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "ELSE without matching IF");
        return;
    }
    
    /* Toggle condition for the IF block */
    ctx->cf_stack[ctx->cf_sp - 1].condition_met = !ctx->cf_stack[ctx->cf_sp - 1].condition_met;
}

/* Simple variable implementation using name-value pairs */
static variable_entry_t* find_variable(rforth_ctx_t *ctx, const char *name) {
    variable_entry_t *var = ctx->variables;
    while (var) {
        if (strcmp(var->name, name) == 0) {
            return var;
        }
        var = var->next;
    }
    return NULL;
}

static void builtin_variable(rforth_ctx_t *ctx) {
    /* Simple implementation: just create the variable, initialized to 0 */
    /* In practice this would need to get name from the input stream */
    /* For now, we'll implement this as a stack-based operation */
    /* Top of stack: name as number (simple case) */
    
    cell_t name_cell;
    if (!stack_pop(ctx->data_stack, &name_cell)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "VARIABLE requires name on stack");
        return;
    }
    
    /* Create variable with numeric name for now */
    variable_entry_t *var = malloc(sizeof(variable_entry_t));
    if (!var) {
        set_error_simple(ctx, RFORTH_ERROR_MEMORY, "Failed to allocate variable");
        return;
    }
    
    snprintf(var->name, sizeof(var->name), "var_%ld", name_cell.value.i);
    var->value = cell_make_int(0);  /* Initialize to 0 */
    var->next = ctx->variables;
    ctx->variables = var;
    
    /* Push variable address onto stack */
    uintptr_t addr = (uintptr_t)var;
    stack_push_int(ctx->data_stack, (int64_t)addr);
}

/* Basic arithmetic extensions */
static void builtin_one_plus(rforth_ctx_t *ctx) {
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "1+ requires value on stack");
        return;
    }
    
    if (a.type == CELL_INT) {
        stack_push_int(ctx->data_stack, a.value.i + 1);
    } else {
        stack_push_float(ctx->data_stack, a.value.f + 1.0);
    }
}

static void builtin_one_minus(rforth_ctx_t *ctx) {
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "1- requires value on stack");
        return;
    }
    
    if (a.type == CELL_INT) {
        stack_push_int(ctx->data_stack, a.value.i - 1);
    } else {
        stack_push_float(ctx->data_stack, a.value.f - 1.0);
    }
}

static void builtin_max(rforth_ctx_t *ctx) {
    cell_t a, b;
    if (!stack_pop(ctx->data_stack, &a) || !stack_pop(ctx->data_stack, &b)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "MAX requires two values on stack");
        return;
    }
    
    /* Compare and push maximum */
    if (b.type == CELL_INT && a.type == CELL_INT) {
        int64_t result = (b.value.i > a.value.i) ? b.value.i : a.value.i;
        stack_push_int(ctx->data_stack, result);
    } else {
        double val_b = (b.type == CELL_INT) ? (double)b.value.i : b.value.f;
        double val_a = (a.type == CELL_INT) ? (double)a.value.i : a.value.f;
        double result = (val_b > val_a) ? val_b : val_a;
        stack_push_float(ctx->data_stack, result);
    }
}

static void builtin_min(rforth_ctx_t *ctx) {
    cell_t a, b;
    if (!stack_pop(ctx->data_stack, &a) || !stack_pop(ctx->data_stack, &b)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "MIN requires two values on stack");
        return;
    }
    
    /* Compare and push minimum */
    if (b.type == CELL_INT && a.type == CELL_INT) {
        int64_t result = (b.value.i < a.value.i) ? b.value.i : a.value.i;
        stack_push_int(ctx->data_stack, result);
    } else {
        double val_b = (b.type == CELL_INT) ? (double)b.value.i : b.value.f;
        double val_a = (a.type == CELL_INT) ? (double)a.value.i : a.value.f;
        double result = (val_b < val_a) ? val_b : val_a;
        stack_push_float(ctx->data_stack, result);
    }
}

/* Table of builtin words */
static const builtin_word_t builtin_words[] = {
    /* Arithmetic */
    {"+", builtin_add},
    {"-", builtin_sub},
    {"*", builtin_mul},
    {"/", builtin_div},
    {"mod", builtin_mod},
    {"negate", builtin_negate},
    
    /* Stack manipulation */
    {"dup", builtin_dup},
    {"drop", builtin_drop},
    {"swap", builtin_swap},
    {"over", builtin_over},
    {"rot", builtin_rot},
    
    /* I/O */
    {".", builtin_dot},
    {"emit", builtin_emit},
    {"cr", builtin_cr},
    {"space", builtin_space},
    
    /* Comparison */
    {"=", builtin_equal},
    {"<", builtin_less},
    {">", builtin_greater},
    
    /* Logic */
    {"and", builtin_and},
    {"or", builtin_or},
    {"not", builtin_not},
    
    /* Floating point */
    {"f.", builtin_f_dot},
    {">float", builtin_int_to_float},
    {">int", builtin_float_to_int},
    {"sqrt", builtin_sqrt},
    {"abs", builtin_abs},
    
    /* System */
    {".s", builtin_dot_s},
    {"words", builtin_words_cmd},
    {"bye", builtin_bye},
    {"turnkey", builtin_turnkey},
    
    /* Control flow (basic) */
    {"if", builtin_if},
    {"then", builtin_then},
    {"else", builtin_else},
    
    /* Variables */
    {"variable", builtin_variable},
    
    /* Extended arithmetic */
    {"1+", builtin_one_plus},
    {"1-", builtin_one_minus},
    {"max", builtin_max},
    {"min", builtin_min},
    
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
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    /* If either operand is float, do floating point arithmetic */
    if (a.type == CELL_FLOAT || b.type == CELL_FLOAT) {
        double result = cell_to_float(&a) + cell_to_float(&b);
        stack_push_float(ctx->data_stack, result);
    } else {
        int64_t result = a.value.i + b.value.i;
        stack_push_int(ctx->data_stack, result);
    }
}

static void builtin_sub(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    /* If either operand is float, do floating point arithmetic */
    if (a.type == CELL_FLOAT || b.type == CELL_FLOAT) {
        double result = cell_to_float(&a) - cell_to_float(&b);
        stack_push_float(ctx->data_stack, result);
    } else {
        int64_t result = a.value.i - b.value.i;
        stack_push_int(ctx->data_stack, result);
    }
}

static void builtin_mul(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    /* If either operand is float, do floating point arithmetic */
    if (a.type == CELL_FLOAT || b.type == CELL_FLOAT) {
        double result = cell_to_float(&a) * cell_to_float(&b);
        stack_push_float(ctx->data_stack, result);
    } else {
        int64_t result = a.value.i * b.value.i;
        stack_push_int(ctx->data_stack, result);
    }
}

static void builtin_div(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    /* Check for division by zero */
    if ((b.type == CELL_INT && b.value.i == 0) || 
        (b.type == CELL_FLOAT && b.value.f == 0.0)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_DIVISION_BY_ZERO, "Division by zero");
        return;
    }
    
    /* If either operand is float, do floating point arithmetic */
    if (a.type == CELL_FLOAT || b.type == CELL_FLOAT) {
        double result = cell_to_float(&a) / cell_to_float(&b);
        stack_push_float(ctx->data_stack, result);
    } else {
        int64_t result = a.value.i / b.value.i;
        stack_push_int(ctx->data_stack, result);
    }
}

static void builtin_mod(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    /* Mod only works with integers */
    if (a.type != CELL_INT || b.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_INVALID_OPERATION, "Mod requires integer operands");
        return;
    }
    
    if (b.value.i == 0) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_DIVISION_BY_ZERO, "Division by zero");
        return;
    }
    
    int64_t result = a.value.i % b.value.i;
    stack_push_int(ctx->data_stack, result);
}

static void builtin_negate(rforth_ctx_t *ctx) {
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    if (a.type == CELL_INT) {
        stack_push_int(ctx->data_stack, -a.value.i);
    } else {
        stack_push_float(ctx->data_stack, -a.value.f);
    }
}

static void builtin_dup(rforth_ctx_t *ctx) {
    if (!stack_dup(ctx->data_stack)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
    }
}

static void builtin_drop(rforth_ctx_t *ctx) {
    if (!stack_drop(ctx->data_stack)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
    }
}

static void builtin_swap(rforth_ctx_t *ctx) {
    if (!stack_swap(ctx->data_stack)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
    }
}

static void builtin_over(rforth_ctx_t *ctx) {
    if (!stack_over(ctx->data_stack)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
    }
}

static void builtin_rot(rforth_ctx_t *ctx) {
    if (!stack_rot(ctx->data_stack)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
    }
}

static void builtin_dot(rforth_ctx_t *ctx) {
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    if (value.type == CELL_INT) {
        printf("%ld ", (long)value.value.i);
    } else {
        printf("%.6g ", value.value.f);
    }
}

static void builtin_emit(rforth_ctx_t *ctx) {
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    int ch;
    if (value.type == CELL_INT) {
        ch = (int)value.value.i;
    } else {
        ch = (int)value.value.f;
    }
    printf("%c", ch);
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
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    bool result;
    if (a.type == CELL_INT && b.type == CELL_INT) {
        result = (a.value.i == b.value.i);
    } else {
        result = (cell_to_float(&a) == cell_to_float(&b));
    }
    
    stack_push_int(ctx->data_stack, result ? -1 : 0);
}

static void builtin_less(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    bool result;
    if (a.type == CELL_INT && b.type == CELL_INT) {
        result = (a.value.i < b.value.i);
    } else {
        result = (cell_to_float(&a) < cell_to_float(&b));
    }
    
    stack_push_int(ctx->data_stack, result ? -1 : 0);
}

static void builtin_greater(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    bool result;
    if (a.type == CELL_INT && b.type == CELL_INT) {
        result = (a.value.i > b.value.i);
    } else {
        result = (cell_to_float(&a) > cell_to_float(&b));
    }
    
    stack_push_int(ctx->data_stack, result ? -1 : 0);
}

static void builtin_and(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    /* Bitwise operations only work with integers */
    if (a.type != CELL_INT || b.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_INVALID_OPERATION, "Bitwise operations require integer operands");
        return;
    }
    
    stack_push_int(ctx->data_stack, a.value.i & b.value.i);
}

static void builtin_or(rforth_ctx_t *ctx) {
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    /* Bitwise operations only work with integers */
    if (a.type != CELL_INT || b.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_INVALID_OPERATION, "Bitwise operations require integer operands");
        return;
    }
    
    stack_push_int(ctx->data_stack, a.value.i | b.value.i);
}

static void builtin_not(rforth_ctx_t *ctx) {
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    /* Bitwise operations only work with integers */
    if (a.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_INVALID_OPERATION, "Bitwise operations require integer operands");
        return;
    }
    
    stack_push_int(ctx->data_stack, ~a.value.i);
}

/* Floating point specific words */

static void builtin_f_dot(rforth_ctx_t *ctx) {
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    /* Always print as floating point */
    printf("%.6g ", cell_to_float(&value));
}

static void builtin_int_to_float(rforth_ctx_t *ctx) {
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    if (value.type == CELL_INT) {
        stack_push_float(ctx->data_stack, (double)value.value.i);
    } else {
        stack_push_cell(ctx->data_stack, value);  /* Already float */
    }
}

static void builtin_float_to_int(rforth_ctx_t *ctx) {
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    if (value.type == CELL_FLOAT) {
        stack_push_int(ctx->data_stack, (int64_t)value.value.f);
    } else {
        stack_push_cell(ctx->data_stack, value);  /* Already int */
    }
}

static void builtin_sqrt(rforth_ctx_t *ctx) {
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    double input = cell_to_float(&value);
    if (input < 0.0) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_INVALID_OPERATION, "Square root of negative number");
        return;
    }
    
    stack_push_float(ctx->data_stack, sqrt(input));
}

static void builtin_abs(rforth_ctx_t *ctx) {
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "Stack underflow");
        return;
    }
    
    if (value.type == CELL_INT) {
        int64_t abs_val = value.value.i < 0 ? -value.value.i : value.value.i;
        stack_push_int(ctx->data_stack, abs_val);
    } else {
        stack_push_float(ctx->data_stack, fabs(value.value.f));
    }
}