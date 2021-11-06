# SPU
This is a Soft Processing Unit application with custom assembly language, implementing it's assembler, disassembler and interpreter.
## Building release version 
(for full stack debug capabilities add `FULL_DEBUG` macro)
```bash
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
```
## Assembly
```
assembler -a [-ho] [filename] < program.gasm [> program.gbin]
``` 
Assembles source code in castom assembly to binary file tha could be interpreted
## Disassembly
```
assembler -d [-ho] [filename] < program.gasm [> program.gbin]
``` 
Disassembles binary file to human-readable format (assembly language)
## Interpreter
```
interpreter < program.gbin
```
Interprets binary file and outputs results to `stdout`

## Opcodes
Opcodes could be easily added and modified in ./include/commands.tpl
| Opcode | Description   | argc |
|:------ |:------------- |:----:|
| idle   | Do nothing, could take one argument to create lable indent | 0/1 |
| push   | Push argument to stack | 1 |
| pop    | Pop from stack and discard the result / save result to argument | 0/1 |
| add    | Pop two values from stack, add them and push the result | 0 |
| add    | Add the second argumen to the first one | 2 |
| sub    | Pop two values from stack, subtracted them and push the result [push(pop_2 - pop_1)] | 0 |
| sub    | Subtract the second argumen from the first one | 2 |
| mul    | Pop two values from stack, multiply them and push the result | 0 |
| mul    | Multiply the first argument by the second one | 2 |
| div    | Pop two values from stack, divides them and push the result [push(pop_2 / pop_1)] | 0 |
| div    | Divides the first argument by the second one | 2 |
| mov    | Copies value from the second argument to the first one | 2 |
| out    | Pops value from stack and prints it to `stdout` | 0 |
| out    | Prints argument to `stdout` | 1 |
| jmp    | Jumps to position of the argument (usually a goto lable) | 1 |
| call   | Makes a function call by starting position (usually a goto lable) | 1 |
| ret    | Returns to the position after last `call` (is stored in general stack) | 0 |
| exit   | Stop the program interpretation with exit code 0 | 0 |
| cmp    | Pop two values from stack, compares them and sets comp register [pop_2 cmp pop_1] | 0 |
| cmp    | Compares arguments and sets comp register | 2 |
| jeq    | Jumps to position of the argument (usually a goto lable) if comp register is set to equal (0) | 1 |
| jl     | Jumps to position of the argument (usually a goto lable) if comp register is set to less  (-1) | 1 |
| jle    | Jumps to position of the argument (usually a goto lable) if comp register is set to less or equal (-1 / 0) | 1 |
| jg     | Jumps to position of the argument (usually a goto lable) if comp register is set to greater (1) | 1 |
| jge    | Jumps to position of the argument (usually a goto lable) if comp register is set to greater or equal (0 / 1) | 1 |
| dec    | Decreases the top stack element or the argument | 0/1 |
| inc    | Increases the top stack element or the argument | 0/1 |
| sqrt   | Calculates sqrt of the top stack element or the argument | 0/1 |

## Done
1. Bacis assembly logic (assembler/disassembler/interpreter)
2. Optimised bytecode conversion and interpretation
3. Basic cli apps and interface (via getopt)
4. Recursive operand parsing
5. Flexible opcode operands, could implement complex calculations, RAM and register calls
6. Limited substraction in operands calculations (better use brackets after substraction, no unary minus, use 0-n)
7. GoTo functionality, lables and jumps
8. Function calls, no real recursion limit
9. `call main` opening is added to every binary
10. Lables could be used as vars stored in RAM (check out examples)
11. Example programs (solver of quadric equasions and fibinacci recursive calc)
12. Easy opcode list extension and modification with preprocessor codegen
13. Screaming Capybara ASCII art
14. CMake config with fetching my Unkillable Stack ang GoogleTest lib
15. Doxygen docs

## TODO
1. Add unit testing and Github Action CI
2. Add video-RAM and draw some circles
3. Status stack for advanced error handling
4. More durable parser and full subtraction support in operands calculations
