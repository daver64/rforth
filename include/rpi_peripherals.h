#ifndef RPI_PERIPHERALS_H
#define RPI_PERIPHERALS_H

#include <stdint.h>
#include <stdbool.h>

/* Raspberry Pi Peripheral Base Addresses */
/* These vary by Pi model - autodetected at runtime */

/* BCM2835 (Pi Zero, Pi 1) */
#define BCM2835_PERI_BASE    0x20000000
#define BCM2835_GPIO_BASE    (BCM2835_PERI_BASE + 0x200000)

/* BCM2836/BCM2837 (Pi 2, Pi 3, Pi Zero 2W) */
#define BCM2836_PERI_BASE    0x3F000000
#define BCM2836_GPIO_BASE    (BCM2836_PERI_BASE + 0x200000)

/* BCM2711 (Pi 4) */
#define BCM2711_PERI_BASE    0xFE000000
#define BCM2711_GPIO_BASE    (BCM2711_PERI_BASE + 0x200000)

/* GPIO Register Offsets */
#define GPFSEL0     0   /* GPIO Function Select 0 */
#define GPFSEL1     1   /* GPIO Function Select 1 */
#define GPFSEL2     2   /* GPIO Function Select 2 */
#define GPFSEL3     3   /* GPIO Function Select 3 */
#define GPFSEL4     4   /* GPIO Function Select 4 */
#define GPFSEL5     5   /* GPIO Function Select 5 */

#define GPSET0      7   /* GPIO Pin Output Set 0 */
#define GPSET1      8   /* GPIO Pin Output Set 1 */

#define GPCLR0      10  /* GPIO Pin Output Clear 0 */
#define GPCLR1      11  /* GPIO Pin Output Clear 1 */

#define GPLEV0      13  /* GPIO Pin Level 0 */
#define GPLEV1      14  /* GPIO Pin Level 1 */

#define GPEDS0      16  /* GPIO Pin Event Detect Status 0 */
#define GPEDS1      17  /* GPIO Pin Event Detect Status 1 */

#define GPREN0      19  /* GPIO Pin Rising Edge Detect Enable 0 */
#define GPREN1      20  /* GPIO Pin Rising Edge Detect Enable 1 */

#define GPFEN0      22  /* GPIO Pin Falling Edge Detect Enable 0 */
#define GPFEN1      23  /* GPIO Pin Falling Edge Detect Enable 1 */

#define GPHEN0      25  /* GPIO Pin High Detect Enable 0 */
#define GPHEN1      26  /* GPIO Pin High Detect Enable 1 */

#define GPLEN0      28  /* GPIO Pin Low Detect Enable 0 */
#define GPLEN1      29  /* GPIO Pin Low Detect Enable 1 */

#define GPAREN0     31  /* GPIO Pin Async Rising Edge Detect 0 */
#define GPAREN1     32  /* GPIO Pin Async Rising Edge Detect 1 */

#define GPAFEN0     34  /* GPIO Pin Async Falling Edge Detect 0 */
#define GPAFEN1     35  /* GPIO Pin Async Falling Edge Detect 1 */

#define GPPUD       37  /* GPIO Pin Pull-up/down Enable */
#define GPPUDCLK0   38  /* GPIO Pin Pull-up/down Enable Clock 0 */
#define GPPUDCLK1   39  /* GPIO Pin Pull-up/down Enable Clock 1 */

/* GPIO Pull Up/Down control values (BCM2835/BCM2836/BCM2837) */
#define GPIO_PUD_OFF    0   /* Disable pull-up/down */
#define GPIO_PUD_DOWN   1   /* Enable pull-down */
#define GPIO_PUD_UP     2   /* Enable pull-up */

/* GPIO Pull Up/Down control registers for BCM2711 */
#define GPPUPPDN0   57  /* GPIO Pull-up/down Register 0 (BCM2711) */
#define GPPUPPDN1   58  /* GPIO Pull-up/down Register 1 (BCM2711) */
#define GPPUPPDN2   59  /* GPIO Pull-up/down Register 2 (BCM2711) */
#define GPPUPPDN3   60  /* GPIO Pull-up/down Register 3 (BCM2711) */

/* GPIO Function Select values */
#define GPIO_FSEL_INPUT   0  /* GPIO Pin is an input */
#define GPIO_FSEL_OUTPUT  1  /* GPIO Pin is an output */
#define GPIO_FSEL_ALT0    4  /* GPIO Pin alternate function 0 */
#define GPIO_FSEL_ALT1    5  /* GPIO Pin alternate function 1 */
#define GPIO_FSEL_ALT2    6  /* GPIO Pin alternate function 2 */
#define GPIO_FSEL_ALT3    7  /* GPIO Pin alternate function 3 */
#define GPIO_FSEL_ALT4    3  /* GPIO Pin alternate function 4 */
#define GPIO_FSEL_ALT5    2  /* GPIO Pin alternate function 5 */

/* Memory page size */
#define PAGE_SIZE       4096
#define BLOCK_SIZE      4096

/* Valid GPIO pins for different Pi models */
#define RPI_GPIO_MIN    0
#define RPI_GPIO_MAX    27   /* Pi Zero has 0-27 */
#define RPI4_GPIO_MAX   53   /* Pi 4 has 0-53 */

/* Pi model detection */
typedef enum {
    RPI_MODEL_UNKNOWN = 0,
    RPI_MODEL_1,
    RPI_MODEL_ZERO,
    RPI_MODEL_ZERO_2W,
    RPI_MODEL_2,
    RPI_MODEL_3,
    RPI_MODEL_4,
    RPI_MODEL_5
} rpi_model_t;

/* Hardware detection */
rpi_model_t rpi_detect_model(void);
uint32_t rpi_get_peripheral_base(void);
const char* rpi_get_model_name(rpi_model_t model);

#endif /* RPI_PERIPHERALS_H */
