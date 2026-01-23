#include "rforth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

rforth_ctx_t* rforth_init(void) {
    rforth_ctx_t *ctx = malloc(sizeof(rforth_ctx_t));
    if (!ctx) return NULL;
    
    /* Initialize stacks */
    ctx->data_stack = stack_create(DEFAULT_STACK_SIZE);
    ctx->return_stack = stack_create(DEFAULT_RETURN_STACK_SIZE);
    if (!ctx->data_stack || !ctx->return_stack) {
        rforth_cleanup(ctx);
        return NULL;
    }
    
    /* Initialize dictionary */
    ctx->dict = dict_create();
    if (!ctx->dict) {
        rforth_cleanup(ctx);
        return NULL;
    }
    
    /* Initialize parser */
    ctx->parser = parser_create();
    if (!ctx->parser) {
        rforth_cleanup(ctx);
        return NULL;
    }
    
    /* Initialize state */
    ctx->state = PARSE_INTERPRET;
    ctx->compile_word_name = NULL;
    ctx->running = true;
    ctx->cf_sp = 0;  /* Initialize control flow stack pointer */
    ctx->skip_mode = false;  /* Initialize skip mode */
    ctx->skip_depth = 0;     /* Initialize skip depth */
    ctx->loop_sp = 0;        /* Initialize loop stack pointer */
    ctx->do_loop_sp = 0;     /* Initialize DO/LOOP stack pointer */
    ctx->variables = NULL;  /* Initialize variable list */
    
    /* Initialize ANSI system variables */
    ctx->here_ptr = NULL;    /* Dictionary pointer */
    ctx->numeric_base = 10;  /* Default decimal base */
    ctx->state_var = 0;      /* Interpret mode */
    
    /* Initialize numeric formatting */
    ctx->format_pos = 0;
    ctx->format_buffer[0] = '\0';
    
    /* Initialize compilation state */
    ctx->compiling = false;
    ctx->current_word_name = NULL;
    
    rforth_clear_error(ctx);
    
    /* Register builtin words */
    if (!builtins_register(ctx->dict)) {
        rforth_cleanup(ctx);
        return NULL;
    }
    
    return ctx;
}

void rforth_cleanup(rforth_ctx_t *ctx) {
    if (!ctx) return;
    
    if (ctx->data_stack) stack_destroy(ctx->data_stack);
    if (ctx->return_stack) stack_destroy(ctx->return_stack);
    if (ctx->dict) dict_destroy(ctx->dict);
    if (ctx->parser) parser_destroy(ctx->parser);
    if (ctx->compile_word_name) free(ctx->compile_word_name);
    
    /* Clean up HERE pointer */
    if (ctx->here_ptr) free(ctx->here_ptr);
    
    /* Clean up compilation state */
    if (ctx->current_word_name) free(ctx->current_word_name);
    
    /* Clean up variables */
    variable_entry_t *var = ctx->variables;
    while (var) {
        variable_entry_t *next = var->next;
        free(var);
        var = next;
    }
    
    free(ctx);
}

/* Helper function to find variable and push its address */
static bool handle_variable_word(rforth_ctx_t *ctx, const char *name) {
    variable_entry_t *var = ctx->variables;
    while (var) {
        if (strcmp(var->name, name) == 0) {
            /* Push variable address onto stack */
            uintptr_t addr = (uintptr_t)var;
            stack_push_int(ctx->data_stack, (int64_t)addr);
            return true;
        }
        var = var->next;
    }
    return false;
}

