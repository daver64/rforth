#include "gpio_rpi.h"
#include "rforth.h"
#include "rpi_peripherals.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

/* Global GPIO state */
static gpio_state_t gpio_state = {
    .gpio_map = NULL,
    .initialized = false,
    .mem_fd = -1,
    .gpio_base = 0,
    .model = RPI_MODEL_UNKNOWN
};

/* Hardware detection functions */
rpi_model_t rpi_detect_model(void) {
    FILE *fp;
    char line[256];
    rpi_model_t model = RPI_MODEL_UNKNOWN;
    
    fp = fopen("/proc/device-tree/model", "r");
    if (fp == NULL) {
        fp = fopen("/sys/firmware/devicetree/base/model", "r");
    }
    
    if (fp != NULL) {
        if (fgets(line, sizeof(line), fp) != NULL) {
            if (strstr(line, "Raspberry Pi 4")) {
                model = RPI_MODEL_4;
            } else if (strstr(line, "Raspberry Pi 3")) {
                model = RPI_MODEL_3;
            } else if (strstr(line, "Raspberry Pi 2")) {
                model = RPI_MODEL_2;
            } else if (strstr(line, "Raspberry Pi Zero 2")) {
                model = RPI_MODEL_ZERO_2W;
            } else if (strstr(line, "Raspberry Pi Zero")) {
                model = RPI_MODEL_ZERO;
            } else if (strstr(line, "Raspberry Pi 1") || strstr(line, "Raspberry Pi Model")) {
                model = RPI_MODEL_1;
            }
        }
        fclose(fp);
    }
    
    return model;
}

uint32_t rpi_get_peripheral_base(void) {
    rpi_model_t model = rpi_detect_model();
    
    switch (model) {
        case RPI_MODEL_1:
        case RPI_MODEL_ZERO:
            return BCM2835_PERI_BASE;
            
        case RPI_MODEL_2:
        case RPI_MODEL_3:
        case RPI_MODEL_ZERO_2W:
            return BCM2836_PERI_BASE;
            
        case RPI_MODEL_4:
        case RPI_MODEL_5:
            return BCM2711_PERI_BASE;
            
        default:
            /* Default to BCM2836 for Pi Zero 2W compatibility */
            return BCM2836_PERI_BASE;
    }
}

const char* rpi_get_model_name(rpi_model_t model) {
    switch (model) {
        case RPI_MODEL_1: return "Raspberry Pi 1";
        case RPI_MODEL_ZERO: return "Raspberry Pi Zero";
        case RPI_MODEL_ZERO_2W: return "Raspberry Pi Zero 2W";
        case RPI_MODEL_2: return "Raspberry Pi 2";
        case RPI_MODEL_3: return "Raspberry Pi 3";
        case RPI_MODEL_4: return "Raspberry Pi 4";
        case RPI_MODEL_5: return "Raspberry Pi 5";
        default: return "Unknown Raspberry Pi";
    }
}

/* Error string conversion */
const char* gpio_error_string(gpio_error_t error) {
    switch (error) {
        case GPIO_OK: return "Success";
        case GPIO_ERROR_NOT_INITIALIZED: return "GPIO not initialized";
        case GPIO_ERROR_ALREADY_INITIALIZED: return "GPIO already initialized";
        case GPIO_ERROR_INVALID_PIN: return "Invalid GPIO pin number";
        case GPIO_ERROR_MEMORY_MAP: return "Failed to map GPIO memory";
        case GPIO_ERROR_PERMISSION: return "Permission denied (try running as root or add user to gpio group)";
        case GPIO_ERROR_PLATFORM: return "Not running on Raspberry Pi";
        case GPIO_ERROR_INVALID_MODE: return "Invalid GPIO mode";
        case GPIO_ERROR_INVALID_VALUE: return "Invalid value";
        default: return "Unknown error";
    }
}

