#include "rforth.h"
#include "turnkey.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>

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
    /* IF - Begin conditional execution ( flag -- ) */
    
    /* If we're already skipping, just track nesting depth */
    if (ctx->skip_mode) {
        ctx->skip_depth++;
        return;
    }
    
    /* Pop condition from stack */
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
    
    /* If condition is false, enter skip mode */
    if (!is_true) {
        ctx->skip_mode = true;
        ctx->skip_depth = 1;  /* Start tracking nesting */
    }
    
    /* Push IF onto control flow stack for ELSE/THEN matching */
    if (ctx->cf_sp >= 32) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Control flow stack overflow");
        return;
    }
    
    ctx->cf_stack[ctx->cf_sp].type = CF_IF;
    ctx->cf_stack[ctx->cf_sp].condition_met = is_true;
    ctx->cf_sp++;
}

static void builtin_then(rforth_ctx_t *ctx) {
    /* THEN - End conditional execution */
    
    /* If we're skipping and this is a nested IF, just decrement depth */
    if (ctx->skip_mode && ctx->skip_depth > 1) {
        ctx->skip_depth--;
        return;
    }
    
    /* Check control flow stack */
    if (ctx->cf_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "THEN without matching IF");
        return;
    }
    
    ctx->cf_sp--;
    if (ctx->cf_stack[ctx->cf_sp].type != CF_IF) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "THEN without matching IF");
        return;
    }
    
    /* If we were skipping due to false IF, stop skipping */
    if (ctx->skip_mode) {
        ctx->skip_mode = false;
        ctx->skip_depth = 0;
    }
}

static void builtin_else(rforth_ctx_t *ctx) {
    /* ELSE - Switch between IF/ELSE branches */
    
    /* If we're in nested skip mode, ignore this ELSE */
    if (ctx->skip_mode && ctx->skip_depth > 1) {
        return;
    }
    
    /* Check control flow stack */
    if (ctx->cf_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "ELSE without matching IF");
        return;
    }
    
    if (ctx->cf_stack[ctx->cf_sp - 1].type != CF_IF) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "ELSE without matching IF");
        return;
    }
    
    /* Toggle execution: if we were executing, start skipping; if skipping, start executing */
    if (ctx->skip_mode) {
        /* We were skipping the IF branch, now execute the ELSE branch */
        ctx->skip_mode = false;
        ctx->skip_depth = 0;
    } else {
        /* We were executing the IF branch, now skip the ELSE branch */
        ctx->skip_mode = true;
        ctx->skip_depth = 1;
    }
    
    /* Update control flow stack to reflect the toggle */
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

/* Advanced stack operations */
static void builtin_pick(rforth_ctx_t *ctx) {
    /* PICK - Copy nth stack item to top ( n -- x ) */
    cell_t index_cell;
    if (!stack_pop(ctx->data_stack, &index_cell)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "PICK requires index on stack");
        return;
    }
    
    if (index_cell.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "PICK requires integer index");
        return;
    }
    
    int64_t index = index_cell.value.i;
    cell_t value;
    if (!stack_pick(ctx->data_stack, (int)index, &value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "PICK index out of range");
        return;
    }
    
    stack_push_cell(ctx->data_stack, value);
}

static void builtin_roll(rforth_ctx_t *ctx) {
    /* ROLL - Move nth stack item to top ( n -- ) */
    cell_t count_cell;
    if (!stack_pop(ctx->data_stack, &count_cell)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "ROLL requires count on stack");
        return;
    }
    
    if (count_cell.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "ROLL requires integer count");
        return;
    }
    
    int64_t count = count_cell.value.i;
    if (count <= 0) return;  /* No-op for count <= 0 */
    
    /* Roll stack by moving nth item to top */
    cell_t temp[32];  /* Temporary storage */
    if (count >= 32) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_OPERATION, "ROLL count too large");
        return;
    }
    
    /* Pop count+1 items */
    for (int i = 0; i <= count; i++) {
        if (!stack_pop(ctx->data_stack, &temp[i])) {
            set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "ROLL insufficient stack depth");
            return;
        }
    }
    
    /* Push back in rolled order: item at count goes to top */
    stack_push_cell(ctx->data_stack, temp[count]);
    for (int i = 0; i < count; i++) {
        stack_push_cell(ctx->data_stack, temp[i]);
    }
}

