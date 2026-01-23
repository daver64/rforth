#include "compiler.h"
#include "rforth.h"
#include "config.h"
#include <ctype.h>
#ifndef _WIN32
    #include <sys/wait.h>
    #include <unistd.h>
#else
    #include <process.h>  /* For _spawnvp on Windows */
#endif

/* Compiler context implementation */
struct compiler_ctx {
    FILE *output;               /* Output C file */
    char *output_filename;      /* Output C filename */
    char *executable_name;      /* Final executable name */
    int word_count;            /* Generated word counter */
    bool in_main;              /* Whether we're in main function */
    int control_depth;         /* Nesting depth for control structures */
    int label_counter;         /* Counter for generating unique labels */
    int if_stack[64];          /* Stack to track nested if statements */
    int if_depth;              /* Current if nesting depth */
};

/* Helper function to convert Forth word name to C identifier */
static void word_name_to_c_identifier(const char *forth_name, char *c_name, size_t size) {
    if (!forth_name || !c_name || size == 0) return;
    
    size_t i = 0, j = 0;
    while (forth_name[i] && j < size - 1) {
        if (forth_name[i] == '-') {
            c_name[j++] = '_';
        } else if (isalnum(forth_name[i])) {
            c_name[j++] = (char)tolower(forth_name[i]);
        }
        /* Skip other characters */
        i++;
    }
    c_name[j] = '\0';
}