static rforth_error_t interpret_token(rforth_ctx_t *ctx, token_t *token) {
    rforth_clear_error(ctx);
    
    /* Handle skip mode for true control flow */
    if (ctx->skip_mode) {
        /* Only allow control flow words during skip mode */
        if (token->type == TOKEN_WORD) {
            /* Allow control flow words to execute even during skip */
            if (strcmp(token->text, "if") == 0 || 
                strcmp(token->text, "else") == 0 || 
                strcmp(token->text, "then") == 0 ||
                strcmp(token->text, "begin") == 0 ||
                strcmp(token->text, "until") == 0 ||
                strcmp(token->text, "while") == 0 ||
                strcmp(token->text, "repeat") == 0 ||
                strcmp(token->text, "do") == 0 ||
                strcmp(token->text, "loop") == 0 ||
                strcmp(token->text, "+loop") == 0 ||
                strcmp(token->text, "leave") == 0) {
                /* These words will handle skip mode logic */
            } else {
                /* Skip all other words */
                return RFORTH_OK;
            }
        } else {
            /* Skip numbers, floats, and other tokens */
            return RFORTH_OK;
        }
    }
    
    switch (token->type) {
        case TOKEN_NUMBER:
            /* Push integer onto stack */
            if (!stack_push_int(ctx->data_stack, token->value.number)) {
                RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Stack overflow pushing number");
                return RFORTH_ERROR_STACK_OVERFLOW;
            }
            break;
            
        case TOKEN_FLOAT:
            /* Push floating point value onto stack */
            if (!stack_push_float(ctx->data_stack, token->value.float_val)) {
                RFORTH_SET_ERROR(ctx, RFORTH_ERROR_STACK_OVERFLOW, "Stack overflow pushing float");
                return RFORTH_ERROR_STACK_OVERFLOW;
            }
            break;
            
        case TOKEN_WORD: {
            /* Look up word in dictionary */
            word_t *word = dict_find(ctx->dict, token->text);
            if (word) {
                word_execute(ctx, word);
                if (ctx->last_error.code != RFORTH_OK) {
                    return ctx->last_error.code;
                }
            } else if (handle_variable_word(ctx, token->text)) {
                /* Variable found and address pushed */
            } else {
                RFORTH_SET_PARSE_ERROR(ctx, RFORTH_ERROR_WORD_NOT_FOUND, "Word not found", token->line, token->col);
                return RFORTH_ERROR_WORD_NOT_FOUND;
            }
            break;
        }
        
        case TOKEN_COLON:
            /* Start word definition */
            if (ctx->state == PARSE_COMPILE) {
                RFORTH_SET_ERROR(ctx, RFORTH_ERROR_SYNTAX_ERROR, "Nested definitions not allowed");
                return RFORTH_ERROR_SYNTAX_ERROR;
            }
            ctx->state = PARSE_COMPILE;
            break;
            
        case TOKEN_SEMICOLON:
            /* End word definition */
            if (ctx->state != PARSE_COMPILE) {
                RFORTH_SET_ERROR(ctx, RFORTH_ERROR_SYNTAX_ERROR, "Unexpected ';' outside definition");
                return RFORTH_ERROR_SYNTAX_ERROR;
            }
            ctx->state = PARSE_INTERPRET;
            break;
            
        case TOKEN_COMMENT_START:
            /* Comments are already handled by parser */
            break;
            
        case TOKEN_STRING:
            /* String literals not implemented yet */
            RFORTH_SET_ERROR(ctx, RFORTH_ERROR_PARSE_ERROR, "String literals not implemented");
            return RFORTH_ERROR_PARSE_ERROR;
            
        case TOKEN_EOF:
            break;
            
        case TOKEN_ERROR:
            RFORTH_SET_ERROR(ctx, RFORTH_ERROR_PARSE_ERROR, "Parse error");
            return RFORTH_ERROR_PARSE_ERROR;
            
        default:
            RFORTH_SET_ERROR(ctx, RFORTH_ERROR_PARSE_ERROR, "Unknown token type");
            return RFORTH_ERROR_PARSE_ERROR;
    }
    
    return RFORTH_OK;
}

