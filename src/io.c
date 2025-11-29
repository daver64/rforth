#include "io.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifndef _WIN32
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <sys/select.h>
#endif

/* Global I/O context */
io_ctx_t *g_io_ctx = NULL;

/* Terminal backend implementation */
typedef struct {
    FILE *input;
    FILE *output;
    FILE *error;
} terminal_ctx_t;

static int terminal_read_char(void) {
    terminal_ctx_t *ctx = (terminal_ctx_t*)g_io_ctx->current->context;
    return fgetc(ctx->input);
}

static bool terminal_data_available(void) {
#ifndef _WIN32
    /* Use select() to check whether there is data available on stdin.
     * This performs a non-blocking check (timeout 0). On terminals in
     * canonical mode input will typically be available only after a
     * newline; that's expected behavior for line-buffered terminals.
     */
    int fd = fileno(stdin);
    if (fd < 0) return false;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int r = select(fd + 1, &readfds, NULL, NULL, &tv);
    if (r > 0) {
        return FD_ISSET(fd, &readfds) != 0;
    }
    return false;
#else
    /* On Windows we keep the conservative behavior for now. If needed,
     * a Win32 implementation using _kbhit() or other APIs can be added.
     */
    return true;
#endif
}

static void terminal_write_char(char c) {
    terminal_ctx_t *ctx = (terminal_ctx_t*)g_io_ctx->current->context;
    fputc(c, ctx->output);
    fflush(ctx->output);
}

static void terminal_write_string(const char *str) {
    terminal_ctx_t *ctx = (terminal_ctx_t*)g_io_ctx->current->context;
    fputs(str, ctx->output);
    fflush(ctx->output);
}

static void terminal_write_number(int64_t n) {
    terminal_ctx_t *ctx = (terminal_ctx_t*)g_io_ctx->current->context;
    fprintf(ctx->output, "%ld ", (long)n);
    fflush(ctx->output);
}

static void terminal_newline(void) {
    terminal_ctx_t *ctx = (terminal_ctx_t*)g_io_ctx->current->context;
    fputc('\n', ctx->output);
    fflush(ctx->output);
}

static void terminal_error_string(const char *str) {
    terminal_ctx_t *ctx = (terminal_ctx_t*)g_io_ctx->current->context;
    fputs(str, ctx->error);
    fflush(ctx->error);
}

io_interface_t* io_terminal_backend_create(void) {
    io_interface_t *interface = malloc(sizeof(io_interface_t));
    if (!interface) return NULL;
    
    terminal_ctx_t *ctx = malloc(sizeof(terminal_ctx_t));
    if (!ctx) {
        free(interface);
        return NULL;
    }
    
    ctx->input = stdin;
    ctx->output = stdout;
    ctx->error = stderr;
    
    interface->read_char = terminal_read_char;
    interface->data_available = terminal_data_available;
    interface->write_char = terminal_write_char;
    interface->write_string = terminal_write_string;
    interface->write_number = terminal_write_number;
    interface->newline = terminal_newline;
    interface->error_string = terminal_error_string;
    interface->name = "terminal";
    interface->context = ctx;
    
    return interface;
}

void io_backend_destroy(io_interface_t *interface) {
    if (interface) {
        if (interface->context) {
            free(interface->context);
        }
        free(interface);
    }
}

io_ctx_t* io_init(void) {
    io_ctx_t *ctx = malloc(sizeof(io_ctx_t));
    if (!ctx) return NULL;
    
    /* Initialize all backends to NULL */
    ctx->current = NULL;
    ctx->terminal = NULL;
    ctx->file = NULL;
    ctx->serial = NULL;
    ctx->network = NULL;
    
    /* Create terminal backend by default */
    ctx->terminal = io_terminal_backend_create();
    if (!ctx->terminal) {
        free(ctx);
        return NULL;
    }
    
    /* Set terminal as default */
    ctx->current = ctx->terminal;
    
    /* Set global context */
    g_io_ctx = ctx;
    
    return ctx;
}

