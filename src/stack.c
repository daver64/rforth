#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/* Cell creation utilities */
cell_t cell_make_int(int64_t value) {
    cell_t cell;
    cell.type = CELL_INT;
    cell.value.i = value;
    return cell;
}

cell_t cell_make_float(double value) {
    cell_t cell;
    cell.type = CELL_FLOAT;
    cell.value.f = value;
    return cell;
}

bool cell_is_int(const cell_t *cell) {
    return cell && cell->type == CELL_INT;
}

bool cell_is_float(const cell_t *cell) {
    return cell && cell->type == CELL_FLOAT;
}

int64_t cell_get_int(const cell_t *cell) {
    return cell ? cell->value.i : 0;
}

double cell_get_float(const cell_t *cell) {
    return cell ? cell->value.f : 0.0;
}

double cell_to_float(const cell_t *cell) {
    if (!cell) return 0.0;
    if (cell->type == CELL_FLOAT) {
        return cell->value.f;
    } else {
        return (double)cell->value.i;
    }
}

rforth_stack_t* stack_create(int size) {
    rforth_stack_t *stack = malloc(sizeof(rforth_stack_t));
    if (!stack) return NULL;
    
    stack->data = malloc(sizeof(cell_t) * size);
    if (!stack->data) {
        free(stack);
        return NULL;
    }
    
    stack->sp = -1;  /* Empty stack */
    stack->size = size;
    return stack;
}

void stack_destroy(rforth_stack_t *stack) {
    if (stack) {
        free(stack->data);
        free(stack);
    }
}

bool stack_push_int(rforth_stack_t *stack, int64_t value) {
    return stack_push_cell(stack, cell_make_int(value));
}

bool stack_push_float(rforth_stack_t *stack, double value) {
    return stack_push_cell(stack, cell_make_float(value));
}

bool stack_push_cell(rforth_stack_t *stack, cell_t cell) {
    if (!stack || stack_is_full(stack)) {
        return false;
    }
    
    stack->data[++stack->sp] = cell;
    return true;
}

bool stack_pop(rforth_stack_t *stack, cell_t *value) {
    if (!stack || stack_is_empty(stack)) {
        return false;
    }
    
    if (value) {
        *value = stack->data[stack->sp];
    }
    stack->sp--;
    return true;
}

bool stack_pop_int(rforth_stack_t *stack, int64_t *value) {
    cell_t cell;
    if (!stack_pop(stack, &cell)) {
        return false;
    }
    
    if (value) {
        if (cell.type == CELL_INT) {
            *value = cell.value.i;
        } else {
            *value = (int64_t)cell.value.f;  /* Convert float to int */
        }
    }
    return true;
}

bool stack_pop_float(rforth_stack_t *stack, double *value) {
    cell_t cell;
    if (!stack_pop(stack, &cell)) {
        return false;
    }
    
    if (value) {
        *value = cell_to_float(&cell);
    }
    return true;
}

bool stack_peek(rforth_stack_t *stack, cell_t *value) {
    if (!stack || stack_is_empty(stack)) {
        return false;
    }
    
    if (value) {
        *value = stack->data[stack->sp];
    }
    return true;
}

bool stack_pick(rforth_stack_t *stack, int index, cell_t *value) {
    if (!stack || index < 0) {
        return false;
    }
    
    int pos = stack->sp - index;
    if (pos < 0) {
        return false;  /* Index beyond stack depth */
    }
    
    if (value) {
        *value = stack->data[pos];
    }
    return true;
}

bool stack_is_empty(rforth_stack_t *stack) {
    return !stack || stack->sp < 0;
}

bool stack_is_full(rforth_stack_t *stack) {
    return !stack || stack->sp >= stack->size - 1;
}

int stack_depth(rforth_stack_t *stack) {
    return stack ? stack->sp + 1 : 0;
}

void stack_clear(rforth_stack_t *stack) {
    if (stack) {
        stack->sp = -1;
    }
}

/* Stack manipulation operations */

bool stack_dup(rforth_stack_t *stack) {
    if (!stack || stack_is_empty(stack) || stack_is_full(stack)) {
        return false;
    }
    
    cell_t top = stack->data[stack->sp];
    return stack_push_cell(stack, top);
}

bool stack_drop(rforth_stack_t *stack) {
    return stack_pop(stack, NULL);
}

bool stack_swap(rforth_stack_t *stack) {
    if (!stack || stack_depth(stack) < 2) {
        return false;
    }
    
    cell_t top = stack->data[stack->sp];
    cell_t second = stack->data[stack->sp - 1];
    
    stack->data[stack->sp] = second;
    stack->data[stack->sp - 1] = top;
    
    return true;
}

bool stack_over(rforth_stack_t *stack) {
    if (!stack || stack_depth(stack) < 2 || stack_is_full(stack)) {
        return false;
    }
    
    cell_t second = stack->data[stack->sp - 1];
    return stack_push_cell(stack, second);
}

bool stack_rot(rforth_stack_t *stack) {
    if (!stack || stack_depth(stack) < 3) {
        return false;
    }
    
    /* ( a b c -- b c a ) */
    cell_t a = stack->data[stack->sp - 2];
    cell_t b = stack->data[stack->sp - 1];
    cell_t c = stack->data[stack->sp];
    
    stack->data[stack->sp - 2] = b;
    stack->data[stack->sp - 1] = c;
    stack->data[stack->sp] = a;
    
    return true;
}

void stack_print(rforth_stack_t *stack) {
    if (!stack) {
        printf("<null stack>\n");
        return;
    }
    
    if (stack_is_empty(stack)) {
        printf("<empty>\n");
        return;
    }
    
    printf("Stack (%d): ", stack_depth(stack));
    for (int i = 0; i <= stack->sp; i++) {
        if (stack->data[i].type == CELL_INT) {
            printf("%ld ", (long)stack->data[i].value.i);
        } else {
            printf("%.6g ", stack->data[i].value.f);
        }
    }
    printf("\n");
}