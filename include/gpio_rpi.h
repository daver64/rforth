#ifndef GPIO_RPI_H
#define GPIO_RPI_H

#include <stdint.h>
#include <stdbool.h>
#include "rpi_peripherals.h"

/* Forward declaration */
typedef struct rforth_ctx rforth_ctx_t;

/* GPIO Error Codes */
typedef enum {
    GPIO_OK = 0,
    GPIO_ERROR_NOT_INITIALIZED,
    GPIO_ERROR_ALREADY_INITIALIZED,
    GPIO_ERROR_INVALID_PIN,
    GPIO_ERROR_MEMORY_MAP,
    GPIO_ERROR_PERMISSION,
    GPIO_ERROR_PLATFORM,
    GPIO_ERROR_INVALID_MODE,
    GPIO_ERROR_INVALID_VALUE
} gpio_error_t;

/* GPIO Pin Modes */
typedef enum {
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT = 1,
    GPIO_MODE_ALT0 = 4,
    GPIO_MODE_ALT1 = 5,
    GPIO_MODE_ALT2 = 6,
    GPIO_MODE_ALT3 = 7,
    GPIO_MODE_ALT4 = 3,
    GPIO_MODE_ALT5 = 2
} gpio_mode_t;

/* GPIO Pull Up/Down Modes */
typedef enum {
    GPIO_PULL_OFF = 0,
    GPIO_PULL_DOWN = 1,
    GPIO_PULL_UP = 2
} gpio_pull_t;

/* GPIO State Structure */
typedef struct {
    volatile uint32_t *gpio_map;  /* Memory-mapped GPIO registers */
    bool initialized;              /* Initialization flag */
    int mem_fd;                    /* File descriptor for /dev/mem or /dev/gpiomem */
    uint32_t gpio_base;            /* Base address for GPIO */
    rpi_model_t model;             /* Detected Pi model */
} gpio_state_t;

/* Core GPIO Functions */
gpio_error_t gpio_init(void);
gpio_error_t gpio_cleanup(void);
bool gpio_is_initialized(void);

/* Pin Configuration */
gpio_error_t gpio_set_mode(uint8_t pin, gpio_mode_t mode);
gpio_error_t gpio_set_pull(uint8_t pin, gpio_pull_t pull);

/* Digital I/O */
gpio_error_t gpio_write(uint8_t pin, bool value);
gpio_error_t gpio_set(uint8_t pin);
gpio_error_t gpio_clear(uint8_t pin);
gpio_error_t gpio_toggle(uint8_t pin);
gpio_error_t gpio_read(uint8_t pin, bool *value);

/* Batch Operations */
gpio_error_t gpio_mask_set(uint32_t mask);
gpio_error_t gpio_mask_clear(uint32_t mask);
uint32_t gpio_mask_read(void);

/* Utility Functions */
bool gpio_is_valid_pin(uint8_t pin);
const char* gpio_error_string(gpio_error_t error);
gpio_error_t gpio_get_mode(uint8_t pin, gpio_mode_t *mode);

/* Forth Word Implementations */
void builtin_gpio_init(rforth_ctx_t *ctx);
void builtin_gpio_close(rforth_ctx_t *ctx);
void builtin_gpio_output(rforth_ctx_t *ctx);
void builtin_gpio_input(rforth_ctx_t *ctx);
void builtin_gpio_alt0(rforth_ctx_t *ctx);
void builtin_gpio_alt1(rforth_ctx_t *ctx);
void builtin_gpio_alt2(rforth_ctx_t *ctx);
void builtin_gpio_alt3(rforth_ctx_t *ctx);
void builtin_gpio_alt4(rforth_ctx_t *ctx);
void builtin_gpio_alt5(rforth_ctx_t *ctx);
void builtin_gpio_pull_up(rforth_ctx_t *ctx);
void builtin_gpio_pull_down(rforth_ctx_t *ctx);
void builtin_gpio_pull_off(rforth_ctx_t *ctx);
void builtin_gpio_set(rforth_ctx_t *ctx);
void builtin_gpio_clr(rforth_ctx_t *ctx);
void builtin_gpio_write(rforth_ctx_t *ctx);
void builtin_gpio_read(rforth_ctx_t *ctx);
void builtin_gpio_toggle(rforth_ctx_t *ctx);
void builtin_gpio_mask_set(rforth_ctx_t *ctx);
void builtin_gpio_mask_clr(rforth_ctx_t *ctx);
void builtin_gpio_mask_read(rforth_ctx_t *ctx);
void builtin_gpio_valid_q(rforth_ctx_t *ctx);

#endif /* GPIO_RPI_H */
