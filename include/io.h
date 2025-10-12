#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* I/O interface for pluggable backends */
typedef struct {
    /* Input functions */
    int (*read_char)(void);              /* Read single character, -1 on EOF */
    bool (*data_available)(void);        /* Check if input data is available */
    
    /* Output functions */
    void (*write_char)(char c);          /* Write single character */
    void (*write_string)(const char *str); /* Write string */
    void (*write_number)(int64_t n);     /* Write number */
    void (*newline)(void);               /* Write newline */
    
    /* Error output */
    void (*error_string)(const char *str); /* Write error message */
    
    /* Backend identification */
    const char *name;                    /* Backend name */
    void *context;                       /* Backend-specific context */
} io_interface_t;

/* I/O context with current interface */
typedef struct {
    io_interface_t *current;            /* Current I/O backend */
    io_interface_t *terminal;           /* Terminal backend */
    io_interface_t *file;               /* File backend */
    io_interface_t *serial;             /* Serial port backend */
    io_interface_t *network;            /* Network backend */
} io_ctx_t;

/* Global I/O context */
extern io_ctx_t *g_io_ctx;

/* I/O system management */
io_ctx_t* io_init(void);
void io_cleanup(io_ctx_t *ctx);
bool io_set_backend(io_ctx_t *ctx, const char *backend_name);

/* High-level I/O functions (use current backend) */
int io_read_char(void);
bool io_data_available(void);
void io_write_char(char c);
void io_write_string(const char *str);
void io_write_number(int64_t n);
void io_newline(void);
void io_error_string(const char *str);

/* Backend registration */
bool io_register_backend(io_ctx_t *ctx, const char *name, io_interface_t *interface);

/* Built-in backends */
io_interface_t* io_terminal_backend_create(void);
io_interface_t* io_file_backend_create(const char *input_file, const char *output_file);
void io_backend_destroy(io_interface_t *interface);

/* Utility functions */
void io_printf(const char *format, ...);  /* printf replacement */
bool io_read_line(char *buffer, size_t size); /* Read line of input */

#endif /* IO_H */