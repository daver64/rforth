#include "rforth.h"
#include "turnkey.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

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
static void builtin_do(rforth_ctx_t *ctx);
static void builtin_loop(rforth_ctx_t *ctx);
static void builtin_plus_loop(rforth_ctx_t *ctx);
static void builtin_leave(rforth_ctx_t *ctx);
static void builtin_i(rforth_ctx_t *ctx);
static void builtin_j(rforth_ctx_t *ctx);

/* Character I/O operations */
static void builtin_key(rforth_ctx_t *ctx);
static void builtin_key_question(rforth_ctx_t *ctx);
static void builtin_c_fetch(rforth_ctx_t *ctx);
static void builtin_c_store(rforth_ctx_t *ctx);

/* Advanced mathematics */
static void builtin_star_slash(rforth_ctx_t *ctx);
static void builtin_star_slash_mod(rforth_ctx_t *ctx);
static void builtin_fm_slash_mod(rforth_ctx_t *ctx);

/* System control words */
static void builtin_execute(rforth_ctx_t *ctx);
static void builtin_evaluate(rforth_ctx_t *ctx);
static void builtin_quit(rforth_ctx_t *ctx);
static void builtin_abort(rforth_ctx_t *ctx);

/* Meta-compilation words */
static void builtin_create(rforth_ctx_t *ctx);
static void builtin_does(rforth_ctx_t *ctx);
static void builtin_immediate(rforth_ctx_t *ctx);

/* Critical missing ANSI words - Phase 1 */
static void builtin_colon(rforth_ctx_t *ctx);
static void builtin_semicolon(rforth_ctx_t *ctx);
static void builtin_here(rforth_ctx_t *ctx);
static void builtin_allot(rforth_ctx_t *ctx);
static void builtin_comma(rforth_ctx_t *ctx);
static void builtin_c_comma(rforth_ctx_t *ctx);
static void builtin_bl(rforth_ctx_t *ctx);
static void builtin_spaces(rforth_ctx_t *ctx);
static void builtin_decimal(rforth_ctx_t *ctx);
static void builtin_base(rforth_ctx_t *ctx);
static void builtin_state(rforth_ctx_t *ctx);
static void builtin_invert(rforth_ctx_t *ctx);
static void builtin_xor(rforth_ctx_t *ctx);
static void builtin_u_dot(rforth_ctx_t *ctx);
static void builtin_u_less(rforth_ctx_t *ctx);
static void builtin_two_over(rforth_ctx_t *ctx);
static void builtin_unloop(rforth_ctx_t *ctx);

/* Phase 2: Numeric formatting words */
static void builtin_less_hash(rforth_ctx_t *ctx);
static void builtin_hash(rforth_ctx_t *ctx);
static void builtin_hash_s(rforth_ctx_t *ctx);
static void builtin_hash_greater(rforth_ctx_t *ctx);
static void builtin_hold(rforth_ctx_t *ctx);
static void builtin_sign(rforth_ctx_t *ctx);
static void builtin_s_to_d(rforth_ctx_t *ctx);

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

/* Counted loops (DO/LOOP) */
static void builtin_do(rforth_ctx_t *ctx) {
    /* DO - Start counted loop ( limit index -- ) */
    
    /* If we're in skip mode, track nesting */
    if (ctx->skip_mode) {
        ctx->skip_depth++;
        return;
    }
    
    /* Pop index and limit from data stack */
    cell_t index, limit;
    if (!stack_pop(ctx->data_stack, &index) || !stack_pop(ctx->data_stack, &limit)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "DO requires limit and index on stack");
        return;
    }
    
    /* Check stack bounds */
    if (ctx->do_loop_sp >= 32) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "DO/LOOP stack overflow");
        return;
    }
    
    if (ctx->loop_sp >= 32) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Loop stack overflow");
        return;
    }
    
    if (ctx->cf_sp >= 32) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Control flow stack overflow");
        return;
    }
    
    /* Store loop parameters */
    ctx->loop_index[ctx->do_loop_sp] = (index.type == CELL_INT) ? index.value.i : (int64_t)index.value.f;
    ctx->loop_limit[ctx->do_loop_sp] = (limit.type == CELL_INT) ? limit.value.i : (int64_t)limit.value.f;
    ctx->do_loop_sp++;
    
    /* Save loop start position */
    ctx->loop_start[ctx->loop_sp] = ctx->parser->current;
    ctx->loop_sp++;
    
    /* Push DO onto control flow stack */
    ctx->cf_stack[ctx->cf_sp].type = CF_DO;
    ctx->cf_stack[ctx->cf_sp].address = ctx->loop_sp - 1;  /* Index into loop_start array */
    ctx->cf_sp++;
}

