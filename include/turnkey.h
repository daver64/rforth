#ifndef TURNKEY_H
#define TURNKEY_H

#include "rforth.h"

/* TURNKEY implementation for creating standalone executables */

/* Create a standalone executable from current interpreter state */
bool turnkey_create_executable(rforth_ctx_t *ctx, const char *output_file);

/* Generate C code representing current dictionary state */
bool turnkey_generate_dictionary_code(FILE *output, dict_t *dict);

/* Generate C code for current stack state (if needed) */
bool turnkey_generate_stack_code(FILE *output, rforth_stack_t *stack);

/* Main TURNKEY word implementation */
void builtin_turnkey(rforth_ctx_t *ctx);

#endif /* TURNKEY_H */