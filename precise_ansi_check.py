#!/usr/bin/env python3

# Current RForth words (91)
current_words = {
    "!", "*", "*/", "+", "+!", "-", ".", ".\"", "/", "<", "<=", "<>", "=", ">", ">=", "?", "@",
    "0<", "0=", "0>", "1+", "1-", "2*", "2/", "2drop", "2dup", "2swap", "?dup", "abs", "and",
    "begin", "c!", "c@", "constant", "count", "cr", "create", "depth", "do", "does>", "drop",
    "dup", "else", "emit", "evaluate", "execute", "f.", ">float", ">int", "fm/mod", "i", "if",
    "immediate", "j", "key", "key?", "leave", "+loop", "loop", "max", "min", "*/mod", "/mod",
    "mod", "negate", "not", "or", "over", "pick", "quit", ">r", "r>", "r@", "repeat", "roll",
    "rot", ".s", "s\"", "space", "sqrt", "swap", "then", "turnkey", "type", "until", "variable",
    "while", "words", "bye", "abort"
}

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

# Convert current words to uppercase for case-insensitive comparison
current_upper = {word.upper() for word in current_words}

# Find missing words
missing = ansi_core_words - current_upper

print(f"ANSI COMPLIANCE REALITY CHECK:")
print(f"================================")
print(f"Current RForth words: {len(current_words)}")
print(f"ANSI Core words required: {len(ansi_core_words)}")
print(f"Actual compliance: {len(current_upper & ansi_core_words)}/{len(ansi_core_words)} = {100 * len(current_upper & ansi_core_words) // len(ansi_core_words)}%")
print(f"Missing words: {len(missing)}")
print(f"")
print(f"MISSING ANSI CORE WORDS ({len(missing)}):")
print(f"=============================")

categories = {
    "Numeric Output": ["#", "#>", "#S", "<#", "SIGN", "HOLD", "U.", "S>D"],
    "Dictionary": ["'", ",", "HERE", "ALLOT", ">BODY", ">IN", ">NUMBER", "FIND", "WORD"],
    "Comments": ["("],
    "Compilation": [":", ";", "[", "]", "LITERAL", "POSTPONE", "RECURSE", "EXIT"],
    "Memory": ["2!", "2@", "2OVER", "CELL+", "CELLS", "ALIGN", "ALIGNED", "FILL", "MOVE"],
    "Character": ["CHAR", "CHAR+", "CHARS", "BL", "C,"],
    "String/Input": ["ACCEPT", "SPACES", "SOURCE", "ABORT\"", "ENVIRONMENT?"],
    "Arithmetic": ["*/MOD", "LSHIFT", "RSHIFT", "M*", "UM*", "UM/MOD", "SM/REM", "INVERT", "XOR", "U<"],
    "System": ["BASE", "DECIMAL", "STATE", "UNLOOP"],
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

print(f"\n🎯 TRUTH: We need {len(missing)} more words for TRUE 100% ANSI compliance!")