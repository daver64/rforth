# RForth - Complete ANSI Forth Core Implementation

RForth is a modern, **100% ANSI Forth Core compliant** implementation of the Forth programming language written in C99. It provides both an interactive interpreter and a powerful compiler that generates native executables.

## 🎯 ANSI Forth Core Compliance - ACHIEVED

**✅ COMPLETE ANSI CORE COMPLIANCE (100%)**

RForth successfully implements **all 133 words** required by the ANSI Forth Core standard (ANSI X3.215-1994). Every word has been implemented, tested, and verified to work correctly:

- **✅ Complete Core Wordset**: All arithmetic, stack, comparison, and logical operations  
- **✅ Memory Operations**: Full cell and character memory access with proper alignment
- **✅ Numeric Formatting**: Complete pictured numeric output system (`<#`, `#`, `#S`, `#>`, `HOLD`, `SIGN`)
- **✅ Compilation Control**: Advanced compilation and interpretation state management
- **✅ Dictionary Access**: Word definition, lookup, and manipulation facilities  
- **✅ Character Operations**: Full character handling and processing
- **✅ String & I/O**: Complete string manipulation and input/output operations
- **✅ Variable System**: Named variables with proper dictionary integration
- **✅ Comment Support**: Both line (`\`) and block (`( ... )`) comments

This makes RForth **fully compatible** with all standard ANSI Forth Core programs.

## Features

- **Three Operation Modes**:
  - 🔄 **REPL Mode**: Interactive Forth interpreter with immediate feedback
  - 📖 **Interpret Mode**: Execute Forth files directly
  - ⚡ **Compile Mode**: Generate native executables via C compilation

- **Complete ANSI Forth Core Implementation**:
  - ✅ **133/133 ANSI Core words** implemented and working (100% compliance)
  - Stack-based execution model with comprehensive error handling
  - User-defined words with colon syntax (`: WORD ... ;`)
  - Complete arithmetic, logical, and comparison operations
  - Full memory operations with proper alignment support
  - Advanced numeric formatting system with pictured output
  - Dictionary manipulation and word lookup facilities
  - Complete I/O operations (`EMIT`, `CR`, `.`, `TYPE`, `ACCEPT`, `KEY`)
  - Named variable system with proper dictionary integration
  - Support for both line comments (`\`) and block comments (`( ... )`)

- **Raspberry Pi GPIO Support** 🍓:
  - Direct hardware control for Raspberry Pi (Zero, Zero 2W, 1-4)
  - Memory-mapped GPIO for fast I/O operations
  - Pin configuration (input/output, pull-up/pull-down)
  - Digital read/write operations
  - Timing functions (delays, microsecond/millisecond counters)
  - Complete set of GPIO Forth words
  - See [GPIO Documentation](docs/GPIO.md) for details

- **Advanced Compiler**:
  - Generates clean, readable C code
  - Handles hyphenated Forth identifiers (converts to valid C names)
  - Produces standalone executables with embedded runtime
  - Uses GCC backend for optimization

## Quick Start

### Building

```bash
git clone https://github.com/daver64/rforth.git
cd rforth
cmake -B build
cmake --build build
```

The executable will be created at `bin/rforth`.

### Usage

#### REPL Mode (Interactive)
```bash
./bin/rforth -r
```
```forth
RForth> 5 3 + .
8 ok
RForth> : square dup * ;
ok  
RForth> 7 square .
49 ok
RForth> bye
```

#### Interpret Mode
```bash
# Run a Forth file directly
./bin/rforth examples/basic/hello.f

# Explicit interpret mode
./bin/rforth -i examples/basic/demo.f
```

#### Compile Mode
```bash
# Compile to native executable
./bin/rforth -c examples/basic/hello.f -o hello
./hello

# Compile complex programs
./bin/rforth -c examples/algorithms/fibonacci.f -o fibonacci
./fibonacci
```

## Examples

📁 **[View All Examples →](examples/README.md)**

The `examples/` directory contains sample Forth programs:

- **[`basic/`](examples/basic/README.md)** - Simple examples for learning Forth basics
  - [`hello.f`](examples/basic/hello.f) - Hello World with character output
  - [`demo.f`](examples/basic/demo.f) - Basic arithmetic operations  
  - [`square.f`](examples/basic/square.f) - User-defined word demonstration
  - [`simple.f`](examples/basic/simple.f) - Minimal arithmetic example

- **[`algorithms/`](examples/algorithms/README.md)** - More complex programs
  - [`fibonacci.f`](examples/algorithms/fibonacci.f) - Demonstrates hyphenated word names and function composition

- **[`rpi/`](examples/rpi/README.md)** 🍓 - Raspberry Pi GPIO examples
  - [`blink.f`](examples/rpi/blink.f) - LED blink example
  - [`button.f`](examples/rpi/button.f) - Button input controlling LED
  - [`toggle.f`](examples/rpi/toggle.f) - Toggle LED state
  - [`fade.f`](examples/rpi/fade.f) - Software PWM fade effect
  - [`traffic.f`](examples/rpi/traffic.f) - Traffic light simulator
  - [`morse.f`](examples/rpi/morse.f) - Morse code output

## Forth Language Support

### ANSI Core Compliance ✅
RForth implements the complete **ANSI Forth Core standard** (133 words) including:

#### Data Types & Stack Operations
- **Integers**: 64-bit signed integers with overflow protection
- **Stack Manipulation**: `DUP` `DROP` `SWAP` `OVER` `ROT` `2DUP` `2DROP` `2SWAP` `PICK` `ROLL`
- **Return Stack**: `>R` `R>` `R@` with proper stack isolation

#### Complete Arithmetic Suite
- **Basic**: `+` `-` `*` `/` `MOD` `ABS` `NEGATE` `1+` `1-` `2*` `2/`
- **Advanced**: `*/` `*/MOD` `FM/MOD` `SM/REM` `M*` `UM*` `UM/MOD`
- **Bit Operations**: `AND` `OR` `XOR` `INVERT` `LSHIFT` `RSHIFT`
- **Comparisons**: `=` `<>` `<` `>` `<=` `>=` `0=` `0<` `0>` `U<`

#### Memory Operations
- **Cell Access**: `@` `!` `+!` `2@` `2!` with alignment support
- **Character Access**: `C@` `C!` with byte-level precision
- **Memory Management**: `ALLOT` `HERE` `ALIGN` `ALIGNED` `CELL+` `CELLS`
- **Block Operations**: `FILL` `MOVE` for efficient memory manipulation

#### Numeric Formatting System
- **Pictured Output**: `<#` `#` `#S` `#>` `HOLD` `SIGN` for custom number formatting
- **Conversion**: `S>D` for single-to-double conversion

#### String & I/O Operations
- **Output**: `.` `U.` `EMIT` `CR` `SPACE` `SPACES` `TYPE`
- **String Literals**: `S"` `."` with proper string handling
- **Input**: `KEY` `KEY?` `ACCEPT` for interactive input
- **Formatting**: Complete pictured numeric output support

#### Dictionary & Compilation
- **Word Definition**: `:` `;` with compilation state management
- **Immediate Words**: `IMMEDIATE` `[` `]` for meta-compilation
- **Compilation Control**: `LITERAL` `POSTPONE` `RECURSE` `EXIT`
- **Dictionary Access**: `'` `FIND` `>BODY` `>IN` `WORD` for runtime introspection

#### Character Operations
- **Character Handling**: `CHAR` `[CHAR]` `CHAR+` `CHARS` `COUNT`
- **ASCII Support**: Full character processing and manipulation

#### Advanced Features
- **Comments**: Line comments (`\`) and block comments (`( ... )`)
- **Environment**: `ENVIRONMENT?` for feature queries
- **Control Flow**: `IF` `THEN` `ELSE` `BEGIN` `UNTIL` `WHILE` `REPEAT` `DO` `LOOP` `+LOOP`
- **Variables**: `VARIABLE` `CONSTANT` `CREATE` `DOES>` for data structures

### Word Definition
```forth
: word-name
    ( word definition here )
;
```

### Comments
```forth
\ This is a line comment
( This is a block comment )

\ Line comments extend to end of line
42 .  \ Print 42

( Block comments can span
  multiple lines and are
  used for documentation )
```

## Architecture

RForth uses a modular C architecture:

- **`src/main.c`** - Command-line interface and mode handling
- **`src/interpreter.c`** - Core Forth execution engine  
- **`src/compiler.c`** - Forth-to-C compiler with code generation
- **`src/parser.c`** - Tokenizer for Forth source code
- **`src/dict.c`** - Word dictionary management
- **`src/stack.c`** - Stack operations implementation
- **`src/builtins.c`** - Built-in Forth words

## Command Line Options

```
Usage: ./bin/rforth [OPTIONS] [FILE]

Options:
  -h          Show help message
  -v          Show version information  
  -r          Start REPL mode
  -c          Compile mode (requires -o)
  -i FILE     Interpret FILE
  -o FILE     Output file for compile mode

Examples:
  ./bin/rforth -r                           # Start REPL
  ./bin/rforth examples/basic/hello.f       # Interpret hello.f
  ./bin/rforth -i examples/basic/demo.f     # Interpret demo.f  
  ./bin/rforth -c hello.f -o hello          # Compile to executable
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

📋 **[Development Roadmap & Analysis →](ANALYSIS.md)**

Contributions are welcome! Please feel free to submit a Pull Request. Check the [ANALYSIS.md](ANALYSIS.md) file for current development priorities and code quality assessment.

## ANSI Compliance Achievement

🎯 **RForth has achieved complete ANSI Forth Core compliance (100%)**

This implementation provides full compatibility with ANSI Forth Core programs and includes all 133 required words from the ANSI X3.215-1994 standard. The systematic implementation ensures robust error handling, proper memory management, and complete feature coverage.

**Achievement Summary:**
- ✅ **133/133 ANSI Core words** implemented and tested
- ✅ **Complete numeric formatting** system with `<#`, `#`, `#S`, `#>`, `HOLD`, `SIGN`
- ✅ **Full memory operations** with alignment (`@`, `!`, `+!`, `2@`, `2!`, `ALIGN`, etc.)
- ✅ **Advanced compilation features** (`LITERAL`, `POSTPONE`, `RECURSE`, `EXIT`)
- ✅ **Comprehensive character operations** (`CHAR`, `CHAR+`, `CHARS`, `COUNT`)
- ✅ **Complete string & I/O** operations (`TYPE`, `ACCEPT`, `KEY`, `KEY?`)
- ✅ **Working variable system** with named variables in dictionary
- ✅ **Full comment support** for both line (`\`) and block (`( ... )`) comments
- ✅ **Robust error handling** and comprehensive type safety

**Testing:** Run `./bin/rforth ansi_test.f` to execute comprehensive tests of all ANSI Core words.
