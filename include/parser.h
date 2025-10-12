#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdint.h>

#ifndef MAX_WORD_LENGTH
#define MAX_WORD_LENGTH 64
#endif

/* Parser state */
typedef enum {
    PARSE_INTERPRET,    /* Interpreting mode */
    PARSE_COMPILE,      /* Compiling mode (inside colon definition) */
    PARSE_COMMENT       /* Inside comment */
} parse_state_t;

/* Token types */
typedef enum {
    TOKEN_WORD,         /* Word name */
    TOKEN_NUMBER,       /* Integer numeric literal */
    TOKEN_FLOAT,        /* Floating point literal */
    TOKEN_STRING,       /* String literal */
    TOKEN_COLON,        /* : (start definition) */
    TOKEN_SEMICOLON,    /* ; (end definition) */
    TOKEN_COMMENT_START, /* ( */
    TOKEN_COMMENT_END,   /* ) */
    
    /* Control flow tokens */
    TOKEN_IF,           /* if */
    TOKEN_THEN,         /* then */
    TOKEN_ELSE,         /* else */
    TOKEN_BEGIN,        /* begin */
    TOKEN_UNTIL,        /* until */
    TOKEN_WHILE,        /* while */
    TOKEN_REPEAT,       /* repeat */
    TOKEN_DO,           /* do */
    TOKEN_LOOP,         /* loop */
    TOKEN_LEAVE,        /* leave */
    
    TOKEN_EOF,          /* End of input */
    TOKEN_ERROR         /* Parse error */
} token_type_t;

/* Token structure */
typedef struct {
    token_type_t type;
    char text[MAX_WORD_LENGTH];
    union {
        int64_t number;     /* For integer tokens */
        double float_val;   /* For floating point tokens */
    };
    int line, col;      /* Position in source */
} token_t;

/* Parser structure */
typedef struct {
    const char *input;          /* Input string */
    const char *current;        /* Current position */
    int line, col;              /* Current line and column */
    parse_state_t state;        /* Parser state */
    char *compile_buffer;       /* Buffer for word being compiled */
    int compile_buffer_size;    /* Size of compile buffer */
    int compile_buffer_pos;     /* Current position in compile buffer */
} parser_t;

/* Parser operations */
parser_t* parser_create(void);
void parser_destroy(parser_t *parser);
void parser_set_input(parser_t *parser, const char *input);
token_t parser_next_token(parser_t *parser);
bool parser_is_number(const char *text, int64_t *value);
bool parser_is_float(const char *text, double *value);
void parser_skip_whitespace(parser_t *parser);
void parser_skip_comment(parser_t *parser);

/* Parsing utilities */
bool is_whitespace(char c);
bool is_alpha(char c);
bool is_digit(char c);
bool is_alnum(char c);

#endif /* PARSER_H */