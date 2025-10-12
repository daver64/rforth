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
} rforth_stack_t;

/* Stack operations */
rforth_stack_t* stack_create(int size);
void stack_destroy(rforth_stack_t *stack);
bool stack_push(rforth_stack_t *stack, cell_t value);
bool stack_pop(rforth_stack_t *stack, cell_t *value);
bool stack_peek(rforth_stack_t *stack, cell_t *value);
bool stack_is_empty(rforth_stack_t *stack);
bool stack_is_full(rforth_stack_t *stack);
int stack_depth(rforth_stack_t *stack);
void stack_clear(rforth_stack_t *stack);

/* Stack manipulation operations */
bool stack_dup(rforth_stack_t *stack);         /* Duplicate top */
bool stack_drop(rforth_stack_t *stack);        /* Drop top */
bool stack_swap(rforth_stack_t *stack);        /* Swap top two */
bool stack_over(rforth_stack_t *stack);        /* Copy second to top */
bool stack_rot(rforth_stack_t *stack);         /* Rotate top three */

/* Stack debugging */
void stack_print(rforth_stack_t *stack);

#endif /* STACK_H */