/* Core GPIO Functions */
gpio_error_t gpio_init(void) {
    if (gpio_state.initialized) {
        return GPIO_ERROR_ALREADY_INITIALIZED;
    }
    
    /* Detect Raspberry Pi model */
    gpio_state.model = rpi_detect_model();
    if (gpio_state.model == RPI_MODEL_UNKNOWN) {
        fprintf(stderr, "Warning: Could not detect Raspberry Pi model, assuming Pi Zero 2W\n");
        gpio_state.model = RPI_MODEL_ZERO_2W;
    }
    
    printf("Detected: %s\n", rpi_get_model_name(gpio_state.model));
    
    /* Get peripheral base address */
    uint32_t peri_base = rpi_get_peripheral_base();
    gpio_state.gpio_base = peri_base + 0x200000;
    
    /* Try to open /dev/gpiomem first (no root required) */
    gpio_state.mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (gpio_state.mem_fd < 0) {
        /* Fall back to /dev/mem (requires root) */
        gpio_state.mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        if (gpio_state.mem_fd < 0) {
            return GPIO_ERROR_PERMISSION;
        }
    }
    
    /* Map GPIO memory */
    void *gpio_map_temp = mmap(
        NULL,                       /* Any address */
        BLOCK_SIZE,                 /* Map length */
        PROT_READ | PROT_WRITE,     /* Enable reading & writing */
        MAP_SHARED,                 /* Shared with other processes */
        gpio_state.mem_fd,          /* File to map */
        gpio_state.gpio_base        /* Offset to GPIO peripheral */
    );
    
    if (gpio_map_temp == MAP_FAILED) {
        close(gpio_state.mem_fd);
        gpio_state.mem_fd = -1;
        return GPIO_ERROR_MEMORY_MAP;
    }
    
    gpio_state.gpio_map = (volatile uint32_t *)gpio_map_temp;
    gpio_state.initialized = true;
    
    return GPIO_OK;
}

gpio_error_t gpio_cleanup(void) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    /* Unmap GPIO memory */
    if (gpio_state.gpio_map != NULL) {
        munmap((void *)gpio_state.gpio_map, BLOCK_SIZE);
        gpio_state.gpio_map = NULL;
    }
    
    /* Close file descriptor */
    if (gpio_state.mem_fd >= 0) {
        close(gpio_state.mem_fd);
        gpio_state.mem_fd = -1;
    }
    
    gpio_state.initialized = false;
    
    return GPIO_OK;
}

bool gpio_is_initialized(void) {
    return gpio_state.initialized;
}

/* Utility Functions */
bool gpio_is_valid_pin(uint8_t pin) {
    /* Pi Zero 2W has GPIO 0-27 */
    if (gpio_state.model == RPI_MODEL_4 || gpio_state.model == RPI_MODEL_5) {
        return pin <= RPI4_GPIO_MAX;
    }
    return pin <= RPI_GPIO_MAX;
}

/* Pin Configuration */
gpio_error_t gpio_set_mode(uint8_t pin, gpio_mode_t mode) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!gpio_is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }
    
    if (mode > 7) {
        return GPIO_ERROR_INVALID_MODE;
    }
    
    /* Each GPFSEL register controls 10 pins (3 bits per pin) */
    uint8_t reg = pin / 10;
    uint8_t shift = (pin % 10) * 3;
    
    uint32_t value = gpio_state.gpio_map[reg];
    value &= ~(7 << shift);           /* Clear the 3 bits */
    value |= (mode << shift);         /* Set the new mode */
    gpio_state.gpio_map[reg] = value;
    
    return GPIO_OK;
}

