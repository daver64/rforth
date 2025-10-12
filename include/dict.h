#ifndef DICT_H
#define DICT_H

#include <stdbool.h>
#include "stack.h"

#ifndef MAX_WORD_LENGTH
#define MAX_WORD_LENGTH 64
#endif

/* Forward declarations */
typedef struct rforth_ctx rforth_ctx_t;

/* Word types */
typedef enum {
    WORD_BUILTIN,       /* C function */
    WORD_USER,          /* User-defined Forth word */
    WORD_IMMEDIATE,     /* Immediate word (executes during compilation) */
    WORD_CONSTANT,      /* Constant value */
    WORD_VARIABLE       /* Variable */
} word_type_t;

/* Word structure */
typedef struct word {
    char name[MAX_WORD_LENGTH];
    word_type_t type;
    union {
        void (*builtin)(rforth_ctx_t *ctx);    /* Builtin function */
        char *definition;                       /* User definition */
        cell_t value;                          /* Constant/variable value */
    } code;
    struct word *next;          /* Next word in dictionary */
} word_t;

/* Dictionary structure */
typedef struct {
    word_t *latest;             /* Most recently defined word */
    int count;                  /* Number of words */
} dict_t;

/* Dictionary operations */
dict_t* dict_create(void);
void dict_destroy(dict_t *dict);
word_t* dict_find(dict_t *dict, const char *name);
bool dict_add_builtin(dict_t *dict, const char *name, void (*func)(rforth_ctx_t *ctx));
bool dict_add_user_word(dict_t *dict, const char *name, const char *definition);
bool dict_add_constant(dict_t *dict, const char *name, cell_t value);
bool dict_add_variable(dict_t *dict, const char *name, cell_t initial_value);
void dict_print(dict_t *dict);

/* Word execution */
void word_execute(rforth_ctx_t *ctx, word_t *word);

#endif /* DICT_H */