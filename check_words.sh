#!/bin/bash
# Extract just the builtin word names (not error messages or format strings)

cd /home/dave/source/rforth

# Get the builtin word names from the builtin_words array
grep -A 200 'builtin_word builtin_words\[\] = {' src/builtins.c | \
grep '{"' | \
sed 's/.*{"\([^"]*\)".*/\1/' | \
sort > current_words.txt

echo "Current RForth words ($(wc -l < current_words.txt)):"
cat current_words.txt
echo
echo "Checking against ANSI Core requirements..."