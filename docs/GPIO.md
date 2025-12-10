# Raspberry Pi GPIO Support for RForth

RForth now includes comprehensive GPIO support for Raspberry Pi hardware, including the Pi Zero 2W. This enables direct hardware control from Forth programs.

## Features

- **Direct GPIO Control**: Memory-mapped register access for fast I/O
- **Pin Configuration**: Set pins as input/output with pull-up/pull-down resistors
- **Digital I/O**: Read and write pin states
- **Batch Operations**: Control multiple pins simultaneously with bitmasks
- **Timing Functions**: Precise delays and time measurement
- **Auto-detection**: Automatically detects Pi model and configures appropriately

## Compatibility

Supported Raspberry Pi models:
- Raspberry Pi Zero
- Raspberry Pi Zero 2W
- Raspberry Pi 1, 2, 3, 4
- Compatible with BCM2835, BCM2836, BCM2837, and BCM2711 SoCs

## GPIO Forth Words Reference

### Initialization

#### `GPIO-INIT` `( -- )`
Initialize the GPIO subsystem. Must be called before any other GPIO operations.
Maps GPIO registers into memory and detects the Raspberry Pi model.

```forth
GPIO-INIT
```

#### `GPIO-CLOSE` `( -- )`
Cleanup and release GPIO resources. Should be called before program exit.

```forth
GPIO-CLOSE
```

### Pin Configuration

#### `GPIO-OUTPUT` `( pin -- )`
Configure a GPIO pin as an output.

```forth
17 GPIO-OUTPUT    \ Set GPIO17 as output
```

#### `GPIO-INPUT` `( pin -- )`
Configure a GPIO pin as an input.

```forth
27 GPIO-INPUT     \ Set GPIO27 as input
```

#### `GPIO-ALT0` through `GPIO-ALT5` `( pin -- )`
Set pin to alternate function (0-5). Used for SPI, I2C, UART, PWM, etc.

```forth
18 GPIO-ALT5      \ Set GPIO18 to ALT5 (PWM)
```

#### `GPIO-PULL-UP` `( pin -- )`
Enable internal pull-up resistor on a pin (typically 50kΩ).

```forth
27 GPIO-PULL-UP   \ Enable pull-up on GPIO27
```

#### `GPIO-PULL-DOWN` `( pin -- )`
Enable internal pull-down resistor on a pin.

```forth
17 GPIO-PULL-DOWN \ Enable pull-down on GPIO17
```

#### `GPIO-PULL-OFF` `( pin -- )`
Disable pull-up/pull-down resistors on a pin.

```forth
17 GPIO-PULL-OFF  \ Disable pull resistors
```

### Digital I/O

#### `GPIO-SET` `( pin -- )`
Set a GPIO pin HIGH (3.3V).

```forth
17 GPIO-SET       \ Set GPIO17 high
```

#### `GPIO-CLR` `( pin -- )`
Set a GPIO pin LOW (0V).

```forth
17 GPIO-CLR       \ Set GPIO17 low
```

#### `GPIO-WRITE` `( value pin -- )`
Write a boolean value to a pin (0 = LOW, non-zero = HIGH).

```forth
1 17 GPIO-WRITE   \ Set GPIO17 high
0 17 GPIO-WRITE   \ Set GPIO17 low
```

#### `GPIO-READ` `( pin -- value )`
Read the current state of a pin. Returns 1 for HIGH, 0 for LOW.

```forth
27 GPIO-READ      \ Read GPIO27, pushes 0 or 1
IF ." High" ELSE ." Low" THEN
```

#### `GPIO-TOGGLE` `( pin -- )`
Toggle the current state of a pin.

```forth
17 GPIO-TOGGLE    \ Toggle GPIO17
```

### Batch Operations

#### `GPIO-MASK-SET` `( mask -- )`
Set multiple pins at once using a 32-bit bitmask.

```forth
0x03 GPIO-MASK-SET   \ Set GPIO0 and GPIO1
```

#### `GPIO-MASK-CLR` `( mask -- )`
Clear multiple pins at once using a 32-bit bitmask.

```forth
0x03 GPIO-MASK-CLR   \ Clear GPIO0 and GPIO1
```

#### `GPIO-MASK-READ` `( -- mask )`
Read all GPIO pin states as a 32-bit bitmask.

```forth
GPIO-MASK-READ    \ Returns current state of all pins
```

### Timing Functions

#### `DELAY-MS` `( ms -- )`
Delay execution for the specified number of milliseconds.

```forth
1000 DELAY-MS     \ Wait 1 second
```

#### `DELAY-US` `( us -- )`
Delay execution for the specified number of microseconds.

```forth
500 DELAY-US      \ Wait 500 microseconds
```

#### `MILLIS` `( -- ms )`
Get milliseconds since timing initialization.