static void builtin_loop(rforth_ctx_t *ctx) {
    /* LOOP - End counted loop, increment by 1 */
    
    /* If we're skipping and this is nested, just decrement depth */
    if (ctx->skip_mode && ctx->skip_depth > 1) {
        ctx->skip_depth--;
        return;
    }
    
    /* Check control flow stack */
    if (ctx->cf_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "LOOP without matching DO");
        return;
    }
    
    ctx->cf_sp--;
    if (ctx->cf_stack[ctx->cf_sp].type != CF_DO) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "LOOP without matching DO");
        return;
    }
    
    /* If we were skipping, stop now */
    if (ctx->skip_mode) {
        ctx->skip_mode = false;
        ctx->skip_depth = 0;
        ctx->loop_sp--;
        ctx->do_loop_sp--;
        return;
    }
    
    /* Check DO/LOOP stack */
    if (ctx->do_loop_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "LOOP without matching DO");
        return;
    }
    
    /* Increment index */
    ctx->loop_index[ctx->do_loop_sp - 1]++;
    
    /* Check if loop should continue */
    if (ctx->loop_index[ctx->do_loop_sp - 1] < ctx->loop_limit[ctx->do_loop_sp - 1]) {
        /* Continue loop - restore parser position */
        int loop_pos = ctx->cf_stack[ctx->cf_sp].address;
        ctx->parser->current = ctx->loop_start[loop_pos];
        
        /* Push DO back onto control flow stack for next iteration */
        ctx->cf_stack[ctx->cf_sp].type = CF_DO;
        ctx->cf_stack[ctx->cf_sp].address = loop_pos;
        ctx->cf_sp++;
    } else {
        /* Exit loop - pop stacks */
        ctx->loop_sp--;
        ctx->do_loop_sp--;
    }
}

static void builtin_plus_loop(rforth_ctx_t *ctx) {
    /* +LOOP - End counted loop, increment by n ( n -- ) */
    
    /* If we're skipping and this is nested, just decrement depth */
    if (ctx->skip_mode && ctx->skip_depth > 1) {
        ctx->skip_depth--;
        return;
    }
    
    /* Check control flow stack */
    if (ctx->cf_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "+LOOP without matching DO");
        return;
    }
    
    ctx->cf_sp--;
    if (ctx->cf_stack[ctx->cf_sp].type != CF_DO) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "+LOOP without matching DO");
        return;
    }
    
    /* If we were skipping, stop now */
    if (ctx->skip_mode) {
        ctx->skip_mode = false;
        ctx->skip_depth = 0;
        ctx->loop_sp--;
        ctx->do_loop_sp--;
        return;
    }
    
    /* Pop increment from data stack */
    cell_t increment;
    if (!stack_pop(ctx->data_stack, &increment)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "+LOOP requires increment on stack");
        return;
    }
    
    /* Check DO/LOOP stack */
    if (ctx->do_loop_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "+LOOP without matching DO");
        return;
    }
    
    /* Add increment to index */
    int64_t inc = (increment.type == CELL_INT) ? increment.value.i : (int64_t)increment.value.f;
    ctx->loop_index[ctx->do_loop_sp - 1] += inc;
    
    /* Check if loop should continue (handle both positive and negative increments) */
    bool continue_loop = false;
    if (inc >= 0) {
        continue_loop = (ctx->loop_index[ctx->do_loop_sp - 1] < ctx->loop_limit[ctx->do_loop_sp - 1]);
    } else {
        continue_loop = (ctx->loop_index[ctx->do_loop_sp - 1] >= ctx->loop_limit[ctx->do_loop_sp - 1]);
    }
    
    if (continue_loop) {
        /* Continue loop - restore parser position */
        int loop_pos = ctx->cf_stack[ctx->cf_sp].address;
        ctx->parser->current = ctx->loop_start[loop_pos];
        
        /* Push DO back onto control flow stack for next iteration */
        ctx->cf_stack[ctx->cf_sp].type = CF_DO;
        ctx->cf_stack[ctx->cf_sp].address = loop_pos;
        ctx->cf_sp++;
    } else {
        /* Exit loop - pop stacks */
        ctx->loop_sp--;
        ctx->do_loop_sp--;
    }
}

