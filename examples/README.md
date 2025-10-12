# RForth Examples

🏠 **[← Back to Main README](../README.md)**

This directory contains example Forth programs demonstrating various features of the RForth interpreter and compiler.

## Directory Structure

- **[`basic/`](basic/README.md)** - Simple examples demonstrating basic Forth concepts
- **[`algorithms/`](algorithms/README.md)** - More complex algorithms and mathematical operations

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
RForth> : square dup * ;
ok
RForth> 7 square .
49 ok
RForth> bye
```

## Quick Test

Try running these examples to test your RForth installation:

```bash
# Basic examples
./bin/rforth examples/basic/hello.f        # Should output: Hello World! 8 100
./bin/rforth examples/basic/square.f       # Should output: 25 49

# Algorithm examples  
./bin/rforth examples/algorithms/fibonacci.f  # Should output: 22 12

# Compilation test
./bin/rforth -c examples/basic/hello.f -o hello_test
./hello_test                               # Should match interpreter output
rm hello_test                             # Clean up
```