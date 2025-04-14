# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands
- Compile: `gcc -o scream scream.c -lncurses`
- Quick build: `./buildit.sh`
- Run: `./scream`

## Code Style Guidelines
- Use C99 standard
- Indentation: 4 spaces
- Function names: snake_case
- Constants: ALL_CAPS
- Variable names: snake_case
- Structs: PascalCase for type names
- Include headers in alphabetical order after stdio.h and stdlib.h
- Max line length: 80 characters
- Error handling: Check system call returns, set status messages
- Always use curly braces for control structures, even for single statements
- Memory management: Free all allocated memory
- Constants: Use #define for constants at top of file