/* Helper function to generate word call code */
static void generate_word_call(compiler_ctx_t *compiler, const char *word_name) {
    FILE *output = compiler->output;
    
    /* Arithmetic Operations */
    if (strcmp(word_name, "+") == 0) {
        fprintf(output, "    forth_add();\n");
    } else if (strcmp(word_name, "-") == 0) {
        fprintf(output, "    forth_sub();\n");
    } else if (strcmp(word_name, "*") == 0) {
        fprintf(output, "    forth_mul();\n");
    } else if (strcmp(word_name, "/") == 0) {
        fprintf(output, "    forth_div();\n");
    } else if (strcmp(word_name, "mod") == 0) {
        fprintf(output, "    forth_mod();\n");
    } else if (strcmp(word_name, "abs") == 0) {
        fprintf(output, "    forth_abs();\n");
    } else if (strcmp(word_name, "negate") == 0) {
        fprintf(output, "    forth_negate();\n");
    } else if (strcmp(word_name, "1+") == 0) {
        fprintf(output, "    push(pop() + 1);\n");
    } else if (strcmp(word_name, "1-") == 0) {
        fprintf(output, "    push(pop() - 1);\n");
    } else if (strcmp(word_name, "2*") == 0) {
        fprintf(output, "    push(pop() * 2);\n");
    } else if (strcmp(word_name, "2/") == 0) {
        fprintf(output, "    push(pop() / 2);\n");
        
    /* Stack Operations */
    } else if (strcmp(word_name, "dup") == 0) {
        fprintf(output, "    forth_dup();\n");
    } else if (strcmp(word_name, "drop") == 0) {
        fprintf(output, "    forth_drop();\n");
    } else if (strcmp(word_name, "swap") == 0) {
        fprintf(output, "    forth_swap();\n");
    } else if (strcmp(word_name, "over") == 0) {
        fprintf(output, "    forth_over();\n");
    } else if (strcmp(word_name, "rot") == 0) {
        fprintf(output, "    forth_rot();\n");
    } else if (strcmp(word_name, "2dup") == 0) {
        fprintf(output, "    forth_2dup();\n");
    } else if (strcmp(word_name, "2drop") == 0) {
        fprintf(output, "    forth_2drop();\n");
        
    /* Return Stack Operations */
    } else if (strcmp(word_name, ">r") == 0) {
        fprintf(output, "    forth_to_r();\n");
    } else if (strcmp(word_name, "r>") == 0) {
        fprintf(output, "    forth_r_from();\n");
    } else if (strcmp(word_name, "r@") == 0) {
        fprintf(output, "    forth_r_fetch();\n");
        
    /* Comparison Operations */
    } else if (strcmp(word_name, "=") == 0) {
        fprintf(output, "    forth_equals();\n");
    } else if (strcmp(word_name, "<>") == 0) {
        fprintf(output, "    forth_not_equals();\n");
    } else if (strcmp(word_name, "<") == 0) {
        fprintf(output, "    forth_less_than();\n");
    } else if (strcmp(word_name, ">") == 0) {
        fprintf(output, "    forth_greater_than();\n");
    } else if (strcmp(word_name, "0=") == 0) {
        fprintf(output, "    forth_zero_equals();\n");
    } else if (strcmp(word_name, "0<") == 0) {
        fprintf(output, "    forth_zero_less();\n");
    } else if (strcmp(word_name, "0>") == 0) {
        fprintf(output, "    forth_zero_greater();\n");
        
    /* Logical Operations */
    } else if (strcmp(word_name, "and") == 0) {
        fprintf(output, "    forth_and();\n");
    } else if (strcmp(word_name, "or") == 0) {
        fprintf(output, "    forth_or();\n");
    } else if (strcmp(word_name, "xor") == 0) {
        fprintf(output, "    forth_xor();\n");
    } else if (strcmp(word_name, "invert") == 0) {
        fprintf(output, "    forth_invert();\n");
    } else if (strcmp(word_name, "lshift") == 0) {
        fprintf(output, "    { int64_t n = pop(), val = pop(); push(val << n); }\n");
    } else if (strcmp(word_name, "rshift") == 0) {
        fprintf(output, "    { int64_t n = pop(), val = pop(); push(val >> n); }\n");
        
    /* I/O Operations */
    } else if (strcmp(word_name, ".") == 0) {
        fprintf(output, "    forth_dot();\n");
    } else if (strcmp(word_name, "emit") == 0) {
        fprintf(output, "    forth_emit();\n");
    } else if (strcmp(word_name, "cr") == 0) {
        fprintf(output, "    forth_cr();\n");
    } else if (strcmp(word_name, "space") == 0) {
        fprintf(output, "    forth_space();\n");
    } else if (strcmp(word_name, "spaces") == 0) {
        fprintf(output, "    forth_spaces();\n");
        
    /* Character Operations */
    } else if (strcmp(word_name, "char") == 0) {
        fprintf(output, "    push(65); /* Simplified CHAR */\n");
    } else if (strcmp(word_name, "chars") == 0) {
        fprintf(output, "    /* CHARS - no-op in this implementation */\n");
    } else if (strcmp(word_name, "char+") == 0) {
        fprintf(output, "    push(pop() + 1);\n");
        
    /* Control Flow */
    } else if (strcmp(word_name, "if") == 0) {
        /* Generate label-based control flow */
        int label = ++compiler->label_counter;
        compiler->if_stack[compiler->if_depth++] = label;
        fprintf(output, "    if (!pop()) goto endif_%d;\n", label);
    } else if (strcmp(word_name, "else") == 0) {
        if (compiler->if_depth > 0) {
            int else_label = ++compiler->label_counter;
            int if_label = compiler->if_stack[compiler->if_depth - 1];
            compiler->if_stack[compiler->if_depth - 1] = else_label; /* Update for endif */
            fprintf(output, "    goto endif_%d;\n", else_label);
            fprintf(output, "endif_%d: /* else clause */\n", if_label);
        }
    } else if (strcmp(word_name, "then") == 0) {
        if (compiler->if_depth > 0) {
            int label = compiler->if_stack[--compiler->if_depth];
            fprintf(output, "endif_%d:\n", label);
        }
        
    /* Special words */
    } else if (strcmp(word_name, "bye") == 0) {
        fprintf(output, "    exit(0);\n");
    } else if (strcmp(word_name, ".\"") == 0) {
        fprintf(output, "    /* .\" handled separately in parsing */\n");
    } else {
        /* Assume it's a user word */
        char c_name[MAX_FILENAME_LENGTH];
        word_name_to_c_identifier(word_name, c_name, sizeof(c_name));
        fprintf(output, "    word_%s();\n", c_name);
    }
}



compiler_ctx_t* compiler_create(const char *output_file) {
    if (!output_file) return NULL;
    
    compiler_ctx_t *compiler = malloc(sizeof(compiler_ctx_t));
    if (!compiler) return NULL;
    
    /* Generate temporary C filename */
    compiler->output_filename = malloc(strlen(output_file) + strlen(C_FILE_EXTENSION) + 1);
    if (!compiler->output_filename) {
        free(compiler);
        return NULL;
    }
    snprintf(compiler->output_filename, strlen(output_file) + strlen(C_FILE_EXTENSION) + 1, 
             "%s%s", output_file, C_FILE_EXTENSION);
    
    /* Store executable name */
    compiler->executable_name = malloc(strlen(output_file) + 1);
    if (!compiler->executable_name) {
        free(compiler->output_filename);
        free(compiler);
        return NULL;
    }
    strcpy(compiler->executable_name, output_file);
    compiler->executable_name[strlen(output_file)] = '\0';
    
    /* Open output file */
    compiler->output = fopen(compiler->output_filename, "w");
    if (!compiler->output) {
        free(compiler->executable_name);
        free(compiler->output_filename);
        free(compiler);
        return NULL;
    }
    
    compiler->word_count = 0;
    compiler->in_main = false;
    compiler->control_depth = 0;
    compiler->label_counter = 0;
    compiler->if_depth = 0;
    
    return compiler;
}

