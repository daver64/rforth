# RForth Examples

This directory contains example Forth programs demonstrating various features of the RForth interpreter and compiler.

## Directory Structure

- `basic/` - Simple examples demonstrating basic Forth concepts
- `algorithms/` - More complex algorithms and mathematical operations

## Running Examples

### Interpreter Mode
```bash
# Run directly
./bin/rforth examples/basic/hello.f

# Interactive interpretation
./bin/rforth -i examples/basic/demo.f
```

### Compiler Mode
```bash
# Compile to executable
./bin/rforth -c examples/basic/hello.f -o hello
./hello

# Compile algorithm examples
./bin/rforth -c examples/algorithms/fibonacci.f -o fibonacci
./fibonacci
```

### REPL Mode
```bash
# Start interactive REPL
./bin/rforth -r

# Then type Forth code interactively
RForth> 5 3 + .
8 ok
RForth> : SQUARE DUP * ;
ok
RForth> 7 SQUARE .
49 ok
```