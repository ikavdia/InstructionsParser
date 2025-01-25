# InstructionsParser
This project involves implementing a small compiler for a simple programming language. The compiler reads an input program and represents it in a linked list data structure as an intermediate representation. The execution of the program is done by interpreting this generated intermediate representation.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Language Grammar](#language-grammar)
- [Execution Semantics](#execution-semantics)
- [Implementation Details](#implementation-details)
- [Usage](#usage)
- [Testing](#testing)
- [Notes](#notes)

## Overview
This project is a small compiler designed for a simple programming language. The compiler generates an intermediate representation (IR) of the input program as a linked list of instructions and executes the program by interpreting the IR.

## Features
- **Custom Language Support**:
  - Variable declarations
  - Input/output operations
  - Assignment statements
  - Control flow (if, while, switch, for loops)
  - Arithmetic operations
- **Intermediate Representation**:
  - Linked list-based data structure
  - Execution via an interpreter
- **Error-Free Input**: Assumes all input programs are syntactically and semantically correct.

## Language Grammar
The compiler is based on the following grammar:
```
program → var_section body inputs
var_section → id_list SEMICOLON
id_list → ID COMMA id_list | ID
body → LBRACE stmt_list RBRACE
stmt_list → stmt stmt_list | stmt
stmt → assign_stmt | while_stmt | if_stmt | switch_stmt | for_stmt
assign_stmt → ID EQUAL primary SEMICOLON
expr → primary op primary
primary → ID | NUM
op → PLUS | MINUS | MULT | DIV
output_stmt → output ID SEMICOLON
input_stmt → input ID SEMICOLON
while_stmt → WHILE condition body
if_stmt → IF condition body
condition → primary relop primary
relop → GREATER | LESS | NOTEQUAL
switch_stmt → SWITCH ID LBRACE case_list RBRACE
for_stmt → FOR LPAREN assign_stmt condition SEMICOLON assign_stmt RPAREN body
```

Key highlights:
1. Integer division is supported.
2. If statements do not have an `else` block.
3. Syntax resembles C-style loops.
4. All variables are implicitly of type `int`.

## Execution Semantics
### Supported Statements
1. **Input/Output**: Read/write variables using the `input` and `output` keywords.
2. **Assignment**: Evaluate right-hand expressions and assign to variables.
3. **If/While**: Conditional execution based on boolean evaluations.
4. **Switch**: Match values to cases and execute corresponding bodies.
5. **For**: Iterative execution with initialization, condition, and update expressions.

### Linked List-Based Execution
- The intermediate representation is a sequence of nodes, each representing an operation:
  - Assignment
  - Conditional jump (CJMP)
  - Unconditional jump (JMP)
  - Input/Output

Execution starts at the first node and proceeds based on node types.

## Implementation Details
1. **Data Structures**:
   - **InstructionNode**: Represents individual operations (e.g., ASSIGN, OUT, CJMP, JMP).
   - **Memory Array**: Stores variable values and constants.
   - **Location Table**: Maps variable names to memory locations.

2. **Control Flow**:
   - **If/While**: Use CJMP nodes for conditional execution.
   - **Switch/For**: Combine CJMP and JMP nodes for branch handling.

3. **Execution**:
   - An interpreter processes the linked list sequentially.
   - Special cases handle conditional and jump instructions.

## Usage
1. **Clone Repository**:
   ```bash
   git clone <repository_url>
   cd <repository_folder>
   ```

2. **Compile Code**:
   Use a C++ compiler to build the program:
   ```bash
   g++ -o compiler instructions_parser.cc parser.cc
   ```

3. **Run Compiler**:
   Provide an input program file as an argument:
   ```bash
   ./compiler <input_program>
   ```

4. **Output**:
   The compiler generates the program's output based on the intermediate representation.

## Testing
Extensive test cases are recommended, particularly for:
- Assignment statements
- Control flow constructs (if, while, switch, for)
- Input/output operations

Example test case:
```plaintext
a, b;
{
    a = 0;
    FOR (a = 1; a < 5; a = a + 1) {
        output a;
    }
}
input: None
Expected output: 1, 2, 3, 4
```

## Notes
- **Language**: This project is implemented in C++.
- **Testing**: Ensure extensive testing for `switch` and `for` constructs.
- **Output**: Input programs should produce correct results based on the described grammar and execution semantics.

---
