#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <stdbool.h>

/* Stack cell type - 64-bit signed integer */
typedef int64_t cell_t;

/* Stack structure */
typedef struct {
    cell_t *data;       /* Stack data array */
    int sp;             /* Stack pointer (top of stack) */
    int size;           /* Maximum stack size */
} stack_t;

/* Stack operations */
stack_t* stack_create(int size);
void stack_destroy(stack_t *stack);
bool stack_push(stack_t *stack, cell_t value);
bool stack_pop(stack_t *stack, cell_t *value);
bool stack_peek(stack_t *stack, cell_t *value);
bool stack_is_empty(stack_t *stack);
bool stack_is_full(stack_t *stack);
int stack_depth(stack_t *stack);
void stack_clear(stack_t *stack);

/* Stack manipulation operations */
bool stack_dup(stack_t *stack);         /* Duplicate top */
bool stack_drop(stack_t *stack);        /* Drop top */
bool stack_swap(stack_t *stack);        /* Swap top two */
bool stack_over(stack_t *stack);        /* Copy second to top */
bool stack_rot(stack_t *stack);         /* Rotate top three */

/* Stack debugging */
void stack_print(stack_t *stack);

#endif /* STACK_H */