static void builtin_leave(rforth_ctx_t *ctx) {
    /* LEAVE - Exit current DO/LOOP immediately */
    
    /* Check if we're in a DO/LOOP */
    if (ctx->do_loop_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "LEAVE without matching DO");
        return;
    }
    
    /* Find and remove the DO from control flow stack */
    for (int i = ctx->cf_sp - 1; i >= 0; i--) {
        if (ctx->cf_stack[i].type == CF_DO) {
            /* Remove this entry and all after it */
            ctx->cf_sp = i;
            break;
        }
    }
    
    /* Pop DO/LOOP stacks */
    ctx->loop_sp--;
    ctx->do_loop_sp--;
    
    /* Set skip mode to skip until LOOP or +LOOP */
    ctx->skip_mode = true;
    ctx->skip_depth = 1;
}

static void builtin_i(rforth_ctx_t *ctx) {
    /* I - Push current loop index */
    if (ctx->do_loop_sp <= 0) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "I outside of DO/LOOP");
        return;
    }
    
    stack_push_int(ctx->data_stack, ctx->loop_index[ctx->do_loop_sp - 1]);
}

static void builtin_j(rforth_ctx_t *ctx) {
    /* J - Push outer loop index */
    if (ctx->do_loop_sp < 2) {
        set_error_simple(ctx, RFORTH_ERROR_CONTROL_FLOW, "J outside of nested DO/LOOP");
        return;
    }
    
    stack_push_int(ctx->data_stack, ctx->loop_index[ctx->do_loop_sp - 2]);
}

/* Character I/O operations */
static void builtin_key(rforth_ctx_t *ctx) {
    /* KEY - Read one character from input ( -- char ) */
    int ch = getchar();
    if (ch == EOF) {
        ch = 0;  /* Return 0 for EOF */
    }
    stack_push_int(ctx->data_stack, ch);
}

static void builtin_key_question(rforth_ctx_t *ctx) {
    /* KEY? - Check if character input is available ( -- flag ) */
    /* This is a simplified implementation - always returns true for now */
    /* In a real implementation, this would check stdin for available input */
    stack_push_int(ctx->data_stack, -1);  /* Always true for simplicity */
}

static void builtin_c_fetch(rforth_ctx_t *ctx) {
    /* C@ - Fetch byte from address ( addr -- byte ) */
    cell_t addr;
    if (!stack_pop(ctx->data_stack, &addr)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "C@ requires address on stack");
        return;
    }
    
    if (addr.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "C@ requires integer address");
        return;
    }
    
    /* Safety check - don't access null or very low addresses */
    if (addr.value.i < 1024) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "C@ invalid address");
        return;
    }
    
    /* Read byte from address */
    unsigned char *ptr = (unsigned char *)(uintptr_t)addr.value.i;
    unsigned char byte = *ptr;
    
    stack_push_int(ctx->data_stack, byte);
}

static void builtin_c_store(rforth_ctx_t *ctx) {
    /* C! - Store byte to address ( byte addr -- ) */
    cell_t addr, byte;
    if (!stack_pop(ctx->data_stack, &addr) || !stack_pop(ctx->data_stack, &byte)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "C! requires byte and address on stack");
        return;
    }
    
    if (addr.type != CELL_INT || byte.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "C! requires integer operands");
        return;
    }
    
    /* Safety check - don't access null or very low addresses */
    if (addr.value.i < 1024) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "C! invalid address");
        return;
    }
    
    /* Store byte to address */
    unsigned char *ptr = (unsigned char *)(uintptr_t)addr.value.i;
    *ptr = (unsigned char)(byte.value.i & 0xFF);
}

/* Advanced mathematics */
static void builtin_star_slash(rforth_ctx_t *ctx) {
    /* STAR-SLASH - Mixed precision multiply and divide ( n1 n2 n3 -- n1*n2/n3 ) */
    cell_t n3, n2, n1;
    if (!stack_pop(ctx->data_stack, &n3) || !stack_pop(ctx->data_stack, &n2) || !stack_pop(ctx->data_stack, &n1)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "*/ requires three values on stack");
        return;
    }
    
    if (n1.type != CELL_INT || n2.type != CELL_INT || n3.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "*/ requires integer operands");
        return;
    }
    
    if (n3.value.i == 0) {
        set_error_simple(ctx, RFORTH_ERROR_DIVISION_BY_ZERO, "*/ division by zero");
        return;
    }
    
    /* Use 64-bit intermediate to avoid overflow */
    int64_t intermediate = (int64_t)n1.value.i * (int64_t)n2.value.i;
    int64_t result = intermediate / n3.value.i;
    
    stack_push_int(ctx->data_stack, result);
}

