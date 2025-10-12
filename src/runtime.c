#include "rforth.h"

/* Placeholder implementations - to be implemented later */

/* Code generation utilities */

bool codegen_init(void) {
    return true;
}

void codegen_cleanup(void) {
    /* Nothing to cleanup yet */
}

/* Runtime support functions for compiled code */

void rf_init(void) {
    /* Initialize runtime for compiled programs */
}

void rf_cleanup(void) {
    /* Cleanup runtime for compiled programs */
}

/* Runtime stack operations for compiled code */
static cell_t runtime_stack[DEFAULT_STACK_SIZE];
static int runtime_sp = -1;

void rf_push(cell_t value) {
    if (runtime_sp < DEFAULT_STACK_SIZE - 1) {
        runtime_stack[++runtime_sp] = value;
    }
}

cell_t rf_pop(void) {
    if (runtime_sp >= 0) {
        return runtime_stack[runtime_sp--];
    }
    cell_t zero = {0};
    zero.type = CELL_INT;
    zero.value.i = 0;
    return zero;
}

void rf_print_num(cell_t value) {
    if (value.type == CELL_INT) {
        printf("%ld ", (long)value.value.i);
    } else {
        printf("%.6g ", value.value.f);
    }
}

void rf_print_char(cell_t value) {
    int ch;
    if (value.type == CELL_INT) {
        ch = (int)value.value.i;
    } else {
        ch = (int)value.value.f;
    }
    printf("%c", ch);
}

void rf_newline(void) {
    printf("\n");
}