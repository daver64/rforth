# Missing Advanced Forth Words Analysis

## Current RForth Implementation (25 words)

### ✅ **Implemented Core Words**
**Arithmetic (5)**: `+`, `-`, `*`, `/`, `mod`  
**Stack Operations (6)**: `dup`, `drop`, `swap`, `over`, `rot`, `negate`  
**Comparison (3)**: `=`, `<`, `>`  
**Logical (3)**: `and`, `or`, `not`  
**I/O (4)**: `.`, `emit`, `cr`, `space`  
**System (4)**: `words`, `.s`, `bye`, `turnkey`  

## 🚫 **Missing Advanced Forth Words by Category**

### **Control Flow (CRITICAL - Most Important Missing Feature)**

#### Conditionals
- `if` ... `then` - Basic conditional execution
- `if` ... `else` ... `then` - Conditional with alternative branch
- `case` ... `of` ... `endof` ... `endcase` - Multi-way branching

#### Loops
- `begin` ... `until` - Post-test loop (loop until condition true)
- `begin` ... `while` ... `repeat` - Pre-test loop with condition
- `do` ... `loop` - Counted loop with automatic increment
- `do` ... `+loop` - Counted loop with custom increment
- `leave` - Exit loop early
- `unloop` - Clean up loop parameters

### **Advanced Stack Operations**

#### Extended Stack Manipulation
- `pick` - Copy nth stack item to top `( n -- x )`
- `roll` - Move nth stack item to top `( n -- )`
- `2dup` - Duplicate top two items `( a b -- a b a b )`
- `2drop` - Drop top two items `( a b -- )`
- `2swap` - Swap top two pairs `( a b c d -- c d a b )`
- `2over` - Copy second pair to top `( a b c d -- a b c d a b )`
- `depth` - Get stack depth `( -- n )`

#### Return Stack Operations
- `>r` - Move to return stack `( n -- ) ( R: -- n )`
- `r>` - Move from return stack `( -- n ) ( R: n -- )`
- `r@` - Copy from return stack `( -- n ) ( R: n -- n )`

### **Memory and Variable Operations**

#### Variables and Constants
- `variable` - Create a variable `variable name`
- `constant` - Create a constant `42 constant answer`
- `@` - Fetch from address `( addr -- n )`
- `!` - Store to address `( n addr -- )`
- `+!` - Add to memory location `( n addr -- )`
- `?` - Fetch and print `( addr -- )`

#### Memory Access
- `c@` - Fetch byte `( c-addr -- char )`
- `c!` - Store byte `( char c-addr -- )`
- `w@` - Fetch 16-bit word `( w-addr -- w )`
- `w!` - Store 16-bit word `( w w-addr -- )`
- `2@` - Fetch two cells `( addr -- x1 x2 )`
- `2!` - Store two cells `( x1 x2 addr -- )`

#### Memory Allocation
- `allot` - Allocate memory `( n -- )`
- `here` - Get current memory pointer `( -- addr )`
- `unused` - Get available memory `( -- n )`

### **String Operations**

#### String Manipulation
- `s"` - Create string literal `s" hello world"`
- `."` - Print string literal `." Hello World"`
- `count` - Get string length `( c-addr -- c-addr+1 n )`
- `type` - Print counted string `( c-addr n -- )`
- `compare` - Compare strings `( c-addr1 n1 c-addr2 n2 -- flag )`

#### String Utilities
- `blank` - Fill with spaces `( c-addr n -- )`
- `erase` - Fill with zeros `( addr n -- )`
- `fill` - Fill with character `( c-addr n char -- )`
- `move` - Copy memory `( addr1 addr2 n -- )`

### **Advanced Arithmetic**

#### Extended Math
- `abs` - Absolute value `( n -- |n| )`
- `max` - Maximum of two values `( n1 n2 -- n )`
- `min` - Minimum of two values `( n1 n2 -- n )`
- `negate` - Change sign *(already implemented)*
- `1+` - Increment `( n -- n+1 )`
- `1-` - Decrement `( n -- n-1 )`
- `2*` - Multiply by 2 `( n -- n*2 )`
- `2/` - Divide by 2 `( n -- n/2 )`

#### Double Number Operations
- `d+` - Double number addition `( d1 d2 -- d3 )`
- `d-` - Double number subtraction `( d1 d2 -- d3 )`
- `d*` - Double number multiplication `( d1 d2 -- d3 )`
- `d/` - Double number division `( d1 d2 -- d3 )`

#### Mixed Precision
- `*/` - Multiply then divide `( n1 n2 n3 -- n1*n2/n3 )`
- `*/mod` - Multiply then divide with remainder `( n1 n2 n3 -- rem quot )`
- `fm/mod` - Floored divide with remainder `( d n -- rem quot )`

### **I/O and Formatting**