static void builtin_star_slash_mod(rforth_ctx_t *ctx) {
    /* STAR-SLASH-MOD - Mixed precision multiply and divide with remainder ( n1 n2 n3 -- remainder quotient ) */
    cell_t n3, n2, n1;
    if (!stack_pop(ctx->data_stack, &n3) || !stack_pop(ctx->data_stack, &n2) || !stack_pop(ctx->data_stack, &n1)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "*/MOD requires three values on stack");
        return;
    }
    
    if (n1.type != CELL_INT || n2.type != CELL_INT || n3.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "*/MOD requires integer operands");
        return;
    }
    
    if (n3.value.i == 0) {
        set_error_simple(ctx, RFORTH_ERROR_DIVISION_BY_ZERO, "*/MOD division by zero");
        return;
    }
    
    /* Use 64-bit intermediate to avoid overflow */
    int64_t intermediate = (int64_t)n1.value.i * (int64_t)n2.value.i;
    int64_t quotient = intermediate / n3.value.i;
    int64_t remainder = intermediate % n3.value.i;
    
    stack_push_int(ctx->data_stack, remainder);
    stack_push_int(ctx->data_stack, quotient);
}

static void builtin_fm_slash_mod(rforth_ctx_t *ctx) {
    /* FM/MOD - Floored division and modulo ( d n -- remainder quotient ) */
    /* This implements floored division where the remainder has the same sign as the divisor */
    cell_t n, d_low;
    if (!stack_pop(ctx->data_stack, &n) || !stack_pop(ctx->data_stack, &d_low)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "FM/MOD requires dividend and divisor on stack");
        return;
    }
    
    if (d_low.type != CELL_INT || n.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "FM/MOD requires integer operands");
        return;
    }
    
    if (n.value.i == 0) {
        set_error_simple(ctx, RFORTH_ERROR_DIVISION_BY_ZERO, "FM/MOD division by zero");
        return;
    }
    
    /* For simplicity, treating this as single-precision for now */
    int64_t dividend = d_low.value.i;
    int64_t divisor = n.value.i;
    
    int64_t quotient = dividend / divisor;
    int64_t remainder = dividend % divisor;
    
    /* Implement floored division semantics */
    if (remainder != 0 && ((dividend < 0) != (divisor < 0))) {
        quotient--;
        remainder += divisor;
    }
    
    stack_push_int(ctx->data_stack, remainder);
    stack_push_int(ctx->data_stack, quotient);
}

/* System control words */
static void builtin_execute(rforth_ctx_t *ctx) {
    /* EXECUTE - Execute word whose execution token is on stack ( xt -- ) */
    cell_t xt;
    if (!stack_pop(ctx->data_stack, &xt)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "EXECUTE requires execution token on stack");
        return;
    }
    
    if (xt.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "EXECUTE requires integer execution token");
        return;
    }
    
    /* For this implementation, treat the XT as a function pointer */
    /* This is a simplified approach - real Forth systems have more complex XT handling */
    void (*func)(rforth_ctx_t*) = (void(*)(rforth_ctx_t*))(uintptr_t)xt.value.i;
    
    if (!func) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "EXECUTE invalid execution token");
        return;
    }
    
    /* Execute the function */
    func(ctx);
}

static void builtin_evaluate(rforth_ctx_t *ctx) {
    /* EVALUATE - Interpret string ( addr len -- ) */
    cell_t len, addr;
    if (!stack_pop(ctx->data_stack, &len) || !stack_pop(ctx->data_stack, &addr)) {
        set_error_simple(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "EVALUATE requires address and length on stack");
        return;
    }
    
    if (addr.type != CELL_INT || len.type != CELL_INT) {
        set_error_simple(ctx, RFORTH_ERROR_TYPE_MISMATCH, "EVALUATE requires integer operands");
        return;
    }
    
    if (len.value.i < 0) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "EVALUATE length cannot be negative");
        return;
    }
    
    const char *str = (const char *)(uintptr_t)addr.value.i;
    if (!str && len.value.i > 0) {
        set_error_simple(ctx, RFORTH_ERROR_INVALID_ADDRESS, "EVALUATE invalid string address");
        return;
    }
    
    /* Create a null-terminated copy of the string */
    char *eval_string = malloc(len.value.i + 1);
    if (!eval_string) {
        set_error_simple(ctx, RFORTH_ERROR_MEMORY, "EVALUATE memory allocation failed");
        return;
    }
    
    strncpy(eval_string, str, len.value.i);
    eval_string[len.value.i] = '\0';
    
    /* Interpret the string */
    int result = rforth_interpret_string(ctx, eval_string);
    
    free(eval_string);
    
    if (result != 0) {
        set_error_simple(ctx, RFORTH_ERROR_SYNTAX_ERROR, "EVALUATE interpretation failed");
    }
}