void compiler_destroy(compiler_ctx_t *compiler) {
    if (compiler) {
        if (compiler->output) fclose(compiler->output);
        free(compiler->output_filename);
        free(compiler->executable_name);
        free(compiler);
    }
}

bool compiler_generate_header(compiler_ctx_t *compiler) {
    if (!compiler || !compiler->output) return false;
    
    fprintf(compiler->output, "/* Generated by RForth compiler */\n");
    fprintf(compiler->output, "#include <stdio.h>\n");
    fprintf(compiler->output, "#include <stdlib.h>\n");
    fprintf(compiler->output, "#include <stdint.h>\n");
    fprintf(compiler->output, "#include <string.h>\n");
    fprintf(compiler->output, "#include <math.h>\n\n");
    
    fprintf(compiler->output, "#define DEFAULT_STACK_SIZE %d\n", DEFAULT_STACK_SIZE);
    fprintf(compiler->output, "#define RETURN_STACK_SIZE %d\n\n", DEFAULT_STACK_SIZE);
    
    /* Runtime declarations */
    fprintf(compiler->output, "/* Runtime stacks */\n");
    fprintf(compiler->output, "static int64_t stack[DEFAULT_STACK_SIZE];\n");
    fprintf(compiler->output, "static int64_t return_stack[RETURN_STACK_SIZE];\n");
    fprintf(compiler->output, "static int sp = -1;\n");
    fprintf(compiler->output, "static int rsp = -1;\n");
    fprintf(compiler->output, "static int64_t *variables[1000];\n");
    fprintf(compiler->output, "static int var_count = 0;\n");
    fprintf(compiler->output, "static char input_buffer[1024];\n");
    fprintf(compiler->output, "static int input_pos = 0;\n\n");
    
    /* Basic stack operations */
    fprintf(compiler->output, "static void push(int64_t value) {\n");
    fprintf(compiler->output, "    if (sp < DEFAULT_STACK_SIZE - 1) stack[++sp] = value;\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static int64_t pop(void) {\n");
    fprintf(compiler->output, "    return (sp >= 0) ? stack[sp--] : 0;\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void rpush(int64_t value) {\n");
    fprintf(compiler->output, "    if (rsp < RETURN_STACK_SIZE - 1) return_stack[++rsp] = value;\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static int64_t rpop(void) {\n");
    fprintf(compiler->output, "    return (rsp >= 0) ? return_stack[rsp--] : 0;\n");
    fprintf(compiler->output, "}\n\n");
    
    /* I/O Operations */
    fprintf(compiler->output, "static void forth_dot(void) {\n");
    fprintf(compiler->output, "    printf(\"%%ld \", (long)pop());\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_emit(void) {\n");
    fprintf(compiler->output, "    printf(\"%%c\", (char)pop());\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_cr(void) {\n");
    fprintf(compiler->output, "    printf(\"\\n\");\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_space(void) {\n");
    fprintf(compiler->output, "    printf(\" \");\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_spaces(void) {\n");
    fprintf(compiler->output, "    int64_t n = pop();\n");
    fprintf(compiler->output, "    for(int i = 0; i < n; i++) printf(\" \");\n");
    fprintf(compiler->output, "}\n\n");
    
    /* Stack Operations */
    fprintf(compiler->output, "static void forth_dup(void) {\n");
    fprintf(compiler->output, "    if (sp >= 0) push(stack[sp]);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_drop(void) {\n");
    fprintf(compiler->output, "    if (sp >= 0) sp--;\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_swap(void) {\n");
    fprintf(compiler->output, "    if (sp >= 1) { int64_t tmp = stack[sp]; stack[sp] = stack[sp-1]; stack[sp-1] = tmp; }\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_over(void) {\n");
    fprintf(compiler->output, "    if (sp >= 1) push(stack[sp-1]);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_rot(void) {\n");
    fprintf(compiler->output, "    if (sp >= 2) {\n");
    fprintf(compiler->output, "        int64_t c = pop(), b = pop(), a = pop();\n");
    fprintf(compiler->output, "        push(b); push(c); push(a);\n");
    fprintf(compiler->output, "    }\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_2dup(void) {\n");
    fprintf(compiler->output, "    if (sp >= 1) { push(stack[sp-1]); push(stack[sp-1]); }\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_2drop(void) {\n");
    fprintf(compiler->output, "    if (sp >= 0) sp--; if (sp >= 0) sp--;\n");
    fprintf(compiler->output, "}\n\n");
    
    /* Return Stack Operations */
    fprintf(compiler->output, "static void forth_to_r(void) {\n");
    fprintf(compiler->output, "    rpush(pop());\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_r_from(void) {\n");
    fprintf(compiler->output, "    push(rpop());\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_r_fetch(void) {\n");
    fprintf(compiler->output, "    if (rsp >= 0) push(return_stack[rsp]);\n");
    fprintf(compiler->output, "}\n\n");
    
    /* Arithmetic Operations */
    fprintf(compiler->output, "static void forth_add(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a + b);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_sub(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a - b);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_mul(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a * b);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_div(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); if(b) push(a / b);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_mod(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); if(b) push(a %% b);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_abs(void) {\n");
    fprintf(compiler->output, "    int64_t a = pop(); push(a < 0 ? -a : a);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_negate(void) {\n");
    fprintf(compiler->output, "    push(-pop());\n");
    fprintf(compiler->output, "}\n\n");
    
    /* Comparison Operations */
    fprintf(compiler->output, "static void forth_equals(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a == b ? -1 : 0);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_not_equals(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a != b ? -1 : 0);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_less_than(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a < b ? -1 : 0);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_greater_than(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a > b ? -1 : 0);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_zero_equals(void) {\n");
    fprintf(compiler->output, "    push(pop() == 0 ? -1 : 0);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_zero_less(void) {\n");
    fprintf(compiler->output, "    push(pop() < 0 ? -1 : 0);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_zero_greater(void) {\n");
    fprintf(compiler->output, "    push(pop() > 0 ? -1 : 0);\n");
    fprintf(compiler->output, "}\n\n");
    
    /* Logical Operations */
    fprintf(compiler->output, "static void forth_and(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a & b);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_or(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a | b);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_xor(void) {\n");
    fprintf(compiler->output, "    int64_t b = pop(), a = pop(); push(a ^ b);\n");
    fprintf(compiler->output, "}\n\n");
    
    fprintf(compiler->output, "static void forth_invert(void) {\n");
    fprintf(compiler->output, "    push(~pop());\n");
    fprintf(compiler->output, "}\n\n");
    
    return true;
}