static void builtin_depth(rforth_ctx_t *ctx) {
    /* DEPTH - Get current stack depth ( -- n ) */
    int depth = stack_depth(ctx->data_stack);
    stack_push_int(ctx->data_stack, depth);
}

static void builtin_two_dup(rforth_ctx_t *ctx) {
    /* 2DUP - Duplicate top two stack items ( a b -- a b a b ) */
    cell_t a, b;
    if (!stack_pop(ctx->data_stack, &a) || !stack_pop(ctx->data_stack, &b)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "2DUP requires two values on stack");
        return;
    }
    
    /* Push back: b a b a */
    stack_push_cell(ctx->data_stack, b);
    stack_push_cell(ctx->data_stack, a);
    stack_push_cell(ctx->data_stack, b);
    stack_push_cell(ctx->data_stack, a);
}

static void builtin_two_drop(rforth_ctx_t *ctx) {
    /* 2DROP - Drop top two stack items ( a b -- ) */
    cell_t dummy;
    if (!stack_pop(ctx->data_stack, &dummy) || !stack_pop(ctx->data_stack, &dummy)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "2DROP requires two values on stack");
        return;
    }
}

static void builtin_two_swap(rforth_ctx_t *ctx) {
    /* 2SWAP - Swap top two pairs ( a b c d -- c d a b ) */
    cell_t a, b, c, d;
    if (!stack_pop(ctx->data_stack, &a) || !stack_pop(ctx->data_stack, &b) ||
        !stack_pop(ctx->data_stack, &c) || !stack_pop(ctx->data_stack, &d)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "2SWAP requires four values on stack");
        return;
    }
    
    /* Push back in swapped order: c d a b */
    stack_push_cell(ctx->data_stack, b);
    stack_push_cell(ctx->data_stack, a);
    stack_push_cell(ctx->data_stack, d);
    stack_push_cell(ctx->data_stack, c);
}

/* Return stack operations */
static void builtin_to_r(rforth_ctx_t *ctx) {
    /* >R - Move value from data stack to return stack */
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, ">R requires value on data stack");
        return;
    }
    
    if (!stack_push_cell(ctx->return_stack, value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Return stack overflow");
        return;
    }
}

static void builtin_from_r(rforth_ctx_t *ctx) {
    /* R> - Move value from return stack to data stack */
    cell_t value;
    if (!stack_pop(ctx->return_stack, &value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "R> requires value on return stack");
        return;
    }
    
    if (!stack_push_cell(ctx->data_stack, value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Data stack overflow");
        return;
    }
}

static void builtin_r_fetch(rforth_ctx_t *ctx) {
    /* R@ - Copy top of return stack to data stack */
    cell_t value;
    if (!stack_peek(ctx->return_stack, &value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "R@ requires value on return stack");
        return;
    }
    
    if (!stack_push_cell(ctx->data_stack, value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Data stack overflow");
        return;
    }
}

/* Extended arithmetic operations */
static void builtin_two_star(rforth_ctx_t *ctx) {
    /* 2* - Multiply by 2 ( n -- n*2 ) */
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "2* requires value on stack");
        return;
    }
    
    if (a.type == CELL_INT) {
        stack_push_int(ctx->data_stack, a.value.i * 2);
    } else {
        stack_push_float(ctx->data_stack, a.value.f * 2.0);
    }
}