gpio_error_t gpio_set_pull(uint8_t pin, gpio_pull_t pull) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!gpio_is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }
    
    /* BCM2711 (Pi 4) uses different pull-up/down mechanism */
    if (gpio_state.model == RPI_MODEL_4 || gpio_state.model == RPI_MODEL_5) {
        /* BCM2711 method: direct register access */
        uint8_t reg = GPPUPPDN0 + (pin / 16);
        uint8_t shift = (pin % 16) * 2;
        
        uint32_t value = gpio_state.gpio_map[reg];
        value &= ~(3 << shift);       /* Clear the 2 bits */
        value |= (pull << shift);     /* Set the new pull mode */
        gpio_state.gpio_map[reg] = value;
    } else {
        /* BCM2835/BCM2836/BCM2837 method: PUD + PUDCLK */
        gpio_state.gpio_map[GPPUD] = pull;
        usleep(10);  /* Wait for control signal to be set up */
        
        /* Clock the control signal into the pin */
        uint8_t reg = (pin < 32) ? GPPUDCLK0 : GPPUDCLK1;
        uint8_t shift = pin % 32;
        gpio_state.gpio_map[reg] = (1 << shift);
        usleep(10);
        
        /* Remove the control signal and clock */
        gpio_state.gpio_map[GPPUD] = 0;
        gpio_state.gpio_map[reg] = 0;
    }
    
    return GPIO_OK;
}

/* Digital I/O */
gpio_error_t gpio_write(uint8_t pin, bool value) {
    if (value) {
        return gpio_set(pin);
    } else {
        return gpio_clear(pin);
    }
}

gpio_error_t gpio_set(uint8_t pin) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!gpio_is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }
    
    /* Use GPSET register to set pin high */
    uint8_t reg = (pin < 32) ? GPSET0 : GPSET1;
    uint8_t shift = pin % 32;
    gpio_state.gpio_map[reg] = (1 << shift);
    
    return GPIO_OK;
}

gpio_error_t gpio_clear(uint8_t pin) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!gpio_is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }
    
    /* Use GPCLR register to set pin low */
    uint8_t reg = (pin < 32) ? GPCLR0 : GPCLR1;
    uint8_t shift = pin % 32;
    gpio_state.gpio_map[reg] = (1 << shift);
    
    return GPIO_OK;
}

gpio_error_t gpio_toggle(uint8_t pin) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!gpio_is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }
    
    /* Read current state and toggle */
    bool current_value;
    gpio_error_t err = gpio_read(pin, &current_value);
    if (err != GPIO_OK) {
        return err;
    }
    
    return gpio_write(pin, !current_value);
}

gpio_error_t gpio_read(uint8_t pin, bool *value) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!gpio_is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }
    
    if (value == NULL) {
        return GPIO_ERROR_INVALID_VALUE;
    }
    
    /* Use GPLEV register to read pin level */
    uint8_t reg = (pin < 32) ? GPLEV0 : GPLEV1;
    uint8_t shift = pin % 32;
    uint32_t level = gpio_state.gpio_map[reg];
    
    *value = (level & (1 << shift)) ? true : false;
    
    return GPIO_OK;
}

/* Batch Operations */
gpio_error_t gpio_mask_set(uint32_t mask) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    gpio_state.gpio_map[GPSET0] = mask;
    
    return GPIO_OK;
}

gpio_error_t gpio_mask_clear(uint32_t mask) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    gpio_state.gpio_map[GPCLR0] = mask;
    
    return GPIO_OK;
}

uint32_t gpio_mask_read(void) {
    if (!gpio_state.initialized) {
        return 0;
    }
    
    return gpio_state.gpio_map[GPLEV0];
}

gpio_error_t gpio_get_mode(uint8_t pin, gpio_mode_t *mode) {
    if (!gpio_state.initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!gpio_is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }
    
    if (mode == NULL) {
        return GPIO_ERROR_INVALID_VALUE;
    }
    
    /* Each GPFSEL register controls 10 pins (3 bits per pin) */
    uint8_t reg = pin / 10;
    uint8_t shift = (pin % 10) * 3;
    
    uint32_t value = gpio_state.gpio_map[reg];
    *mode = (gpio_mode_t)((value >> shift) & 7);
    
    return GPIO_OK;
}

/* Forth Word Implementations */

static void set_gpio_error(rforth_ctx_t *ctx, gpio_error_t err, const char *operation) {
    if (err != GPIO_OK) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s: %s", operation, gpio_error_string(err));
        rforth_set_error(ctx, RFORTH_ERROR_IO_FAILURE, msg, "gpio", __FILE__, __LINE__, 0);
    }
}

