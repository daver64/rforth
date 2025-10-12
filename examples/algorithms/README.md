# Algorithm Examples

🏠 **[← Back to Main README](../../README.md)** | 📁 **[← Examples](../README.md)** | 📚 **[← Basic Examples](../basic/README.md)**

More complex Forth programs demonstrating algorithms and advanced programming techniques.

## Files

- **[`fibonacci.f`](fibonacci.f)** - Demonstrates user-defined words with hyphenated names and function composition

## Concepts Demonstrated

- Hyphenated word names (`ADD-ONE`, `TIMES-TWO`)
- Function composition and chaining
- Stack manipulation for data processing
- Multiple word definitions in one file

## Notes

The `fibonacci.f` example specifically tests the compiler's ability to handle hyphenated identifiers by converting them to valid C identifiers (e.g., `ADD-ONE` becomes `word_add_one`).