#### Numeric Output
- `.r` - Print number right-justified `( n width -- )`
- `u.` - Print unsigned number `( u -- )`
- `u.r` - Print unsigned right-justified `( u width -- )`
- `hex` - Set hexadecimal base
- `decimal` - Set decimal base
- `octal` - Set octal base
- `base` - Variable holding current base

#### Character I/O
- `key` - Read character from input `( -- char )`
- `key?` - Check if character available `( -- flag )`
- `expect` - Read line of input `( c-addr n -- )`
- `span` - Variable holding input length

#### Advanced Output
- `page` - Clear screen
- `at-xy` - Position cursor `( col row -- )`
- `spaces` - Print n spaces `( n -- )`

### **Compilation and Meta-Operations**

#### Word Definition
- `create` - Create a new word header
- `does>` - Define runtime behavior
- `immediate` - Mark word as immediate
- `compile,` - Compile xt into current definition
- `literal` - Compile a literal
- `postpone` - Compile or execute word

#### Dictionary Inspection
- `find` - Find word in dictionary `( c-addr -- c-addr 0 | xt 1 | xt -1 )`
- `>name` - Get name from execution token `( xt -- nt )`
- `name>` - Get execution token from name `( nt -- xt )`
- `see` - Decompile a word

#### Execution Control
- `execute` - Execute word by xt `( xt -- )`
- `catch` - Execute with exception handling `( xt -- ... 0 | ... error )`
- `throw` - Throw exception `( error -- )`

### **File Operations**

#### File I/O
- `open-file` - Open file `( c-addr n fam -- fileid ior )`
- `close-file` - Close file `( fileid -- ior )`
- `read-file` - Read from file `( c-addr n fileid -- n ior )`
- `write-file` - Write to file `( c-addr n fileid -- ior )`
- `file-position` - Get file position `( fileid -- pos ior )`
- `reposition-file` - Set file position `( pos fileid -- ior )`

#### File Utilities
- `include` - Include another file
- `included` - Include file by name `( c-addr n -- )`
- `save-buffers` - Save all buffers

### **Environmental and System Words**

#### System Information
- `environment?` - Query environment `( c-addr n -- false | ... true )`
- `pad` - Get scratch area address `( -- c-addr )`
- `tib` - Terminal input buffer address `( -- c-addr )`
- `>in` - Input buffer pointer variable

#### Time and Date
- `time&date` - Get current time `( -- sec min hour day month year )`
- `ms` - Millisecond delay `( n -- )`

## 📋 **Implementation Priority Ranking**

### **HIGHEST Priority (Essential for Usability)**
1. **Control Flow**: `if/then/else`, `begin/until/while/repeat`
2. **Variables**: `variable`, `constant`, `@`, `!`
3. **Return Stack**: `>r`, `r>`, `r@`
4. **Basic Math**: `abs`, `max`, `min`, `1+`, `1-`

### **HIGH Priority (Important for Productivity)**
1. **Advanced Stack**: `pick`, `roll`, `2dup`, `2drop`, `depth`
2. **String I/O**: `s"`, `."`, `type`
3. **Memory**: `allot`, `here`, `c@`, `c!`
4. **Loops**: `do/loop`, `leave`

### **MEDIUM Priority (Nice to Have)**
1. **Double Numbers**: `d+`, `d-`, `d*`, `d/`
2. **Mixed Precision**: `*/`, `*/mod`
3. **Advanced I/O**: `.r`, `u.`, `hex`, `decimal`
4. **File Operations**: Basic file I/O

### **LOWER Priority (Advanced Features)**
1. **Meta-compilation**: `create`, `does>`, `immediate`
2. **Exception handling**: `catch`, `throw`
3. **Environmental queries**: `environment?`
4. **Time/Date**: `time&date`, `ms`

## 🎯 **Recommended Implementation Plan**

### **Phase 1: Essential Control Flow** *(1-2 weeks)*
Implement `if/then/else` and `begin/until/while/repeat` with proper compilation support.

### **Phase 2: Variables and Memory** *(1-2 weeks)*  
Add `variable`, `constant`, `@`, `!`, and return stack operations.

### **Phase 3: Enhanced Stack and Math** *(1 week)*
Implement advanced stack operations and missing arithmetic words.

### **Phase 4: String Handling** *(1-2 weeks)*
Add string literals, printing, and basic string manipulation.

### **Phase 5: Advanced Features** *(2-4 weeks)*
File I/O, double numbers, and meta-compilation features.

## 💡 **Impact Assessment**

**Current State**: RForth is a functional calculator with basic word definition capability.

**After Phase 1-2**: RForth becomes a real programming language suitable for:
- Simple algorithms and utilities
- Educational use  
- Embedded scripting

**After Phase 3-4**: RForth becomes competitive with other Forth implementations for:
- Real applications
- Data processing
- System utilities

**After Phase 5**: RForth becomes a full-featured Forth system suitable for:
- Professional development
- Complex applications
- ANS Forth compatibility