static void builtin_two_slash(rforth_ctx_t *ctx) {
    /* 2/ - Divide by 2 ( n -- n/2 ) */
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "2/ requires value on stack");
        return;
    }
    
    if (a.type == CELL_INT) {
        stack_push_int(ctx->data_stack, a.value.i / 2);
    } else {
        stack_push_float(ctx->data_stack, a.value.f / 2.0);
    }
}

static void builtin_slash_mod(rforth_ctx_t *ctx) {
    /* /MOD - Divide with remainder ( n1 n2 -- rem quot ) */
    cell_t divisor, dividend;
    if (!stack_pop(ctx->data_stack, &divisor) || !stack_pop(ctx->data_stack, &dividend)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "/MOD requires two values on stack");
        return;
    }
    
    if (divisor.type != CELL_INT || dividend.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "/MOD requires integer operands");
        return;
    }
    
    if (divisor.value.i == 0) {
        set_error_simple(ctx, RFORTH_ERROR_DIVISION_BY_ZERO, "/MOD division by zero");
        return;
    }
    
    int64_t quot = dividend.value.i / divisor.value.i;
    int64_t rem = dividend.value.i % divisor.value.i;
    
    stack_push_int(ctx->data_stack, rem);   /* remainder first */
    stack_push_int(ctx->data_stack, quot);  /* quotient second */
}

static void builtin_question_dup(rforth_ctx_t *ctx) {
    /* ?DUP - Duplicate if non-zero ( n -- n n | n ) */
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "?DUP requires value on stack");
        return;
    }
    
    /* Always push the value back */
    stack_push_cell(ctx->data_stack, a);
    
    /* If non-zero, duplicate it */
    bool is_nonzero = false;
    if (a.type == CELL_INT) {
        is_nonzero = (a.value.i != 0);
    } else {
        is_nonzero = (a.value.f != 0.0);
    }
    
    if (is_nonzero) {
        stack_push_cell(ctx->data_stack, a);
    }
}

static void builtin_zero_equals(rforth_ctx_t *ctx) {
    /* 0= - Test for zero ( n -- flag ) */
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "0= requires value on stack");
        return;
    }
    
    bool is_zero = false;
    if (a.type == CELL_INT) {
        is_zero = (a.value.i == 0);
    } else {
        is_zero = (a.value.f == 0.0);
    }
    
    stack_push_int(ctx->data_stack, is_zero ? -1 : 0);  /* Forth true is -1 */
}

static void builtin_zero_less(rforth_ctx_t *ctx) {
    /* 0< - Test for negative ( n -- flag ) */
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "0< requires value on stack");
        return;
    }
    
    bool is_negative = false;
    if (a.type == CELL_INT) {
        is_negative = (a.value.i < 0);
    } else {
        is_negative = (a.value.f < 0.0);
    }
    
    stack_push_int(ctx->data_stack, is_negative ? -1 : 0);
}

static void builtin_zero_greater(rforth_ctx_t *ctx) {
    /* 0> - Test for positive ( n -- flag ) */
    cell_t a;
    if (!stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "0> requires value on stack");
        return;
    }
    
    bool is_positive = false;
    if (a.type == CELL_INT) {
        is_positive = (a.value.i > 0);
    } else {
        is_positive = (a.value.f > 0.0);
    }
    
    stack_push_int(ctx->data_stack, is_positive ? -1 : 0);
}

/* Additional comparison operators */
static void builtin_not_equals(rforth_ctx_t *ctx) {
    /* <> - Not equal ( n1 n2 -- flag ) */
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "<> requires two values on stack");
        return;
    }
    
    bool not_equal = false;
    if (a.type == CELL_INT && b.type == CELL_INT) {
        not_equal = (a.value.i != b.value.i);
    } else if (a.type == CELL_FLOAT && b.type == CELL_FLOAT) {
        not_equal = (a.value.f != b.value.f);
    } else if (a.type == CELL_INT && b.type == CELL_FLOAT) {
        not_equal = ((double)a.value.i != b.value.f);
    } else if (a.type == CELL_FLOAT && b.type == CELL_INT) {
        not_equal = (a.value.f != (double)b.value.i);
    }
    
    stack_push_int(ctx->data_stack, not_equal ? -1 : 0);
}

