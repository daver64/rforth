#!/usr/bin/env python3

# Get current words from builtin table  
import subprocess
import re

# Extract current words from C code
result = subprocess.run(['grep', '-o', '    {"[^"]*"', 'src/builtins.c'], 
                       capture_output=True, text=True, cwd='/home/dave/source/rforth')
current_words = set()
for line in result.stdout.strip().split('\n'):
    match = re.search(r'"([^"]*)"', line)
    if match:
        current_words.add(match.group(1).upper())

# ANSI Core words (133) - MANDATORY for 100% compliance  
ansi_core_words = {
    "!", "#", "#>", "#S", "'", "(", "*", "*/", "*/MOD", "+", "+!", "+LOOP", ",", "-", ".", ".\"",
    "/", "/MOD", "0<", "0=", "1+", "1-", "2!", "2*", "2/", "2@", "2DROP", "2DUP", "2OVER", "2SWAP",
    ":", ";", "<", "<#", "=", ">", ">BODY", ">IN", ">NUMBER", ">R", "?DUP", "@", "ABORT", "ABORT\"",
    "ABS", "ACCEPT", "ALIGN", "ALIGNED", "ALLOT", "AND", "BASE", "BEGIN", "BL", "C!", "C,", "C@",
    "CELL+", "CELLS", "CHAR", "CHAR+", "CHARS", "CONSTANT", "COUNT", "CR", "CREATE", "DECIMAL",
    "DEPTH", "DO", "DOES>", "DROP", "DUP", "ELSE", "EMIT", "ENVIRONMENT?", "EVALUATE", "EXECUTE",
    "EXIT", "FILL", "FIND", "FM/MOD", "HERE", "HOLD", "I", "IF", "IMMEDIATE", "INVERT", "J", "KEY",
    "LEAVE", "LITERAL", "LOOP", "LSHIFT", "M*", "MAX", "MIN", "MOD", "MOVE", "NEGATE", "OR", "OVER",
    "POSTPONE", "QUIT", "R>", "R@", "RECURSE", "REPEAT", "ROT", "RSHIFT", "S\"", "S>D", "SIGN",
    "SM/REM", "SOURCE", "SPACE", "SPACES", "STATE", "SWAP", "THEN", "TYPE", "U.", "U<", "UM*",
    "UM/MOD", "UNLOOP", "UNTIL", "VARIABLE", "WHILE", "WORD", "XOR", "[", "[']", "[CHAR]", "]"
}

# Find missing words
missing = ansi_core_words - current_words
implemented = ansi_core_words & current_words

print(f"🎯 UPDATED ANSI COMPLIANCE STATUS:")
print(f"==================================")
print(f"Current RForth words: {len(current_words)}")
print(f"ANSI Core words required: {len(ansi_core_words)}")
print(f"Actual compliance: {len(implemented)}/{len(ansi_core_words)} = {100 * len(implemented) // len(ansi_core_words)}%")
print(f"Missing words: {len(missing)}")
print(f"")

if len(missing) > 0:
    print(f"REMAINING MISSING WORDS ({len(missing)}):")
    print(f"===============================")
    
    categories = {
        "Numeric Output": ["#", "#>", "#S", "<#", "SIGN", "HOLD", "S>D"],
        "Dictionary": ["'", ">BODY", ">IN", ">NUMBER", "FIND", "WORD"],
        "Comments": ["("],
        "Compilation": ["EXIT", "LITERAL", "POSTPONE", "RECURSE", "[", "]"],
        "Memory": ["2!", "2@", "ALIGN", "ALIGNED", "CELL+", "CELLS", "FILL", "MOVE"],
        "Character": ["CHAR", "CHAR+", "CHARS", "C,"],
        "String/Input": ["ABORT\"", "ACCEPT", "ENVIRONMENT?", "SOURCE"],
        "Arithmetic": ["LSHIFT", "M*", "RSHIFT", "SM/REM", "UM*", "UM/MOD"],
        "Advanced": ["[']", "[CHAR]"]
    }
    
    for category, words in categories.items():
        cat_missing = [w for w in words if w in missing]
        if cat_missing:
            print(f"\n{category} ({len(cat_missing)}):")
            for word in sorted(cat_missing):
                print(f"  {word}")
    
    print(f"\nOTHER MISSING:")
    other_missing = missing - {w for cat_words in categories.values() for w in cat_words}
    for word in sorted(other_missing):
        print(f"  {word}")
    
    print(f"\n🎯 Need {len(missing)} more words for TRUE 100% ANSI compliance!")
else:
    print(f"🎉 CONGRATULATIONS! 100% ANSI CORE COMPLIANCE ACHIEVED! 🎉")

print(f"\n🚀 RECENT ADDITIONS WORKING:")
new_words = ["HERE", "ALLOT", ",", "C,", "BL", "SPACES", "DECIMAL", "BASE", "STATE", "INVERT", "XOR", "U.", "U<", "2OVER", "UNLOOP", ":", ";"]
working = [w for w in new_words if w in implemented]
print(f"✅ {len(working)} new words implemented: {', '.join(working)}")