void io_cleanup(io_ctx_t *ctx) {
    if (!ctx) return;
    
    if (ctx->terminal) io_backend_destroy(ctx->terminal);
    if (ctx->file) io_backend_destroy(ctx->file);
    if (ctx->serial) io_backend_destroy(ctx->serial);
    if (ctx->network) io_backend_destroy(ctx->network);
    
    /* Clear global context if this is it */
    if (g_io_ctx == ctx) {
        g_io_ctx = NULL;
    }
    
    free(ctx);
}

bool io_set_backend(io_ctx_t *ctx, const char *backend_name) {
    if (!ctx || !backend_name) return false;
    
    if (strcmp(backend_name, "terminal") == 0 && ctx->terminal) {
        ctx->current = ctx->terminal;
        return true;
    } else if (strcmp(backend_name, "file") == 0 && ctx->file) {
        ctx->current = ctx->file;
        return true;
    } else if (strcmp(backend_name, "serial") == 0 && ctx->serial) {
        ctx->current = ctx->serial;
        return true;
    } else if (strcmp(backend_name, "network") == 0 && ctx->network) {
        ctx->current = ctx->network;
        return true;
    }
    
    return false;
}

/* High-level I/O functions */
int io_read_char(void) {
    if (!g_io_ctx || !g_io_ctx->current) return -1;
    return g_io_ctx->current->read_char();
}

bool io_data_available(void) {
    if (!g_io_ctx || !g_io_ctx->current) return false;
    return g_io_ctx->current->data_available();
}

void io_write_char(char c) {
    if (!g_io_ctx || !g_io_ctx->current) return;
    g_io_ctx->current->write_char(c);
}

void io_write_string(const char *str) {
    if (!g_io_ctx || !g_io_ctx->current || !str) return;
    g_io_ctx->current->write_string(str);
}

void io_write_number(int64_t n) {
    if (!g_io_ctx || !g_io_ctx->current) return;
    g_io_ctx->current->write_number(n);
}

void io_newline(void) {
    if (!g_io_ctx || !g_io_ctx->current) return;
    g_io_ctx->current->newline();
}

void io_error_string(const char *str) {
    if (!g_io_ctx || !g_io_ctx->current || !str) return;
    g_io_ctx->current->error_string(str);
}

bool io_register_backend(io_ctx_t *ctx, const char *name, io_interface_t *interface) {
    if (!ctx || !name || !interface) return false;
    
    if (strcmp(name, "terminal") == 0) {
        if (ctx->terminal) io_backend_destroy(ctx->terminal);
        ctx->terminal = interface;
    } else if (strcmp(name, "file") == 0) {
        if (ctx->file) io_backend_destroy(ctx->file);
        ctx->file = interface;
    } else if (strcmp(name, "serial") == 0) {
        if (ctx->serial) io_backend_destroy(ctx->serial);
        ctx->serial = interface;
    } else if (strcmp(name, "network") == 0) {
        if (ctx->network) io_backend_destroy(ctx->network);
        ctx->network = interface;
    } else {
        return false;
    }
    
    return true;
}

void io_printf(const char *format, ...) {
    if (!g_io_ctx || !g_io_ctx->current || !format) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[IO_BUFFER_SIZE];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    va_end(args);
    
    io_write_string(buffer);
}

bool io_read_line(char *buffer, size_t size) {
    if (!buffer || size == 0) return false;
    
    size_t i = 0;
    int c = 0;
    
    while (i < size - 1) {
        c = io_read_char();
        if (c == -1) {
            /* EOF */
            break;
        } else if (c == '\n') {
            /* End of line */
            break;
        } else if (c == '\r') {
            /* Ignore carriage return */
            continue;
        } else {
            buffer[i++] = (char)c;
        }
    }
    
    buffer[i] = '\0';
    return i > 0 || c != -1;
}