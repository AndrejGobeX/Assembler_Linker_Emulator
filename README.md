# Assembler_Linker_Emulator
System software school project, written in C++, Flex and Bison
## Intro
This is a half-semester project which contains three programs: Assembler, Linker and Emulator.\
Assembler uses Flex and Bison for lexical analysis and assembly of scripts. The input scripts are written in a custom assembly language somewhat based on the intel architecture, but way simplified.\
Linker combines the assembled files and pops out a single file containing machine code for the emulated machine.\
Emulator represents an interpreter which emulates an imaginary architecture for which the above-mentioned language is written.\
\
The emulated machine is a Von Neumann machine. It has a 16bit CPU, a terminal and a timer, with support for interrupts. The emulator uses <termios.h> for terminal input management.