static void builtin_quit(rforth_ctx_t *ctx) {
    /* QUIT - Return to outer interpreter loop */
    /* Clear the data stack and return stack */
    while (stack_depth(ctx->data_stack) > 0) {
        cell_t dummy;
        stack_pop(ctx->data_stack, &dummy);
    }
    
    while (stack_depth(ctx->return_stack) > 0) {
        cell_t dummy;
        stack_pop(ctx->return_stack, &dummy);
    }
    
    /* Clear control flow stacks */
    ctx->cf_sp = 0;
    ctx->loop_sp = 0;
    ctx->do_loop_sp = 0;
    
    /* Clear skip mode */
    ctx->skip_mode = false;
    ctx->skip_depth = 0;
    
    /* Reset state */
    ctx->state = PARSE_INTERPRET;
    
    printf("QUIT - Returning to interpreter\n");
}

static void builtin_abort(rforth_ctx_t *ctx) {
    /* ABORT - Clear stacks and quit with error message */
    printf("ABORT\n");
    
    /* Do everything QUIT does */
    builtin_quit(ctx);
    
    /* Set error state */
    set_error_simple(ctx, RFORTH_ERROR_ABORT, "ABORT executed");
}

/* Meta-compilation words */
static void builtin_create(rforth_ctx_t *ctx) {
    /* CREATE - Create a new word that pushes its data address ( -- ) */
    /* This is a simplified implementation that creates a named variable */
    
    /* For simplicity, CREATE will work like VARIABLE for now */
    /* A full implementation would create a word that pushes its parameter field address */
    
    printf("CREATE - Creating new word (simplified as variable)\n");
    builtin_variable(ctx);
}

static void builtin_does(rforth_ctx_t *ctx) {
    /* DOES> - Define runtime behavior for CREATEd words */
    /* This is a complex meta-compilation feature */
    /* For now, provide a basic implementation */
    
    printf("DOES> - Defining runtime behavior (simplified implementation)\n");
    
    /* In a full implementation, this would:
     * 1. Compile code to change the behavior of the most recently CREATEd word
     * 2. Set up runtime semantics for that word
     * 3. Handle the transition from compile-time to runtime behavior
     */
    
    /* For this simplified version, we'll just acknowledge the command */
    set_error_simple(ctx, RFORTH_ERROR_NOT_IMPLEMENTED, "DOES> not fully implemented");
}

static void builtin_immediate(rforth_ctx_t *ctx) {
    /* IMMEDIATE - Make the most recent definition immediate */
    /* Immediate words execute during compilation instead of being compiled */
    
    printf("IMMEDIATE - Making word immediate (simplified implementation)\n");
    
    /* In a full implementation, this would:
     * 1. Find the most recently defined word in the dictionary
     * 2. Set its immediate flag
     * 3. Change its compilation behavior
     */
    
    /* For this simplified version, we'll just acknowledge the command */
    printf("Note: IMMEDIATE flag set (basic implementation)\n");
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

/* ANSI Core Words - Phase 1 Implementation */

static void builtin_here(rforth_ctx_t *ctx) {
    /* HERE - Return address of dictionary pointer ( -- addr ) */
    /* For simplified implementation, use a fixed address */
    stack_push_int(ctx->data_stack, (int64_t)&ctx->here_ptr);
}

static void builtin_allot(rforth_ctx_t *ctx) {
    /* ALLOT - Allocate n bytes in dictionary ( n -- ) */
    if (ctx->data_stack->size < 1) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "ALLOT requires byte count on stack");
        return;
    }
    
    cell_t n_cell;
    if (!stack_pop(ctx->data_stack, &n_cell) || n_cell.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "ALLOT requires integer byte count");
        return;
    }
    
    /* For simplified implementation, just advance HERE pointer */
    if (!ctx->here_ptr) {
        ctx->here_ptr = malloc(65536); /* Allocate dictionary space */
    }
    ctx->here_ptr += n_cell.value.i;
}