bool compiler_generate_word(compiler_ctx_t *compiler, const char *name, const char *definition) {
    if (!compiler || !compiler->output || !name || !definition) return false;
    
    /* Convert name to C-compatible identifier */
    char c_name[MAX_WORD_LENGTH];
    word_name_to_c_identifier(name, c_name, sizeof(c_name));
    
    fprintf(compiler->output, "/* User word: %s */\n", name);
    fprintf(compiler->output, "static void word_%s(void) {\n", c_name);
    
    /* Parse and compile the definition */
    parser_t *parser = parser_create();
    if (!parser) return false;
    
    parser_set_input(parser, definition);
    
    token_t token;
    while ((token = parser_next_token(parser)).type != TOKEN_EOF) {
        switch (token.type) {
            case TOKEN_NUMBER:
                fprintf(compiler->output, "    push(%ld);\n", (long)token.value.number);
                break;
                
            case TOKEN_WORD:
                generate_word_call(compiler, token.text);
                break;
                
            default:
                /* Skip other tokens */
                break;
        }
    }
    
    fprintf(compiler->output, "}\n\n");
    parser_destroy(parser);
    return true;
}

bool compiler_generate_main(compiler_ctx_t *compiler, const char *main_code) {
    if (!compiler || !compiler->output || !main_code) return false;
    
    fprintf(compiler->output, "int main(int argc, char *argv[]) {\n");
    fprintf(compiler->output, "    (void)argc; (void)argv;\n\n");
    
    /* Parse and compile the main code */
    parser_t *parser = parser_create();
    if (!parser) return false;
    
    parser_set_input(parser, main_code);
    
    token_t token;
    while ((token = parser_next_token(parser)).type != TOKEN_EOF) {
        switch (token.type) {
            case TOKEN_NUMBER:
                fprintf(compiler->output, "    push(%ld);\n", (long)token.value.number);
                break;
                
            case TOKEN_WORD:
                /* Handle special words that expect strings */
                if (strcmp(token.text, ".\"") == 0) {
                    /* Manually parse the string content until closing quote */
                    const char *current = parser->current;
                    
                    /* Skip whitespace */
                    while (*current && isspace(*current)) current++;
                    
                    if (*current) {
                        const char *start = current;
                        const char *end = strchr(start, '"');
                        if (end) {
                            /* Extract the string content */
                            size_t len = end - start;
                            char string_content[1024];
                            if (len < sizeof(string_content) - 1) {
                                strncpy(string_content, start, len);
                                string_content[len] = '\0';
                                fprintf(compiler->output, "    printf(\"%s\");\n", string_content);
                                
                                /* Advance parser past the closing quote */
                                parser->current = end + 1;
                            }
                        }
                    }
                } else {
                    generate_word_call(compiler, token.text);
                }
                break;
                
            case TOKEN_STRING:
                /* Handle standalone string literals */
                fprintf(compiler->output, "    printf(\"%s\");\n", token.text);
                break;
                
            default:
                /* Skip other tokens */
                break;
        }
    }
    
    /* Close any remaining open control structures */
    while (compiler->if_depth > 0) {
        int label = compiler->if_stack[--compiler->if_depth];
        fprintf(compiler->output, "endif_%d:\n", label);
    }
    
    fprintf(compiler->output, "\n    return 0;\n");
    fprintf(compiler->output, "}\n");
    
    parser_destroy(parser);
    return true;
}

