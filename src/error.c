#include "rforth.h"
#include "io.h"

/* Error message strings */
static const char* error_messages[] = {
    [RFORTH_OK] = "No error",
    
    /* Stack errors */
    [RFORTH_ERROR_STACK_UNDERFLOW] = "Stack underflow",
    [RFORTH_ERROR_STACK_OVERFLOW] = "Stack overflow", 
    [RFORTH_ERROR_RETURN_STACK_UNDERFLOW] = "Return stack underflow",
    [RFORTH_ERROR_RETURN_STACK_OVERFLOW] = "Return stack overflow",
    
    /* Dictionary errors */
    [RFORTH_ERROR_WORD_NOT_FOUND] = "Word not found",
    [RFORTH_ERROR_WORD_REDEFINED] = "Word redefined",
    [RFORTH_ERROR_DICT_FULL] = "Dictionary full",
    
    /* Parser errors */
    [RFORTH_ERROR_PARSE_ERROR] = "Parse error",
    [RFORTH_ERROR_SYNTAX_ERROR] = "Syntax error", 
    [RFORTH_ERROR_UNEXPECTED_EOF] = "Unexpected end of file",
    [RFORTH_ERROR_INVALID_NUMBER] = "Invalid number format",
    [RFORTH_ERROR_UNTERMINATED_STRING] = "Unterminated string",
    
    /* File errors */
    [RFORTH_ERROR_FILE_NOT_FOUND] = "File not found",
    [RFORTH_ERROR_FILE_READ_ERROR] = "File read error",
    [RFORTH_ERROR_FILE_WRITE_ERROR] = "File write error", 
    [RFORTH_ERROR_FILE_PERMISSION] = "File permission denied",
    
    /* Compiler errors */
    [RFORTH_ERROR_COMPILE_ERROR] = "Compilation error",
    [RFORTH_ERROR_COMPILER_NOT_FOUND] = "Compiler not found",
    [RFORTH_ERROR_LINK_ERROR] = "Link error",
    
    /* Memory errors */
    [RFORTH_ERROR_MEMORY] = "Memory allocation error",
    [RFORTH_ERROR_NULL_POINTER] = "Null pointer dereference",
    [RFORTH_ERROR_BUFFER_OVERFLOW] = "Buffer overflow",
    
    /* I/O errors */
    [RFORTH_ERROR_IO_TIMEOUT] = "I/O timeout",
    [RFORTH_ERROR_IO_FAILURE] = "I/O failure",
    [RFORTH_ERROR_IO_NOT_AVAILABLE] = "I/O not available",
    
    /* Runtime errors */
    [RFORTH_ERROR_DIVISION_BY_ZERO] = "Division by zero",
    [RFORTH_ERROR_INVALID_OPERATION] = "Invalid operation",
    [RFORTH_ERROR_EXECUTION_ERROR] = "Execution error",
    
    /* Generic error */
    [RFORTH_ERROR_UNKNOWN] = "Unknown error"
};

void rforth_set_error(rforth_ctx_t *ctx, rforth_error_t code, const char *message,
                      const char *function, const char *filename, int line, int column) {
    if (!ctx) return;
    
    ctx->last_error.code = code;
    ctx->last_error.message = message ? message : rforth_error_string(code);
    ctx->last_error.function = function;
    ctx->last_error.filename = filename;
    ctx->last_error.line = line;
    ctx->last_error.column = column;
}

void rforth_clear_error(rforth_ctx_t *ctx) {
    if (!ctx) return;
    
    ctx->last_error.code = RFORTH_OK;
    ctx->last_error.message = NULL;
    ctx->last_error.function = NULL;
    ctx->last_error.filename = NULL;
    ctx->last_error.line = 0;
    ctx->last_error.column = 0;
}

const char* rforth_error_string(rforth_error_t code) {
    if (code >= 0 && code < sizeof(error_messages)/sizeof(error_messages[0])) {
        return error_messages[code];
    }
    return error_messages[RFORTH_ERROR_UNKNOWN];
}

void rforth_print_error(rforth_ctx_t *ctx) {
    if (!ctx || ctx->last_error.code == RFORTH_OK) {
        return;
    }
    
    io_error_string("Error: ");
    io_error_string(ctx->last_error.message);
    
    if (ctx->last_error.line > 0) {
        char line_info[64];
        snprintf(line_info, sizeof(line_info), " at line %d", ctx->last_error.line);
        io_error_string(line_info);
        
        if (ctx->last_error.column > 0) {
            char col_info[32];
            snprintf(col_info, sizeof(col_info), ", column %d", ctx->last_error.column);
            io_error_string(col_info);
        }
    }
    
    if (ctx->last_error.function) {
        io_error_string(" in ");
        io_error_string(ctx->last_error.function);
        io_error_string("()");
    }
    
    io_error_string("\n");
}