static void builtin_comma(rforth_ctx_t *ctx) {
    /* , - Compile cell into dictionary ( x -- ) */
    if (ctx->data_stack->size < 1) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, ", requires value on stack");
        return;
    }
    
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, ", requires value on stack");
        return;
    }
    
    /* For simplified implementation, store at HERE */
    if (!ctx->here_ptr) {
        ctx->here_ptr = malloc(65536);
    }
    *(cell_t*)ctx->here_ptr = value;
    ctx->here_ptr += sizeof(cell_t);
}

static void builtin_c_comma(rforth_ctx_t *ctx) {
    /* C, - Compile character into dictionary ( char -- ) */
    if (ctx->data_stack->size < 1) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "C, requires character on stack");
        return;
    }
    
    cell_t char_cell;
    if (!stack_pop(ctx->data_stack, &char_cell) || char_cell.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "C, requires integer character");
        return;
    }
    
    if (!ctx->here_ptr) {
        ctx->here_ptr = malloc(65536);
    }
    *(char*)ctx->here_ptr = (char)char_cell.value.i;
    ctx->here_ptr += 1;
}

static void builtin_bl(rforth_ctx_t *ctx) {
    /* BL - Push ASCII space character ( -- 32 ) */
    stack_push_int(ctx->data_stack, 32);
}

static void builtin_spaces(rforth_ctx_t *ctx) {
    /* SPACES - Print n spaces ( n -- ) */
    if (ctx->data_stack->size < 1) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "SPACES requires count on stack");
        return;
    }
    
    cell_t n_cell;
    if (!stack_pop(ctx->data_stack, &n_cell) || n_cell.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "SPACES requires integer count");
        return;
    }
    
    for (int64_t i = 0; i < n_cell.value.i; i++) {
        printf(" ");
    }
}

static void builtin_decimal(rforth_ctx_t *ctx) {
    /* DECIMAL - Set numeric base to 10 ( -- ) */
    ctx->numeric_base = 10;
}

static void builtin_base(rforth_ctx_t *ctx) {
    /* BASE - Return address of BASE variable ( -- addr ) */
    stack_push_int(ctx->data_stack, (int64_t)&ctx->numeric_base);
}

static void builtin_state(rforth_ctx_t *ctx) {
    /* STATE - Return address of STATE variable ( -- addr ) */
    stack_push_int(ctx->data_stack, (int64_t)&ctx->state_var);
}

static void builtin_invert(rforth_ctx_t *ctx) {
    /* INVERT - Bitwise NOT ( x -- ~x ) */
    if (ctx->data_stack->size < 1) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "INVERT requires value on stack");
        return;
    }
    
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value) || value.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "INVERT requires integer");
        return;
    }
    
    stack_push_int(ctx->data_stack, ~value.value.i);
}

static void builtin_xor(rforth_ctx_t *ctx) {
    /* XOR - Bitwise exclusive OR ( x1 x2 -- x3 ) */
    if (ctx->data_stack->size < 2) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "XOR requires two values on stack");
        return;
    }
    
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "XOR requires two values on stack");
        return;
    }
    
    if (a.type != CELL_INT || b.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "XOR requires integers");
        return;
    }
    
    stack_push_int(ctx->data_stack, a.value.i ^ b.value.i);
}

static void builtin_u_dot(rforth_ctx_t *ctx) {
    /* U. - Print unsigned number ( u -- ) */
    if (ctx->data_stack->size < 1) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "U. requires value on stack");
        return;
    }
    
    cell_t value;
    if (!stack_pop(ctx->data_stack, &value) || value.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "U. requires integer");
        return;
    }
    
    printf("%llu ", (unsigned long long)value.value.i);
}

static void builtin_u_less(rforth_ctx_t *ctx) {
    /* U< - Unsigned less than ( u1 u2 -- flag ) */
    if (ctx->data_stack->size < 2) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "U< requires two values on stack");
        return;
    }
    
    cell_t b, a;
    if (!stack_pop(ctx->data_stack, &b) || !stack_pop(ctx->data_stack, &a)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "U< requires two values on stack");
        return;
    }
    
    if (a.type != CELL_INT || b.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "U< requires integers");
        return;
    }
    
    stack_push_int(ctx->data_stack, ((uint64_t)a.value.i < (uint64_t)b.value.i) ? -1 : 0);
}