bool compiler_generate_footer(compiler_ctx_t *compiler) {
    /* Nothing additional needed for footer */
    (void)compiler;
    return true;
}

bool compile_forth_to_c(const char *input_file, const char *output_file) {
    if (!input_file || !output_file) return false;
    
    /* Read the input file */
    FILE *file = fopen(input_file, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", input_file);
        return false;
    }
    
    /* Read entire file into memory */
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *content = malloc(size + 1);
    if (!content) {
        fclose(file);
        return false;
    }
    
    size_t read_size = fread(content, 1, size, file);
    content[read_size] = '\0';
    fclose(file);
    
    /* Create compiler context */
    compiler_ctx_t *compiler = compiler_create(output_file);
    if (!compiler) {
        free(content);
        return false;
    }
    
    /* Generate header */
    if (!compiler_generate_header(compiler)) {
        compiler_destroy(compiler);
        free(content);
        return false;
    }
    
    /* Parse the content and separate word definitions from main code */
    parser_t *parser = parser_create();
    if (!parser) {
        compiler_destroy(compiler);
        free(content);
        return false;
    }
    
    parser_set_input(parser, content);
    
    /* First pass: collect word definitions */
    token_t token;
    while ((token = parser_next_token(parser)).type != TOKEN_EOF) {
        if (token.type == TOKEN_COLON) {
            /* Word definition */
            token_t name_token = parser_next_token(parser);
            if (name_token.type != TOKEN_WORD) {
                fprintf(stderr, "Error: Expected word name after ':'\n");
                parser_destroy(parser);
                compiler_destroy(compiler);
                free(content);
                return false;
            }
            
            /* Collect definition until semicolon */
            char definition[MAX_DEFINITION_LENGTH] = "";
            int def_pos = 0;
            
            while ((token = parser_next_token(parser)).type != TOKEN_EOF) {
                if (token.type == TOKEN_SEMICOLON) {
                    break;
                }
                
                /* Add token to definition */
                if (def_pos > 0 && def_pos < (int)(sizeof(definition) - 2)) {
                    definition[def_pos++] = ' ';
                }
                
                const char *token_text = NULL;
                char number_str[MAX_NUMBER_STRING_LENGTH];
                
                if (token.type == TOKEN_NUMBER) {
                    snprintf(number_str, sizeof(number_str), "%ld", (long)token.value.number);
                    token_text = number_str;
                } else if (token.type == TOKEN_WORD) {
                    token_text = token.text;
                }
                
                if (token_text) {
                    int len = (int)strlen(token_text);
                    if (def_pos + len < (int)(sizeof(definition) - 1)) {
                        strncpy(definition + def_pos, token_text, len);
                        def_pos += len;
                        definition[def_pos] = '\0';
                    }
                }
            }
            
            definition[def_pos] = '\0';
            
            /* Generate the word function */
            if (!compiler_generate_word(compiler, name_token.text, definition)) {
                parser_destroy(parser);
                compiler_destroy(compiler);
                free(content);
                return false;
            }
        }
    }
    
    /* Second pass: collect main code (everything not in word definitions) */
    parser_set_input(parser, content);
    char main_code[2048] = "";
    int main_pos = 0;
    bool in_definition = false;
    
    while ((token = parser_next_token(parser)).type != TOKEN_EOF) {
        if (token.type == TOKEN_COLON) {
            in_definition = true;
        } else if (token.type == TOKEN_SEMICOLON) {
            in_definition = false;
        } else if (!in_definition && token.type != TOKEN_COMMENT_START) {
            /* Add to main code */
            if (main_pos > 0 && main_pos < (int)(sizeof(main_code) - 2)) {
                main_code[main_pos++] = ' ';
            }
            
            const char *token_text = NULL;
            char number_str[MAX_NUMBER_STRING_LENGTH];
            
            if (token.type == TOKEN_NUMBER) {
                snprintf(number_str, sizeof(number_str), "%ld", (long)token.value.number);
                token_text = number_str;
            } else if (token.type == TOKEN_WORD) {
                token_text = token.text;
            }
            
            if (token_text) {
                int len = (int)strlen(token_text);
                if (main_pos + len < (int)(sizeof(main_code) - 1)) {
                    strncpy(main_code + main_pos, token_text, len);
                    main_pos += len;
                    main_code[main_pos] = '\0';
                }
            }
        }
    }
    
    main_code[main_pos] = '\0';
    
    /* Generate main function */
    if (!compiler_generate_main(compiler, main_code)) {
        parser_destroy(parser);
        compiler_destroy(compiler);
        free(content);
        return false;
    }
    
    /* Generate footer */
    compiler_generate_footer(compiler);
    
    /* Compile the C code */
    char *c_filename = compiler->output_filename;
    char *exe_filename = compiler->executable_name;
    
    /* Close the file before compiling */
    fclose(compiler->output);
    compiler->output = NULL;
    
    bool compile_success = invoke_c_compiler(c_filename, exe_filename);
    
    /* Cleanup */
    if (compile_success) {
        /* Keep C file for debugging - comment out to remove */
        /* unlink(c_filename); */
        printf("Generated C code saved as: %s\n", c_filename);
    }
    
    parser_destroy(parser);
    compiler_destroy(compiler);
    free(content);
    
    return compile_success;
}

