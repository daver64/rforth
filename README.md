# RForth - A Complete Forth Interpreter and Compiler

RForth is a modern, feature-complete implementation of the Forth programming language written in C99. It provides both an interactive interpreter and a powerful compiler that generates native executables.

## Features

- **Three Operation Modes**:
  - 🔄 **REPL Mode**: Interactive Forth interpreter with immediate feedback
  - 📖 **Interpret Mode**: Execute Forth files directly
  - ⚡ **Compile Mode**: Generate native executables via C compilation

- **Complete Forth Implementation**:
  - Stack-based execution model with 256-element stack
  - User-defined words with colon syntax (`: WORD ... ;`)
  - All standard arithmetic and stack manipulation operations
  - I/O operations (`EMIT`, `CR`, `.`)
  - Support for comments `( ... )`

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

## Forth Language Support

### Data Types
- **Integers**: 64-bit signed integers
- **Strings**: Character sequences via `emit`

### Stack Operations
- `dup` - Duplicate top of stack
- `drop` - Remove top of stack  
- `swap` - Exchange top two items

### Arithmetic
- `+` `-` `*` `/` - Basic arithmetic operations

### I/O Operations
- `.` - Print top of stack as number
- `emit` - Print top of stack as ASCII character
- `cr` - Print carriage return

### Word Definition
```forth
: word-name
    ( word definition here )
;
```

### Comments
```forth
( This is a comment )
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