static void builtin_greater_equals(rforth_ctx_t *ctx) {
    /* >= - Greater than or equal ( n1 n2 -- flag ) */
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, ">= requires two values on stack");
        return;
    }
    
    bool greater_equal = false;
    if (a.type == CELL_INT && b.type == CELL_INT) {
        greater_equal = (a.value.i >= b.value.i);
    } else if (a.type == CELL_FLOAT && b.type == CELL_FLOAT) {
        greater_equal = (a.value.f >= b.value.f);
    } else if (a.type == CELL_INT && b.type == CELL_FLOAT) {
        greater_equal = ((double)a.value.i >= b.value.f);
    } else if (a.type == CELL_FLOAT && b.type == CELL_INT) {
        greater_equal = (a.value.f >= (double)b.value.i);
    }
    
    stack_push_int(ctx->data_stack, greater_equal ? -1 : 0);
}

static void builtin_less_equals(rforth_ctx_t *ctx) {
    /* <= - Less than or equal ( n1 n2 -- flag ) */
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "<= requires two values on stack");
        return;
    }
    
    bool less_equal = false;
    if (a.type == CELL_INT && b.type == CELL_INT) {
        less_equal = (a.value.i <= b.value.i);
    } else if (a.type == CELL_FLOAT && b.type == CELL_FLOAT) {
        less_equal = (a.value.f <= b.value.f);
    } else if (a.type == CELL_INT && b.type == CELL_FLOAT) {
        less_equal = ((double)a.value.i <= b.value.f);
    } else if (a.type == CELL_FLOAT && b.type == CELL_INT) {
        less_equal = (a.value.f <= (double)b.value.i);
    }
    
    stack_push_int(ctx->data_stack, less_equal ? -1 : 0);
}

/* Loop constructs */
static void builtin_begin(rforth_ctx_t *ctx) {
    /* BEGIN - Start indefinite loop */
    
    /* If we're in skip mode, track nesting */
    if (ctx->skip_mode) {
        ctx->skip_depth++;
        return;
    }
    
    /* Push BEGIN onto control flow stack */
    if (ctx->cf_sp >= 32) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Control flow stack overflow");
        return;
    }
    
    /* Save current parser position for loop restart */
    if (ctx->loop_sp >= 32) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Loop stack overflow");
        return;
    }
    
    ctx->loop_start[ctx->loop_sp] = ctx->parser->current;
    ctx->loop_sp++;
    
    ctx->cf_stack[ctx->cf_sp].type = CF_BEGIN;
    ctx->cf_stack[ctx->cf_sp].address = ctx->loop_sp - 1;  /* Index into loop_start array */
    ctx->cf_sp++;
}

static void builtin_until(rforth_ctx_t *ctx) {
    /* UNTIL - End loop if condition is true ( flag -- ) */
    
    /* If we're skipping and this is nested, just decrement depth */
    if (ctx->skip_mode && ctx->skip_depth > 1) {
        ctx->skip_depth--;
        return;
    }
    
    /* Check control flow stack */
    if (ctx->cf_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "UNTIL without matching BEGIN");
        return;
    }
    
    ctx->cf_sp--;
    if (ctx->cf_stack[ctx->cf_sp].type != CF_BEGIN) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "UNTIL without matching BEGIN");
        return;
    }
    
    /* If we were skipping, stop now */
    if (ctx->skip_mode) {
        ctx->skip_mode = false;
        ctx->skip_depth = 0;
        ctx->loop_sp--;  /* Pop loop stack */
        return;
    }
    
    /* Pop condition from stack */
    cell_t condition;
    if (!stack_pop(ctx->data_stack, &condition)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "UNTIL requires condition on stack");
        return;
    }
    
    /* Check if condition is true (non-zero) */
    bool exit_loop = false;
    if (condition.type == CELL_INT) {
        exit_loop = (condition.value.i != 0);
    } else {
        exit_loop = (condition.value.f != 0.0);
    }
    
    if (exit_loop) {
        /* Exit loop - pop loop stack */
        ctx->loop_sp--;
    } else {
        /* Continue loop - restore parser position */
        int loop_index = ctx->cf_stack[ctx->cf_sp].address;
        ctx->parser->current = ctx->loop_start[loop_index];
        
        /* Push BEGIN back onto control flow stack for next iteration */
        ctx->cf_stack[ctx->cf_sp].type = CF_BEGIN;
        ctx->cf_stack[ctx->cf_sp].address = loop_index;
        ctx->cf_sp++;
    }
}