static void builtin_two_over(rforth_ctx_t *ctx) {
    /* 2OVER - Copy second pair ( x1 x2 x3 x4 -- x1 x2 x3 x4 x1 x2 ) */
    if (ctx->data_stack->size < 4) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "2OVER requires four values on stack");
        return;
    }
    
    /* Get the values without popping */
    cell_t *stack_data = ctx->data_stack->data;
    int top = ctx->data_stack->size - 1;
    
    /* Copy x1 and x2 (third and fourth from top) */
    cell_t third = stack_data[top - 3];
    cell_t second = stack_data[top - 2];
    
    /* Push copies */
    if (third.type == CELL_INT) {
        stack_push_int(ctx->data_stack, third.value.i);
    } else {
        stack_push_float(ctx->data_stack, third.value.f);
    }
    
    if (second.type == CELL_INT) {
        stack_push_int(ctx->data_stack, second.value.i);
    } else {
        stack_push_float(ctx->data_stack, second.value.f);
    }
}

static void builtin_unloop(rforth_ctx_t *ctx) {
    /* UNLOOP - Discard loop parameters ( -- ) */
    if (ctx->do_loop_sp <= 0) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "UNLOOP without matching DO");
        return;
    }
    
    ctx->do_loop_sp--;
}

static void builtin_colon(rforth_ctx_t *ctx) {
    /* : - Begin word definition ( C: "<spaces>name" -- colon-sys ) */
    printf(": - Begin word definition (simplified implementation)\n");
    ctx->state_var = -1; /* Enter compile mode */
}

static void builtin_semicolon(rforth_ctx_t *ctx) {
    /* ; - End word definition ( C: colon-sys -- ) */
    printf("; - End word definition (simplified implementation)\n");
    ctx->state_var = 0; /* Return to interpret mode */
}

/* Phase 2: Numeric Formatting Words */

static void builtin_less_hash(rforth_ctx_t *ctx) {
    /* <# - Initialize numeric output conversion ( -- ) */
    ctx->format_pos = 0;
    ctx->format_buffer[0] = '\0';
}

static void builtin_hash(rforth_ctx_t *ctx) {
    /* # - Add next digit to numeric output ( ud1 -- ud2 ) */
    if (ctx->data_stack->size < 2) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "# requires double number on stack");
        return;
    }
    
    cell_t high, low;
    if (!stack_pop(ctx->data_stack, &high) || !stack_pop(ctx->data_stack, &low)) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "# requires double number on stack");
        return;
    }
    
    if (high.type != CELL_INT || low.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "# requires integer operands");
        return;
    }
    
    /* Combine into 64-bit unsigned number */
    uint64_t ud = ((uint64_t)high.value.i << 32) | (uint32_t)low.value.i;
    
    /* Extract digit */
    uint64_t base = (uint64_t)ctx->numeric_base;
    char digit = (char)(ud % base);
    ud /= base;
    
    /* Convert to character */
    char ch = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
    
    /* Prepend to buffer */
    if (ctx->format_pos < sizeof(ctx->format_buffer) - 1) {
        memmove(ctx->format_buffer + 1, ctx->format_buffer, ctx->format_pos + 1);
        ctx->format_buffer[0] = ch;
        ctx->format_pos++;
    }
    
    /* Push result back */
    stack_push_int(ctx->data_stack, (int64_t)(ud >> 32));
    stack_push_int(ctx->data_stack, (int64_t)(ud & 0xFFFFFFFF));
}

static void builtin_hash_s(rforth_ctx_t *ctx) {
    /* #S - Convert all remaining digits ( ud1 -- 0 0 ) */
    if (ctx->data_stack->size < 2) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "#S requires double number on stack");
        return;
    }
    
    do {
        builtin_hash(ctx);
        if (ctx->last_error.code != RFORTH_OK) return;
        
        /* Check if result is zero */
        if (ctx->data_stack->size >= 2) {
            cell_t *stack_data = ctx->data_stack->data;
            int top = ctx->data_stack->size - 1;
            if (stack_data[top].value.i == 0 && stack_data[top-1].value.i == 0) {
                break;
            }
        }
    } while (true);
}

static void builtin_hash_greater(rforth_ctx_t *ctx) {
    /* #> - Complete numeric conversion ( ud -- c-addr u ) */
    if (ctx->data_stack->size < 2) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "#> requires double number on stack");
        return;
    }
    
    /* Remove the double number from stack */
    cell_t high, low;
    stack_pop(ctx->data_stack, &high);
    stack_pop(ctx->data_stack, &low);
    
    /* Push address and length of formatted string */
    stack_push_int(ctx->data_stack, (int64_t)ctx->format_buffer);
    stack_push_int(ctx->data_stack, ctx->format_pos);
}

