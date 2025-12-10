# Raspberry Pi GPIO Examples

This directory contains Forth programs demonstrating GPIO control on Raspberry Pi Zero 2W (and other compatible Pi models).

## Prerequisites

To use GPIO on Raspberry Pi, you need appropriate permissions. Either:
1. Run as root: `sudo ./bin/rforth examples/rpi/blink.f`
2. Add your user to the gpio group: `sudo usermod -a -G gpio $USER` (logout and login again)

## Hardware Setup

### LED Blink (`blink.f`)
- Connect an LED and 220Ω resistor between GPIO17 and GND
- Long leg of LED → GPIO17
- Short leg of LED → 220Ω resistor → GND

### Button Control (`button.f`)
- Connect a button between GPIO27 and GND
- LED on GPIO17 as above
- Button uses internal pull-up resistor

### PWM Fade (`fade.f`)
- Connect an LED and 220Ω resistor between GPIO18 and GND
- Note: Basic timing-based PWM, not hardware PWM

## Running Examples

```bash
# Interpret mode (run directly)
./bin/rforth examples/rpi/blink.f

# Compile to executable
./bin/rforth -c examples/rpi/blink.f -o blink
./blink

# Interactive REPL with GPIO
./bin/rforth -r
```

## Example Programs

- **blink.f** - Simple LED blink
- **button.f** - Button input controlling LED
- **toggle.f** - Toggle LED state with GPIO-TOGGLE
- **fade.f** - Software PWM LED fade effect
- **traffic.f** - Traffic light simulation
- **morse.f** - Morse code output on LED

## GPIO Pin Reference (Pi Zero 2W)

Valid GPIO pins: 0-27

Common pins:
- GPIO2, GPIO3 - I2C (SDA, SCL)
- GPIO14, GPIO15 - UART (TX, RX)
- GPIO17 - Often used for LED
- GPIO18 - Hardware PWM capable
- GPIO27 - Often used for button input

Always check your Pi's pinout diagram before connecting hardware!