int rforth_interpret_string(rforth_ctx_t *ctx, const char *input) {
    if (!ctx || !input) return -1;
    
    parser_set_input(ctx->parser, input);
    
    char *compile_buffer = NULL;
    int compile_buffer_size = 0;
    int compile_buffer_pos = 0;
    char *word_name = NULL;
    
    token_t token;
    while ((token = parser_next_token(ctx->parser)).type != TOKEN_EOF) {
        
        if (token.type == TOKEN_COLON) {
            /* Start word definition */
            if (ctx->state == PARSE_COMPILE) {
                fprintf(stderr, "Error: Nested definitions not allowed at line %d, col %d\n", 
                        token.line, token.col);
                goto error;
            }
            
            /* Get the word name */
            token_t name_token = parser_next_token(ctx->parser);
            if (name_token.type != TOKEN_WORD) {
                fprintf(stderr, "Error: Expected word name after ':' at line %d, col %d\n", 
                        name_token.line, name_token.col);
                goto error;
            }
            
            /* Allocate word name */
            word_name = malloc(strlen(name_token.text) + 1);
            if (!word_name) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                goto error;
            }
            size_t name_len = strlen(name_token.text);
            strncpy(word_name, name_token.text, name_len);
            word_name[name_len] = '\0';
            
            /* Initialize compile buffer */
            compile_buffer_size = COMPILE_BUFFER_INITIAL_SIZE;
            compile_buffer = malloc(compile_buffer_size);
            if (!compile_buffer) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                goto error;
            }
            compile_buffer[0] = '\0';
            compile_buffer_pos = 0;
            
            ctx->state = PARSE_COMPILE;
            continue;
            
        } else if (token.type == TOKEN_SEMICOLON) {
            /* End word definition */
            if (ctx->state != PARSE_COMPILE) {
                fprintf(stderr, "Error: Unexpected ';' outside definition at line %d, col %d\n", 
                        token.line, token.col);
                goto error;
            }
            
            /* Add word to dictionary */
            if (!dict_add_user_word(ctx->dict, word_name, compile_buffer)) {
                fprintf(stderr, "Error: Failed to add word '%s' to dictionary\n", word_name);
                goto error;
            }
            
            /* Clean up */
            free(word_name);
            free(compile_buffer);
            word_name = NULL;
            compile_buffer = NULL;
            
            ctx->state = PARSE_INTERPRET;
            continue;
        }
        
        if (ctx->state == PARSE_COMPILE) {
            /* Compiling mode - add token to definition */
            const char *token_text = NULL;
            char number_str[MAX_NUMBER_STRING_LENGTH];
            
            if (token.type == TOKEN_NUMBER) {
                snprintf(number_str, sizeof(number_str), "%ld", (long)token.value.number);
                token_text = number_str;
            } else if (token.type == TOKEN_WORD) {
                token_text = token.text;
            } else {
                fprintf(stderr, "Error: Unexpected token in definition at line %d, col %d\n", 
                        token.line, token.col);
                goto error;
            }
            
            /* Check if we need to expand buffer */
            int needed = compile_buffer_pos + (int)strlen(token_text) + 2; /* +2 for space and null */
            if (needed > compile_buffer_size) {
                compile_buffer_size = needed * 2;
                char *new_buffer = realloc(compile_buffer, compile_buffer_size);
                if (!new_buffer) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    goto error;
                }
                compile_buffer = new_buffer;
            }
            
            /* Add token to buffer */
            if (compile_buffer_pos > 0) {
                size_t available = compile_buffer_size - compile_buffer_pos - 1;
                if (available > 0) {
                    compile_buffer[compile_buffer_pos++] = ' ';
                    compile_buffer[compile_buffer_pos] = '\0';
                }
            }
            
            size_t token_len = strlen(token_text);
            size_t available = compile_buffer_size - compile_buffer_pos - 1;
            if (token_len < available) {
                strncpy(compile_buffer + compile_buffer_pos, token_text, token_len);
                compile_buffer_pos += token_len;
                compile_buffer[compile_buffer_pos] = '\0';
            } else {
                /* Truncate if needed */
                strncpy(compile_buffer + compile_buffer_pos, token_text, available);
                compile_buffer_pos += available;
                compile_buffer[compile_buffer_pos] = '\0';
            }
            
        } else {
            /* Interpreting mode - execute token */
            rforth_error_t result = interpret_token(ctx, &token);
            if (result != RFORTH_OK) {
                if (ctx->last_error.code == RFORTH_ERROR_WORD_NOT_FOUND) {
                    fprintf(stderr, "Error: Word '%s' not found at line %d, col %d\n", 
                            token.text, token.line, token.col);
                } else {
                    rforth_print_error(ctx);
                }
                goto error;
            }
        }
    }
    
    /* Check for unclosed definition */
    if (ctx->state == PARSE_COMPILE) {
        fprintf(stderr, "Error: Unclosed definition for word '%s'\n", word_name ? word_name : "<unknown>");
        goto error;
    }
    
    return 0;
    
error:
    if (word_name) free(word_name);
    if (compile_buffer) free(compile_buffer);
    ctx->state = PARSE_INTERPRET;
    return -1;
}

int rforth_interpret_file(rforth_ctx_t *ctx, const char *filename) {
    if (!ctx || !filename) return -1;
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return -1;
    }
    
    /* Read entire file into memory */
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *content = malloc(size + 1);
    if (!content) {
        fclose(file);
        return -1;
    }
    
    size_t read_size = fread(content, 1, size, file);
    content[read_size] = '\0';
    fclose(file);
    
    /* Interpret the content */
    int result = rforth_interpret_string(ctx, content);
    free(content);
    
    return result;
}

int rforth_repl(rforth_ctx_t *ctx) {
    if (!ctx) return -1;
    
    char input[MAX_INPUT_LENGTH];
    
    ctx->running = true;
    while (ctx->running) {
        printf("> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break; /* EOF */
        }
        
        /* Remove trailing newline */
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        /* Skip empty lines */
        if (strlen(input) == 0) {
            continue;
        }
        
        /* Interpret the input */
        if (rforth_interpret_string(ctx, input) == 0) {
            if (ctx->last_error.code == RFORTH_OK) {
                printf("ok");
                if (stack_depth(ctx->data_stack) > 0) {
                    printf(" ");
                    builtin_dot_s(ctx); /* Show stack */
                } else {
                    printf("\n");
                }
            }
        }
        /* Error messages already printed by interpret_string */
    }
    
    return 0;
}

int rforth_compile_file(rforth_ctx_t *ctx, const char *input_file, const char *output_file) {
    (void)ctx; /* Context not needed for compilation */
    
    if (!input_file || !output_file) {
        fprintf(stderr, "Error: Input and output files required\n");
        return -1;
    }
    
    /* Use the compiler to generate C code and compile it */
    if (compile_forth_to_c(input_file, output_file)) {
        return 0;
    } else {
        return -1;
    }
}