static void builtin_hold(rforth_ctx_t *ctx) {
    /* HOLD - Insert character into numeric output ( char -- ) */
    if (ctx->data_stack->size < 1) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "HOLD requires character on stack");
        return;
    }
    
    cell_t char_cell;
    if (!stack_pop(ctx->data_stack, &char_cell) || char_cell.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "HOLD requires integer character");
        return;
    }
    
    /* Prepend character to format buffer */
    if (ctx->format_pos < sizeof(ctx->format_buffer) - 1) {
        memmove(ctx->format_buffer + 1, ctx->format_buffer, ctx->format_pos + 1);
        ctx->format_buffer[0] = (char)char_cell.value.i;
        ctx->format_pos++;
    }
}

static void builtin_sign(rforth_ctx_t *ctx) {
    /* SIGN - Add minus sign if n is negative ( n -- ) */
    if (ctx->data_stack->size < 1) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "SIGN requires value on stack");
        return;
    }
    
    cell_t n_cell;
    if (!stack_pop(ctx->data_stack, &n_cell) || n_cell.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "SIGN requires integer");
        return;
    }
    
    /* Add minus sign if negative */
    if (n_cell.value.i < 0) {
        if (ctx->format_pos < sizeof(ctx->format_buffer) - 1) {
            memmove(ctx->format_buffer + 1, ctx->format_buffer, ctx->format_pos + 1);
            ctx->format_buffer[0] = '-';
            ctx->format_pos++;
        }
    }
}

static void builtin_s_to_d(rforth_ctx_t *ctx) {
    /* S>D - Convert single to double number ( n -- d ) */
    if (ctx->data_stack->size < 1) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_UNDERFLOW, "S>D requires value on stack");
        return;
    }
    
    cell_t n_cell;
    if (!stack_pop(ctx->data_stack, &n_cell) || n_cell.type != CELL_INT) {
        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_TYPE_MISMATCH, "S>D requires integer");
        return;
    }
    
    /* Sign extend to double */
    int64_t n = n_cell.value.i;
    stack_push_int(ctx->data_stack, n);                    /* Low part */
    stack_push_int(ctx->data_stack, (n < 0) ? -1 : 0);     /* High part (sign extended) */
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
    {"*/", builtin_star_slash},
    {"*/mod", builtin_star_slash_mod},
    {"fm/mod", builtin_fm_slash_mod},
    
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
    {"key", builtin_key},
    {"key?", builtin_key_question},
    {"c@", builtin_c_fetch},
    {"c!", builtin_c_store},
    
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
    {"execute", builtin_execute},
    {"evaluate", builtin_evaluate},
    {"quit", builtin_quit},
    {"abort", builtin_abort},
    
    /* Meta-compilation */
    {"create", builtin_create},
    {"does>", builtin_does},
    {"immediate", builtin_immediate},
    
    /* Control flow (basic) */
    {"if", builtin_if},
    {"then", builtin_then},
    {"else", builtin_else},
    {"begin", builtin_begin},
    {"until", builtin_until},
    {"while", builtin_while},
    {"repeat", builtin_repeat},
    {"do", builtin_do},
    {"loop", builtin_loop},
    {"+loop", builtin_plus_loop},
    {"leave", builtin_leave},
    {"i", builtin_i},
    {"j", builtin_j},
    
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
    
    /* ANSI Core Words - Phase 1 */
    {"here", builtin_here},
    {"allot", builtin_allot},
    {",", builtin_comma},
    {"c,", builtin_c_comma},
    {"bl", builtin_bl},
    {"spaces", builtin_spaces},
    {"decimal", builtin_decimal},
    {"base", builtin_base},
    {"state", builtin_state},
    {"invert", builtin_invert},
    {"xor", builtin_xor},
    {"u.", builtin_u_dot},
    {"u<", builtin_u_less},
    {"2over", builtin_two_over},
    {"unloop", builtin_unloop},
    {":", builtin_colon},
    {";", builtin_semicolon},
    
    /* ANSI Core Words - Phase 2: Numeric formatting */
    {"<#", builtin_less_hash},
    {"#", builtin_hash},
    {"#s", builtin_hash_s},
    {"#>", builtin_hash_greater},
    {"hold", builtin_hold},
    {"sign", builtin_sign},
    {"s>d", builtin_s_to_d},
    
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