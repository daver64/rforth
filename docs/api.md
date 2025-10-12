# RForth API Reference

## Builtin Words

### Stack Operations
- `dup` - Duplicate top of stack
- `drop` - Remove top of stack  
- `swap` - Swap top two stack items
- `over` - Copy second stack item to top
- `rot` - Rotate top three stack items
- `pick` - Copy nth stack item to top

### Arithmetic
- `+` - Add top two stack items
- `-` - Subtract second from top
- `*` - Multiply top two stack items  
- `/` - Divide second by top
- `mod` - Modulo operation
- `abs` - Absolute value

### Comparison  
- `=` - Equal comparison
- `<` - Less than comparison
- `>` - Greater than comparison

### I/O Operations
- `emit` - Output character from stack
- `cr` - Output carriage return
- `.` - Output number from stack

### Control Flow
- `if` ... `then` - Conditional execution
- `if` ... `else` ... `then` - Conditional with alternative
- `begin` ... `until` - Loop until condition true
- `begin` ... `while` ... `repeat` - Loop while condition true

### System Words
- `words` - List all defined words
- `bye` - Exit interpreter
- `turnkey` - Create standalone executable

## Programming Interface

### Word Definition
```forth
: word-name
  word-body
;
```

### Variables and Constants
```forth
42 constant answer
variable counter
```

### Comments
```forth
\ This is a line comment
( This is a block comment )
```