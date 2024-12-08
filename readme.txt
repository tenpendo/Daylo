# Daylo Programming Language

A LISP-like programming language interpreter written in C, featuring dynamic typing, first-class functions, and an interactive REPL.

## Quick Start

### Windows

bash
.\compile.bat
.\daylo.exe

### Unix-like Systems

bash
make
./daylo


## Prerequisites
- GCC or compatible C compiler
- Make (optional, for Unix systems)
- libedit-dev (for Unix systems)

## Basic Usage

; Start the REPL
Daylo> (+ 1 2 3)
6
Daylo> (def {x} 100)
()
Daylo> (def {add} (\ {x y} {+ x y}))
()
Daylo> (add 1 2)
3

## Features
- Interactive REPL
- Dynamic typing
- First-class functions with closures
- Lexical scoping
- Error handling
- File loading
- Mathematical and logical operations
- List manipulation

## Project Structure
Daylo/
├── docs/
│ ├── daylo_documentation.txt # Comprehensive documentation
├── include/
│ ├── builtins.h # Built-in function declarations
│ ├── lenv.h # Environment declarations
│ ├── lval.h # Value type declarations
│ ├── mpc.h # Parser declarations
│ └── types.h # Type definitions
├── src/
│ ├── builtins.c # Built-in function implementations
│ ├── lenv.c # Environment implementations
│ ├── lval.c # Value type implementations
│ ├── mpc.c # Parser implementations
│ └── project.c # Main program entry
├── README.md # Project overview
├── Makefile # Unix build script
└── compile.bat # Windows build script



## Documentation
For detailed documentation, see (doc.txt).

## Contributing
Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments
- Parser based on the MPC library
- Based on "Build Your Own Lisp" concepts

## Version
0.1.0.0.0

## Author
Danish Shah