bool invoke_c_compiler(const char *c_file, const char *output_file) {
    if (!c_file || !output_file) return false;
    
    /* Validate file paths to prevent injection */
    if (strstr(c_file, "..") || strstr(output_file, "..") ||
        strchr(c_file, ';') || strchr(output_file, ';') ||
        strchr(c_file, '&') || strchr(output_file, '&') ||
        strchr(c_file, '|') || strchr(output_file, '|')) {
        fprintf(stderr, "Error: Invalid characters in file paths\n");
        return false;
    }
    
#ifdef _WIN32
    /* Windows process creation */
    char *args[] = {
        "cl.exe",
        "/O2",
        "/Fe:",
        (char*)output_file,
        (char*)c_file,
        NULL
    };
    
    /* Build command string for Windows */
    char command[1024];
    snprintf(command, sizeof(command), "cl.exe /O2 /Fe:\"%s\" \"%s\"", 
             output_file, c_file);
    
    int result = system(command);
    if (result == 0) {
        return true;
    } else {
        fprintf(stderr, "cl.exe compilation failed (exit code: %d)\n", result);
        return false;
    }
#else
    /* Unix fork and exec gcc safely */
    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Error: Failed to fork process\n");
        return false;
    }
    
    if (pid == 0) {
        /* Child process - exec gcc */
        char *args[] = {
            "gcc",
            "-O2",
            "-o",
            (char*)output_file,
            (char*)c_file,
            NULL
        };
        
        execvp("gcc", args);
        /* If we get here, exec failed */
        fprintf(stderr, "Error: Failed to execute gcc\n");
        _exit(1);
    } else {
        /* Parent process - wait for child */
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            fprintf(stderr, "Error: Failed to wait for gcc process\n");
            return false;
        }
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        } else {
            fprintf(stderr, "gcc compilation failed (exit code: %d)\n", 
                    WIFEXITED(status) ? WEXITSTATUS(status) : -1);
            return false;
        }
    }
#endif
}