static void builtin_while(rforth_ctx_t *ctx) {
    /* WHILE - Continue loop if condition is true ( flag -- ) */
    
    /* If we're in skip mode, track nesting */
    if (ctx->skip_mode) {
        return;  /* Don't change skip state */
    }
    
    /* Pop condition from stack */
    cell_t condition;
    if (!stack_pop(ctx->data_stack, &condition)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "WHILE requires condition on stack");
        return;
    }
    
    /* Check if condition is false */
    bool continue_loop = false;
    if (condition.type == CELL_INT) {
        continue_loop = (condition.value.i != 0);
    } else {
        continue_loop = (condition.value.f != 0.0);
    }
    
    if (!continue_loop) {
        /* Condition is false - enter skip mode until REPEAT */
        ctx->skip_mode = true;
        ctx->skip_depth = 1;
    }
}

static void builtin_repeat(rforth_ctx_t *ctx) {
    /* REPEAT - End of BEGIN/WHILE loop */
    
    /* If we're skipping due to false WHILE, stop skipping and exit loop */
    if (ctx->skip_mode) {
        ctx->skip_mode = false;
        ctx->skip_depth = 0;
        
        /* Pop control flow and loop stacks */
        if (ctx->cf_sp > 0) ctx->cf_sp--;
        if (ctx->loop_sp > 0) ctx->loop_sp--;
        return;
    }
    
    /* Check control flow stack */
    if (ctx->cf_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "REPEAT without matching BEGIN");
        return;
    }
    
    ctx->cf_sp--;
    if (ctx->cf_stack[ctx->cf_sp].type != CF_BEGIN) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "REPEAT without matching BEGIN");
        return;
    }
    
    /* Loop back to BEGIN */
    int loop_index = ctx->cf_stack[ctx->cf_sp].address;
    ctx->parser->current = ctx->loop_start[loop_index];
    
    /* Push BEGIN back onto control flow stack for next iteration */
    ctx->cf_stack[ctx->cf_sp].type = CF_BEGIN;
    ctx->cf_stack[ctx->cf_sp].address = loop_index;
    ctx->cf_sp++;
}

/* String operations */
static void builtin_s_quote(rforth_ctx_t *ctx) {
    /* S" - Compile string literal ( -- addr len ) */
    /* For interpreted mode, we need to parse until closing quote */
    /* This is a simplified implementation - proper Forth would compile */
    
    /* Find the closing quote in the input stream */
    const char *input = ctx->parser->current;
    if (!input) {
        set_error_simple(ctx, RFORTH_ERROR_UNTERMINATED_STRING, "S\" requires string");
        return;
    }
    
    /* Skip whitespace */
    while (*input && isspace(*input)) input++;
    
    const char *start = input;
    const char *end = strchr(start, '"');
    if (!end) {
        set_error_simple(ctx, RFORTH_ERROR_UNTERMINATED_STRING, "Unterminated string literal");
        return;
    }
    
    /* Calculate length */
    size_t len = end - start;
    
    /* Allocate string storage (simplified - should use heap) */
    static char string_buffer[1024];
    if (len >= sizeof(string_buffer)) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "String too long");
        return;
    }
    
    strncpy(string_buffer, start, len);
    string_buffer[len] = '\0';
    
    /* Push address and length */
    stack_push_int(ctx->data_stack, (long)string_buffer);
    stack_push_int(ctx->data_stack, (long)len);
    
    /* Advance input pointer past closing quote */
    ctx->parser->current = end + 1;
}

