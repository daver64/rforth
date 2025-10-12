#include "dict.h"
#include "rforth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

dict_t* dict_create(void) {
    dict_t *dict = malloc(sizeof(dict_t));
    if (!dict) return NULL;
    
    dict->latest = NULL;
    dict->count = 0;
    return dict;
}

void dict_destroy(dict_t *dict) {
    if (!dict) return;
    
    word_t *current = dict->latest;
    while (current) {
        word_t *next = current->next;
        
        /* Free definition string for user words */
        if (current->type == WORD_USER && current->code.definition) {
            free(current->code.definition);
        }
        
        free(current);
        current = next;
    }
    
    free(dict);
}

word_t* dict_find(dict_t *dict, const char *name) {
    if (!dict || !name) return NULL;
    
    word_t *current = dict->latest;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

static word_t* dict_create_word(const char *name) {
    if (!name || strlen(name) >= MAX_WORD_LENGTH) {
        return NULL;
    }
    
    word_t *word = malloc(sizeof(word_t));
    if (!word) return NULL;
    
    strcpy(word->name, name);
    word->next = NULL;
    return word;
}

static bool dict_add_word(dict_t *dict, word_t *word) {
    if (!dict || !word) return false;
    
    /* Check if word already exists */
    if (dict_find(dict, word->name)) {
        /* Allow redefinition - remove old word */
        /* For now, just add to front (shadows old definition) */
    }
    
    word->next = dict->latest;
    dict->latest = word;
    dict->count++;
    return true;
}

bool dict_add_builtin(dict_t *dict, const char *name, void (*func)(rforth_ctx_t *ctx)) {
    if (!dict || !name || !func) return false;
    
    word_t *word = dict_create_word(name);
    if (!word) return false;
    
    word->type = WORD_BUILTIN;
    word->code.builtin = func;
    
    return dict_add_word(dict, word);
}

bool dict_add_user_word(dict_t *dict, const char *name, const char *definition) {
    if (!dict || !name || !definition) return false;
    
    word_t *word = dict_create_word(name);
    if (!word) return false;
    
    /* Copy definition string */
    word->code.definition = malloc(strlen(definition) + 1);
    if (!word->code.definition) {
        free(word);
        return false;
    }
    strcpy(word->code.definition, definition);
    
    word->type = WORD_USER;
    
    return dict_add_word(dict, word);
}

bool dict_add_constant(dict_t *dict, const char *name, cell_t value) {
    if (!dict || !name) return false;
    
    word_t *word = dict_create_word(name);
    if (!word) return false;
    
    word->type = WORD_CONSTANT;
    word->code.value = value;
    
    return dict_add_word(dict, word);
}

bool dict_add_variable(dict_t *dict, const char *name, cell_t initial_value) {
    if (!dict || !name) return false;
    
    word_t *word = dict_create_word(name);
    if (!word) return false;
    
    word->type = WORD_VARIABLE;
    word->code.value = initial_value;
    
    return dict_add_word(dict, word);
}

void dict_print(dict_t *dict) {
    if (!dict) {
        printf("Dictionary: <null>\n");
        return;
    }
    
    printf("Dictionary (%d words):\n", dict->count);
    
    word_t *current = dict->latest;
    while (current) {
        const char *type_str;
        switch (current->type) {
            case WORD_BUILTIN: type_str = "builtin"; break;
            case WORD_USER: type_str = "user"; break;
            case WORD_IMMEDIATE: type_str = "immediate"; break;
            case WORD_CONSTANT: type_str = "constant"; break;
            case WORD_VARIABLE: type_str = "variable"; break;
            default: type_str = "unknown"; break;
        }
        
        printf("  %-20s (%s)", current->name, type_str);
        
        if (current->type == WORD_CONSTANT || current->type == WORD_VARIABLE) {
            if (current->code.value.type == CELL_INT) {
                printf(" = %ld", (long)current->code.value.value.i);
            } else {
                printf(" = %.6g", current->code.value.value.f);
            }
        } else if (current->type == WORD_USER) {
            printf(" : %s", current->code.definition ? current->code.definition : "<null>");
        }
        
        printf("\n");
        current = current->next;
    }
}

void word_execute(rforth_ctx_t *ctx, word_t *word) {
    if (!ctx || !word) return;
    
    switch (word->type) {
        case WORD_BUILTIN:
            if (word->code.builtin) {
                word->code.builtin(ctx);
            }
            break;
            
        case WORD_USER:
            if (word->code.definition) {
                /* Execute user-defined word by interpreting its definition */
                /* Save current parser state */
                parser_t *saved_parser = ctx->parser;
                parse_state_t saved_state = ctx->state;
                
                /* Create temporary parser for user word execution */
                ctx->parser = parser_create();
                if (!ctx->parser) {
                    RFORTH_SET_ERROR(ctx, RFORTH_ERROR_MEMORY, "Failed to create parser for word execution");
                    ctx->parser = saved_parser;
                    return;
                }
                
                ctx->state = PARSE_INTERPRET;
                int result = rforth_interpret_string(ctx, word->code.definition);
                
                /* Restore parser state */
                parser_destroy(ctx->parser);
                ctx->parser = saved_parser;
                ctx->state = saved_state;
                
                if (result != 0) {
                    /* Error occurred during user word execution */
                    if (ctx->last_error.code == RFORTH_OK) {
                        /* Set a generic error if none was set */
                        RFORTH_SET_ERROR(ctx, RFORTH_ERROR_EXECUTION_ERROR, "Error executing user word");
                    }
                }
            }
            break;
            
        case WORD_CONSTANT:
            /* Push constant value onto stack */
            stack_push_cell(ctx->data_stack, word->code.value);
            break;
            
        case WORD_VARIABLE:
            /* Push variable address onto stack */
            /* The address of the value field in the word structure */
            uintptr_t addr = (uintptr_t)&word->code.value;
            stack_push_int(ctx->data_stack, (int64_t)addr);
            break;
            
        case WORD_IMMEDIATE:
            /* Execute immediately (during compilation) */
            if (word->code.builtin) {
                word->code.builtin(ctx);
            }
            break;
    }
}