#include "parser.h"
#include "rforth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

parser_t* parser_create(void) {
    parser_t *parser = malloc(sizeof(parser_t));
    if (!parser) return NULL;
    
    parser->input = NULL;
    parser->current = NULL;
    parser->line = 1;
    parser->col = 1;
    parser->state = PARSE_INTERPRET;
    parser->compile_buffer = NULL;
    parser->compile_buffer_size = 0;
    parser->compile_buffer_pos = 0;
    
    return parser;
}

void parser_destroy(parser_t *parser) {
    if (parser) {
        if (parser->compile_buffer) {
            free(parser->compile_buffer);
        }
        free(parser);
    }
}

void parser_set_input(parser_t *parser, const char *input) {
    if (!parser) return;
    
    parser->input = input;
    parser->current = input;
    parser->line = 1;
    parser->col = 1;
}

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

void parser_skip_whitespace(parser_t *parser) {
    if (!parser || !parser->current) return;
    
    while (*parser->current && is_whitespace(*parser->current)) {
        if (*parser->current == '\n') {
            parser->line++;
            parser->col = 1;
        } else {
            parser->col++;
        }
        parser->current++;
    }
}

void parser_skip_comment(parser_t *parser) {
    if (!parser || !parser->current) return;
    
    /* Skip until ')' or end of input */
    while (*parser->current && *parser->current != ')') {
        if (*parser->current == '\n') {
            parser->line++;
            parser->col = 1;
        } else {
            parser->col++;
        }
        parser->current++;
    }
    
    /* Skip the closing ')' */
    if (*parser->current == ')') {
        parser->current++;
        parser->col++;
    }
}

bool parser_is_number(const char *text, int64_t *value) {
    if (!text || !*text) return false;
    
    char *endptr;
    long long result = strtoll(text, &endptr, 10);
    
    /* Check if entire string was consumed and no overflow */
    if (*endptr == '\0' && endptr != text) {
        if (value) *value = (int64_t)result;
        return true;
    }
    
    return false;
}

bool parser_is_float(const char *text, double *value) {
    if (!text || !*text) return false;
    
    char *endptr;
    double result = strtod(text, &endptr);
    
    /* Check if entire string was consumed and contains decimal point or exponent */
    if (*endptr == '\0' && endptr != text) {
        /* Must contain '.' or 'e'/'E' to be considered a float */
        if (strchr(text, '.') || strchr(text, 'e') || strchr(text, 'E')) {
            if (value) *value = result;
            return true;
        }
    }
    
    return false;
}

token_t parser_next_token(parser_t *parser) {
    token_t token = {0};
    
    if (!parser || !parser->current) {
        token.type = TOKEN_EOF;
        return token;
    }
    
    /* Skip whitespace */
    parser_skip_whitespace(parser);
    
    /* Check for end of input */
    if (!*parser->current) {
        token.type = TOKEN_EOF;
        return token;
    }
    
    /* Save position */
    token.line = parser->line;
    token.col = parser->col;
    
    /* Parse token based on first character */
    char c = *parser->current;
    
    if (c == '(') {
        /* Comment start */
        token.type = TOKEN_COMMENT_START;
        strcpy(token.text, "(");
        parser->current++;
        parser->col++;
        parser_skip_comment(parser);
        return token;
    }
    
    if (c == ':') {
        /* Colon */
        token.type = TOKEN_COLON;
        strcpy(token.text, ":");
        parser->current++;
        parser->col++;
        return token;
    }
    
    if (c == ';') {
        /* Semicolon */
        token.type = TOKEN_SEMICOLON;
        strcpy(token.text, ";");
        parser->current++;
        parser->col++;
        return token;
    }
    
    if (c == '"') {
        /* String literal (not implemented yet) */
        token.type = TOKEN_STRING;
        parser->current++;
        parser->col++;
        
        int i = 0;
        while (*parser->current && *parser->current != '"' && i < MAX_WORD_LENGTH - 1) {
            token.text[i++] = *parser->current++;
            parser->col++;
        }
        token.text[i] = '\0';
        
        if (*parser->current == '"') {
            parser->current++;
            parser->col++;
        }
        
        return token;
    }
    
    /* Parse word or number */
    int i = 0;
    
    /* Collect non-whitespace characters */
    while (*parser->current && !is_whitespace(*parser->current) && 
           *parser->current != '(' && *parser->current != ')' &&
           i < MAX_WORD_LENGTH - 1) {
        token.text[i++] = *parser->current++;
        parser->col++;
    }
    token.text[i] = '\0';
    
    /* Determine if it's a number, float, or word */
    int64_t number_value;
    double float_value;
    
    if (parser_is_float(token.text, &float_value)) {
        token.type = TOKEN_FLOAT;
        token.float_val = float_value;
    } else if (parser_is_number(token.text, &number_value)) {
        token.type = TOKEN_NUMBER;
        token.number = number_value;
    } else {
        token.type = TOKEN_WORD;
    }
    
    return token;
}