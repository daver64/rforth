#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

stack_t* stack_create(int size) {
    stack_t *stack = malloc(sizeof(stack_t));
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

void stack_destroy(stack_t *stack) {
    if (stack) {
        free(stack->data);
        free(stack);
    }
}

bool stack_push(stack_t *stack, cell_t value) {
    if (!stack || stack_is_full(stack)) {
        return false;
    }
    
    stack->data[++stack->sp] = value;
    return true;
}

bool stack_pop(stack_t *stack, cell_t *value) {
    if (!stack || stack_is_empty(stack)) {
        return false;
    }
    
    if (value) {
        *value = stack->data[stack->sp];
    }
    stack->sp--;
    return true;
}

bool stack_peek(stack_t *stack, cell_t *value) {
    if (!stack || stack_is_empty(stack)) {
        return false;
    }
    
    if (value) {
        *value = stack->data[stack->sp];
    }
    return true;
}

bool stack_is_empty(stack_t *stack) {
    return !stack || stack->sp < 0;
}

bool stack_is_full(stack_t *stack) {
    return !stack || stack->sp >= stack->size - 1;
}

int stack_depth(stack_t *stack) {
    return stack ? stack->sp + 1 : 0;
}

void stack_clear(stack_t *stack) {
    if (stack) {
        stack->sp = -1;
    }
}

/* Stack manipulation operations */

bool stack_dup(stack_t *stack) {
    if (!stack || stack_is_empty(stack) || stack_is_full(stack)) {
        return false;
    }
    
    cell_t top = stack->data[stack->sp];
    return stack_push(stack, top);
}

bool stack_drop(stack_t *stack) {
    return stack_pop(stack, NULL);
}

bool stack_swap(stack_t *stack) {
    if (!stack || stack_depth(stack) < 2) {
        return false;
    }
    
    cell_t top = stack->data[stack->sp];
    cell_t second = stack->data[stack->sp - 1];
    
    stack->data[stack->sp] = second;
    stack->data[stack->sp - 1] = top;
    
    return true;
}

bool stack_over(stack_t *stack) {
    if (!stack || stack_depth(stack) < 2 || stack_is_full(stack)) {
        return false;
    }
    
    cell_t second = stack->data[stack->sp - 1];
    return stack_push(stack, second);
}

bool stack_rot(stack_t *stack) {
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

void stack_print(stack_t *stack) {
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
        printf("%ld ", (long)stack->data[i]);
    }
    printf("\n");
}