static void builtin_dot_quote(rforth_ctx_t *ctx) {
    /* ." - Print string literal */
    /* Parse until closing quote and print immediately */
    
    const char *input = ctx->parser->current;
    if (!input) {
        set_error_simple(ctx, RFORTH_ERROR_UNTERMINATED_STRING, ".\" requires string");
        return;
    }
    
    /* Skip whitespace */
    while (*input && isspace(*input)) input++;
    
    const char *start = input;
    const char *end = strchr(start, '"');
    if (!end) {
        set_error_simple(ctx, RFORTH_ERROR_UNTERMINATED_STRING, "Unterminated string literal");
        return;
    }
    
    /* Print the string */
    printf("%.*s", (int)(end - start), start);
    
    /* Advance input pointer past closing quote */
    ctx->parser->current = end + 1;
}

static void builtin_type(rforth_ctx_t *ctx) {
    /* TYPE - Print string from address and length ( addr len -- ) */
    cell_t len, addr;
    if (!stack_pop(ctx->data_stack, &len) || !stack_pop(ctx->data_stack, &addr)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "TYPE requires address and length");
        return;
    }
    
    if (len.type != CELL_INT || addr.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "TYPE requires integer operands");
        return;
    }
    
    if (len.value.i < 0) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "TYPE length cannot be negative");
        return;
    }
    
    const char *str = (const char *)(uintptr_t)addr.value.i;
    if (!str && len.value.i > 0) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "TYPE invalid string address");
        return;
    }
    
    /* Print the string */
    printf("%.*s", (int)len.value.i, str);
}

static void builtin_count(rforth_ctx_t *ctx) {
    /* COUNT - Convert counted string to address/length ( caddr -- addr len ) */
    /* Counted string: first byte is length, followed by string data */
    cell_t caddr;
    if (!stack_pop(ctx->data_stack, &caddr)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "COUNT requires counted string address");
        return;
    }
    
    if (caddr.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "COUNT requires integer address");
        return;
    }
    
    const unsigned char *cstr = (const unsigned char *)(uintptr_t)caddr.value.i;
    if (!cstr) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "COUNT invalid address");
        return;
    }
    
    /* First byte is the length */
    unsigned char len = cstr[0];
    
    /* Address of string data (skip length byte) */
    const char *str_addr = (const char *)(cstr + 1);
    
    /* Push string address and length */
    stack_push_int(ctx->data_stack, (long)str_addr);
    stack_push_int(ctx->data_stack, (long)len);
}

/* Memory operations */
static void builtin_fetch(rforth_ctx_t *ctx) {
    /* @ - Fetch value from address */
    cell_t addr;
    if (!stack_pop(ctx->data_stack, &addr)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "@ requires address on stack");
        return;
    }
    
    /* For simplicity, treat address as variable pointer */
    variable_entry_t *var = (variable_entry_t*)(uintptr_t)addr.value.i;
    if (!var) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "Invalid variable address");
        return;
    }
    
    /* Push variable value */
    stack_push_cell(ctx->data_stack, var->value);
}

static void builtin_store(rforth_ctx_t *ctx) {
    /* ! - Store value to address */
    cell_t addr, value;
    if (!stack_pop(ctx->data_stack, &addr) || !stack_pop(ctx->data_stack, &value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "! requires value and address on stack");
        return;
    }
    
    /* Store value in variable */
    variable_entry_t *var = (variable_entry_t*)(uintptr_t)addr.value.i;
    if (!var) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "Invalid variable address");
        return;
    }
    
    var->value = value;
}

