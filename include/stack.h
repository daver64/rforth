#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <stdbool.h>

/* Cell value types */
typedef enum {
    CELL_INT,       /* Integer value */
    CELL_FLOAT      /* Floating point value */
} cell_type_t;

/* Stack cell - can hold either integer or floating point */
typedef struct {
    cell_type_t type;
    union {
        int64_t i;      /* Integer value */
        double f;       /* Floating point value */
    } value;
} cell_t;

/* Stack structure */
typedef struct {
    cell_t *data;       /* Stack data array */
    int sp;             /* Stack pointer (top of stack) */
    int size;           /* Maximum stack size */
} rforth_stack_t;

/* Stack operations */
rforth_stack_t* stack_create(int size);
void stack_destroy(rforth_stack_t *stack);
bool stack_push_int(rforth_stack_t *stack, int64_t value);
bool stack_push_float(rforth_stack_t *stack, double value);
bool stack_push_cell(rforth_stack_t *stack, cell_t cell);
bool stack_pop(rforth_stack_t *stack, cell_t *value);
bool stack_pop_int(rforth_stack_t *stack, int64_t *value);
bool stack_pop_float(rforth_stack_t *stack, double *value);
bool stack_peek(rforth_stack_t *stack, cell_t *value);
bool stack_is_empty(rforth_stack_t *stack);
bool stack_is_full(rforth_stack_t *stack);
int stack_depth(rforth_stack_t *stack);
void stack_clear(rforth_stack_t *stack);

/* Cell creation utilities */
cell_t cell_make_int(int64_t value);
cell_t cell_make_float(double value);
bool cell_is_int(const cell_t *cell);
bool cell_is_float(const cell_t *cell);
int64_t cell_get_int(const cell_t *cell);
double cell_get_float(const cell_t *cell);
double cell_to_float(const cell_t *cell);  /* Convert any cell to float */

/* Stack manipulation operations */
bool stack_dup(rforth_stack_t *stack);         /* Duplicate top */
bool stack_drop(rforth_stack_t *stack);        /* Drop top */
bool stack_swap(rforth_stack_t *stack);        /* Swap top two */
bool stack_over(rforth_stack_t *stack);        /* Copy second to top */
bool stack_rot(rforth_stack_t *stack);         /* Rotate top three */

/* Stack debugging */
void stack_print(rforth_stack_t *stack);

#endif /* STACK_H */