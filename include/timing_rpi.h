#ifndef TIMING_RPI_H
#define TIMING_RPI_H

#include <stdint.h>
#include "rforth.h"

/* Timing initialization */
void timing_init(void);

/* Delay functions */
void delay_microseconds(uint32_t us);
void delay_milliseconds(uint32_t ms);

/* Time measurement functions */
uint64_t micros(void);
uint64_t millis(void);

/* Forth word implementations */
void builtin_delay_ms(rforth_ctx_t *ctx);
void builtin_delay_us(rforth_ctx_t *ctx);
void builtin_micros(rforth_ctx_t *ctx);
void builtin_millis(rforth_ctx_t *ctx);

#endif /* TIMING_RPI_H */