void builtin_gpio_init(rforth_ctx_t *ctx) {
    /* GPIO-INIT ( -- ) Initialize GPIO subsystem */
    gpio_error_t err = gpio_init();
    if (err == GPIO_OK) {
        printf("GPIO initialized successfully\n");
    } else {
        set_gpio_error(ctx, err, "GPIO-INIT");
    }
}

void builtin_gpio_close(rforth_ctx_t *ctx) {
    /* GPIO-CLOSE ( -- ) Cleanup GPIO subsystem */
    gpio_error_t err = gpio_cleanup();
    if (err == GPIO_OK) {
        printf("GPIO closed successfully\n");
    } else {
        set_gpio_error(ctx, err, "GPIO-CLOSE");
    }
}

void builtin_gpio_output(rforth_ctx_t *ctx) {
    /* GPIO-OUTPUT ( pin -- ) Set pin as output */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-OUTPUT requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_mode((uint8_t)pin_cell.value.i, GPIO_MODE_OUTPUT);
    set_gpio_error(ctx, err, "GPIO-OUTPUT");
}

void builtin_gpio_input(rforth_ctx_t *ctx) {
    /* GPIO-INPUT ( pin -- ) Set pin as input */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-INPUT requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_mode((uint8_t)pin_cell.value.i, GPIO_MODE_INPUT);
    set_gpio_error(ctx, err, "GPIO-INPUT");
}

void builtin_gpio_alt0(rforth_ctx_t *ctx) {
    /* GPIO-ALT0 ( pin -- ) Set pin to alternate function 0 */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-ALT0 requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_mode((uint8_t)pin_cell.value.i, GPIO_MODE_ALT0);
    set_gpio_error(ctx, err, "GPIO-ALT0");
}

void builtin_gpio_alt1(rforth_ctx_t *ctx) {
    /* GPIO-ALT1 ( pin -- ) Set pin to alternate function 1 */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-ALT1 requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_mode((uint8_t)pin_cell.value.i, GPIO_MODE_ALT1);
    set_gpio_error(ctx, err, "GPIO-ALT1");
}

void builtin_gpio_alt2(rforth_ctx_t *ctx) {
    /* GPIO-ALT2 ( pin -- ) Set pin to alternate function 2 */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-ALT2 requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_mode((uint8_t)pin_cell.value.i, GPIO_MODE_ALT2);
    set_gpio_error(ctx, err, "GPIO-ALT2");
}

void builtin_gpio_alt3(rforth_ctx_t *ctx) {
    /* GPIO-ALT3 ( pin -- ) Set pin to alternate function 3 */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-ALT3 requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_mode((uint8_t)pin_cell.value.i, GPIO_MODE_ALT3);
    set_gpio_error(ctx, err, "GPIO-ALT3");
}

void builtin_gpio_alt4(rforth_ctx_t *ctx) {
    /* GPIO-ALT4 ( pin -- ) Set pin to alternate function 4 */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-ALT4 requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_mode((uint8_t)pin_cell.value.i, GPIO_MODE_ALT4);
    set_gpio_error(ctx, err, "GPIO-ALT4");
}

void builtin_gpio_alt5(rforth_ctx_t *ctx) {
    /* GPIO-ALT5 ( pin -- ) Set pin to alternate function 5 */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-ALT5 requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_mode((uint8_t)pin_cell.value.i, GPIO_MODE_ALT5);
    set_gpio_error(ctx, err, "GPIO-ALT5");
}

void builtin_gpio_pull_up(rforth_ctx_t *ctx) {
    /* GPIO-PULL-UP ( pin -- ) Enable pull-up resistor */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-PULL-UP requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_pull((uint8_t)pin_cell.value.i, GPIO_PULL_UP);
    set_gpio_error(ctx, err, "GPIO-PULL-UP");
}

void builtin_gpio_pull_down(rforth_ctx_t *ctx) {
    /* GPIO-PULL-DOWN ( pin -- ) Enable pull-down resistor */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-PULL-DOWN requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_pull((uint8_t)pin_cell.value.i, GPIO_PULL_DOWN);
    set_gpio_error(ctx, err, "GPIO-PULL-DOWN");
}