static void builtin_plus_store(rforth_ctx_t *ctx) {
    /* +! - Add value to memory location */
    cell_t addr, value;
    if (!stack_pop(ctx->data_stack, &addr) || !stack_pop(ctx->data_stack, &value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "+! requires value and address on stack");
        return;
    }
    
    /* Add value to variable */
    variable_entry_t *var = (variable_entry_t*)(uintptr_t)addr.value.i;
    if (!var) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "Invalid variable address");
        return;
    }
    
    /* Perform mixed-type addition */
    if (var->value.type == CELL_INT && value.type == CELL_INT) {
        var->value.value.i += value.value.i;
    } else {
        double a = (var->value.type == CELL_INT) ? (double)var->value.value.i : var->value.value.f;
        double b = (value.type == CELL_INT) ? (double)value.value.i : value.value.f;
        var->value = cell_make_float(a + b);
    }
}

static void builtin_question(rforth_ctx_t *ctx) {
    /* ? - Fetch and print value from address */
    cell_t addr;
    if (!stack_pop(ctx->data_stack, &addr)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "? requires address on stack");
        return;
    }
    
    /* Fetch and print variable value */
    variable_entry_t *var = (variable_entry_t*)(uintptr_t)addr.value.i;
    if (!var) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "Invalid variable address");
        return;
    }
    
    /* Print the value */
    if (var->value.type == CELL_INT) {
        printf("%ld ", var->value.value.i);
    } else {
        printf("%g ", var->value.value.f);
    }
}

static void builtin_constant(rforth_ctx_t *ctx) {
    /* CONSTANT - Create a named constant (stack-based for now) */
    cell_t value, name_cell;
    if (!stack_pop(ctx->data_stack, &name_cell) || !stack_pop(ctx->data_stack, &value)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "CONSTANT requires value and name on stack");
        return;
    }
    
    /* Create constant (like variable but initialized) */
    variable_entry_t *var = malloc(sizeof(variable_entry_t));
    if (!var) {
        set_error_simple(ctx, RFORTH_ERROR_MEMORY, "Failed to allocate constant");
        return;
    }
    
    snprintf(var->name, sizeof(var->name), "const_%ld", name_cell.value.i);
    var->value = value;
    var->next = ctx->variables;
    ctx->variables = var;
    
    /* Push constant address onto stack for access */
    uintptr_t addr = (uintptr_t)var;
    stack_push_int(ctx->data_stack, (int64_t)addr);
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
    {"abs", builtin_abs},
    
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
    {"<>", builtin_not_equals},
    {">=", builtin_greater_equals},
    {"<=", builtin_less_equals},
    
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
    {"begin", builtin_begin},
    {"until", builtin_until},
    {"while", builtin_while},
    {"repeat", builtin_repeat},
    
    /* Variables */
    {"variable", builtin_variable},
    {"constant", builtin_constant},
    
    /* Memory operations */
    {"@", builtin_fetch},
    {"!", builtin_store},
    {"+!", builtin_plus_store},
    {"?", builtin_question},
    
    /* Return stack */
    {">r", builtin_to_r},
    {"r>", builtin_from_r},
    {"r@", builtin_r_fetch},
    
    /* Advanced stack operations */
    {"pick", builtin_pick},
    {"roll", builtin_roll},
    {"depth", builtin_depth},
    {"2dup", builtin_two_dup},
    {"2drop", builtin_two_drop},
    {"2swap", builtin_two_swap},
    
    /* Extended arithmetic */
    {"1+", builtin_one_plus},
    {"1-", builtin_one_minus},
    {"2*", builtin_two_star},
    {"2/", builtin_two_slash},
    {"/mod", builtin_slash_mod},
    {"?dup", builtin_question_dup},
    {"0=", builtin_zero_equals},
    {"0<", builtin_zero_less},
    {"0>", builtin_zero_greater},
    {"s\"", builtin_s_quote},
    {".\"", builtin_dot_quote},
    {"type", builtin_type},
    {"count", builtin_count},
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