```forth
MILLIS .          \ Print milliseconds elapsed
```

#### `MICROS` `( -- us )`
Get microseconds since timing initialization.

```forth
MICROS .          \ Print microseconds elapsed
```

### Utility

#### `GPIO-VALID?` `( pin -- flag )`
Check if a pin number is valid for the current Pi model.
Returns -1 (true) if valid, 0 (false) if invalid.

```forth
17 GPIO-VALID? IF ." Valid pin" THEN
99 GPIO-VALID? IF ." Valid" ELSE ." Invalid pin" THEN
```

## Setup and Permissions

### Option 1: Add User to GPIO Group (Recommended)

```bash
sudo usermod -a -G gpio $USER
```

Log out and log back in for changes to take effect. This allows access to `/dev/gpiomem` without root privileges.

### Option 2: Run as Root

```bash
sudo ./bin/rforth examples/rpi/blink.f
```

## Building with GPIO Support

The GPIO support is automatically included when you build RForth:

```bash
cd rforth
cmake -B build
cmake --build build
```

The build system will compile the GPIO and timing modules and link them with the main executable.

## Example Programs

See the `examples/rpi/` directory for complete examples:

- **blink.f** - Simple LED blink
- **button.f** - Button input controlling LED
- **toggle.f** - Toggle LED state
- **fade.f** - Software PWM LED fade
- **traffic.f** - Traffic light simulation
- **morse.f** - Morse code output

## Quick Start Example

```forth
\ Simple LED blink on GPIO17
GPIO-INIT
17 GPIO-OUTPUT

: BLINK
  10 0 DO
    17 GPIO-SET
    500 DELAY-MS
    17 GPIO-CLR
    500 DELAY-MS
  LOOP ;

BLINK
GPIO-CLOSE
```

## Hardware Considerations

### Voltage Levels
- GPIO pins operate at **3.3V logic**
- **Never connect 5V directly** to GPIO pins
- Maximum current per pin: 16mA (keep well below this)

### LED Connections
Always use a current-limiting resistor (220Ω - 1kΩ) with LEDs:

```
GPIO Pin → LED (long leg) → LED (short leg) → Resistor → GND
```

### Button Connections
Use internal pull-up resistor for buttons:

```
GPIO Pin (with pull-up enabled)
    |
  Button
    |
   GND
```

### Pin Numbering
RForth uses **BCM GPIO numbering**, not physical pin numbers.
Consult a pinout diagram for your specific Pi model.

## Error Handling

GPIO operations will set RForth error codes on failure:
- Invalid pin numbers
- Operations on uninitialized GPIO
- Memory mapping failures
- Permission denied

Always check for errors in production code:

```forth
GPIO-INIT
\ Check if initialization succeeded
\ ... rest of code ...
GPIO-CLOSE
```

## Performance

GPIO operations are very fast:
- Direct memory-mapped register access
- No system calls during I/O
- Pin writes: ~100ns
- Pin reads: ~100ns

For timing-critical applications, this is suitable for bitbanging protocols up to several MHz.

## Limitations

- Not thread-safe (document this if using with threads)
- Software PWM only (hardware PWM coming in future version)
- No SPI/I2C high-level interfaces yet (coming soon)
- GPIO pins only (no ADC/DAC support)

## Advanced Topics

### Memory-Mapped I/O
The GPIO implementation uses direct memory mapping of BCM peripheral registers via `/dev/gpiomem` or `/dev/mem`. This provides maximum performance with minimal overhead.

### Platform Detection
The code automatically detects:
- Pi model from `/proc/device-tree/model`
- Correct peripheral base address for each BCM chip
- Valid GPIO pin range for the model

### Alternate Functions
GPIO pins can be configured for alternate functions:
- ALT0: UART, PCM, etc.
- ALT1-ALT5: Various peripheral functions
- See BCM datasheet for pin function mapping

## Troubleshooting

### "Permission denied" error
- Add user to gpio group: `sudo usermod -a -G gpio $USER`
- Or run with sudo
- Check `/dev/gpiomem` permissions

### "Not running on Raspberry Pi" error
- You're not on a Raspberry Pi
- The code will fail gracefully with an error message

### Pin doesn't respond
- Verify pin number (BCM numbering)
- Check physical connections
- Verify voltage levels (3.3V)
- Use a multimeter to check pin voltage

## Contributing

GPIO support is actively being developed. Planned features:
- Hardware PWM support
- SPI high-level interface
- I2C high-level interface  
- UART serial interface
- Interrupt support
- Event detection

## References

- [BCM2835 ARM Peripherals](https://www.raspberrypi.org/documentation/hardware/raspberrypi/)
- [Raspberry Pi GPIO Pinout](https://pinout.xyz/)
- [RForth Documentation](../docs/api.md)

---

For questions or issues, please report to the RForth GitHub repository.