void builtin_gpio_pull_off(rforth_ctx_t *ctx) {
    /* GPIO-PULL-OFF ( pin -- ) Disable pull resistors */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-PULL-OFF requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set_pull((uint8_t)pin_cell.value.i, GPIO_PULL_OFF);
    set_gpio_error(ctx, err, "GPIO-PULL-OFF");
}

void builtin_gpio_set(rforth_ctx_t *ctx) {
    /* GPIO-SET ( pin -- ) Set pin HIGH */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-SET requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_set((uint8_t)pin_cell.value.i);
    set_gpio_error(ctx, err, "GPIO-SET");
}

void builtin_gpio_clr(rforth_ctx_t *ctx) {
    /* GPIO-CLR ( pin -- ) Set pin LOW */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-CLR requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_clear((uint8_t)pin_cell.value.i);
    set_gpio_error(ctx, err, "GPIO-CLR");
}

void builtin_gpio_write(rforth_ctx_t *ctx) {
    /* GPIO-WRITE ( value pin -- ) Write boolean value to pin */
    cell_t pin_cell, value_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT ||
        !stack_pop(ctx->data_stack, &value_cell) || value_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-WRITE requires value and pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_write((uint8_t)pin_cell.value.i, value_cell.value.i != 0);
    set_gpio_error(ctx, err, "GPIO-WRITE");
}

void builtin_gpio_read(rforth_ctx_t *ctx) {
    /* GPIO-READ ( pin -- value ) Read pin state */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-READ requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    bool value;
    gpio_error_t err = gpio_read((uint8_t)pin_cell.value.i, &value);
    if (err == GPIO_OK) {
        stack_push_int(ctx->data_stack, value ? 1 : 0);
    } else {
        set_gpio_error(ctx, err, "GPIO-READ");
    }
}

void builtin_gpio_toggle(rforth_ctx_t *ctx) {
    /* GPIO-TOGGLE ( pin -- ) Toggle pin state */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-TOGGLE requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_toggle((uint8_t)pin_cell.value.i);
    set_gpio_error(ctx, err, "GPIO-TOGGLE");
}

void builtin_gpio_mask_set(rforth_ctx_t *ctx) {
    /* GPIO-MASK-SET ( mask -- ) Set multiple pins using bitmask */
    cell_t mask_cell;
    if (!stack_pop(ctx->data_stack, &mask_cell) || mask_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-MASK-SET requires bitmask", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_mask_set((uint32_t)mask_cell.value.i);
    set_gpio_error(ctx, err, "GPIO-MASK-SET");
}

void builtin_gpio_mask_clr(rforth_ctx_t *ctx) {
    /* GPIO-MASK-CLR ( mask -- ) Clear multiple pins using bitmask */
    cell_t mask_cell;
    if (!stack_pop(ctx->data_stack, &mask_cell) || mask_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-MASK-CLR requires bitmask", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    gpio_error_t err = gpio_mask_clear((uint32_t)mask_cell.value.i);
    set_gpio_error(ctx, err, "GPIO-MASK-CLR");
}

void builtin_gpio_mask_read(rforth_ctx_t *ctx) {
    /* GPIO-MASK-READ ( -- mask ) Read all GPIO states as bitmask */
    uint32_t mask = gpio_mask_read();
    stack_push_int(ctx->data_stack, (int64_t)mask);
}

void builtin_gpio_valid_q(rforth_ctx_t *ctx) {
    /* GPIO-VALID? ( pin -- flag ) Check if pin number is valid */
    cell_t pin_cell;
    if (!stack_pop(ctx->data_stack, &pin_cell) || pin_cell.type != CELL_INT) {
        rforth_set_error(ctx, RFORTH_ERROR_STACK_UNDERFLOW, 
                        "GPIO-VALID? requires pin number", "gpio", __FILE__, __LINE__, 0);
        return;
    }
    
    bool valid = gpio_is_valid_pin((uint8_t)pin_cell.value.i);
    stack_push_int(ctx->data_stack, valid ? -1 : 0);  /* Forth true is -1 */
}
