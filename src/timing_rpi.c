#include "timing_rpi.h"
#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Global start time for relative timing */
static struct timespec start_time;
static bool timing_initialized = false;

/* Initialize timing system */
void timing_init(void) {
    if (!timing_initialized) {
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        timing_initialized = true;
    }
}

/* Get microseconds since initialization */
uint64_t micros(void) {
    struct timespec current_time;
    
    if (!timing_initialized) {
        timing_init();
    }
    
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    
    uint64_t current_us = (uint64_t)current_time.tv_sec * 1000000ULL + 
                          (uint64_t)current_time.tv_nsec / 1000ULL;
    uint64_t start_us = (uint64_t)start_time.tv_sec * 1000000ULL + 
                        (uint64_t)start_time.tv_nsec / 1000ULL;
    
    return current_us - start_us;
}

/* Get milliseconds since initialization */
uint64_t millis(void) {
    return micros() / 1000;
}

/* Delay for specified microseconds */
void delay_microseconds(uint32_t us) {
    if (us == 0) {
        return;
    }
    
    struct timespec req, rem;
    
    req.tv_sec = us / 1000000;
    req.tv_nsec = (us % 1000000) * 1000;
    
    /* Use nanosleep with retry on interrupt */
    while (nanosleep(&req, &rem) == -1) {
        if (errno == EINTR) {
            /* Interrupted by signal, continue with remaining time */
            req = rem;
        } else {
            /* Other error, give up */
            break;
        }
    }
}

/* Delay for specified milliseconds */
void delay_milliseconds(uint32_t ms) {
    if (ms == 0) {
        return;
    }
    
    delay_microseconds(ms * 1000);
}

/* Forth Word Implementations */

void builtin_delay_ms(rforth_ctx_t *ctx) {
    /* DELAY-MS ( ms -- ) Delay for milliseconds */
    cell_t ms_cell;
    if (!stack_pop(ctx->data_stack, &ms_cell) || ms_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "DELAY-MS requires milliseconds on stack", 
                        "timing", __FILE__, __LINE__, 0);
        return;
    }
    
    if (ms_cell.value.i < 0) {
        rforth_set_error(ctx, RFORTH_ERROR_INVALID_OPERATION, 
                        "DELAY-MS requires positive value", 
                        "timing", __FILE__, __LINE__, 0);
        return;
    }
    
    delay_milliseconds((uint32_t)ms_cell.value.i);
}

void builtin_delay_us(rforth_ctx_t *ctx) {
    /* DELAY-US ( us -- ) Delay for microseconds */
    cell_t us_cell;
    if (!stack_pop(ctx->data_stack, &us_cell) || us_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "DELAY-US requires microseconds on stack", 
                        "timing", __FILE__, __LINE__, 0);
        return;
    }
    
    if (us_cell.value.i < 0) {
        rforth_set_error(ctx, RFORTH_ERROR_INVALID_OPERATION, 
                        "DELAY-US requires positive value", 
                        "timing", __FILE__, __LINE__, 0);
        return;
    }
    
    delay_microseconds((uint32_t)us_cell.value.i);
}

void builtin_micros(rforth_ctx_t *ctx) {
    /* MICROS ( -- us ) Get microseconds since start */
    uint64_t us = micros();
    stack_push_int(ctx->data_stack, (int64_t)us);
}

void builtin_millis(rforth_ctx_t *ctx) {
    /* MILLIS ( -- ms ) Get milliseconds since start */
    uint64_t ms = millis();
    stack_push_int(ctx->data_stack, (int64_t)ms);
}
