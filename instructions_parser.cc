
#include <iostream>
#include <cstdlib>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#include "lexer.h"
#include <map>

using namespace std;

// Global variables
LexicalAnalyzer lexer;
InstructionNode* head;
InstructionNode* current;
map<string, int> var_map;
Token token;
int varIndex;


int location(Token token) {
    string variableName = token.lexeme;
    if (!var_map.count(variableName)) {
        var_map[variableName] = varIndex;
    }
    if (token.token_type == NUM) {
        mem[varIndex] = stoi(variableName);
    }
    varIndex++;
    return var_map[variableName];
}

ArithmeticOperatorType arithmetic_op_from_token(TokenType tokenType) {
    switch (tokenType) {
        case PLUS:
            return OPERATOR_PLUS;
        case MINUS:
            return OPERATOR_MINUS;
        case MULT:
            return OPERATOR_MULT;
        case DIV:
            return OPERATOR_DIV;
        default:
            return OPERATOR_NONE;
    }

} 

ConditionalOperatorType conditional_op_from_token(TokenType tokenType) {
    switch (tokenType) {
        case NOTEQUAL:
            return CONDITION_NOTEQUAL;
        case GREATER:
            return CONDITION_GREATER;
        case LESS:
            return CONDITION_LESS;
        default:
            return CONDITION_NOTEQUAL;
    }
}

struct InstructionNode* parse_assignment_statement() {
    InstructionNode* instruction = new InstructionNode;
    instruction->next = nullptr;
    instruction->type = ASSIGN;
    instruction->assign_inst.left_hand_side_index = location(token);
    lexer.GetToken(); // EQUAL

    token = lexer.GetToken(); // First operand
    instruction->assign_inst.operand1_index = location(token);

    token = lexer.GetToken(); // Operation or semicolon
    if (token.token_type == SEMICOLON) {
        instruction->assign_inst.op = OPERATOR_NONE;
    } else {
        instruction->assign_inst.op = arithmetic_op_from_token(token.token_type);
        token = lexer.GetToken(); // Second operand
        instruction->assign_inst.operand2_index = location(token);
        token = lexer.GetToken(); // Semicolon
    }

    return instruction;
}

struct InstructionNode* parse_conditional_statement() {
    InstructionNode* instruction = new InstructionNode;
    instruction->next = nullptr;
    instruction->type = CJMP;
    token = lexer.GetToken(); // First operand
    instruction->cjmp_inst.operand1_index = location(token);

    token = lexer.GetToken(); // Operator
    instruction->cjmp_inst.condition_op = conditional_op_from_token(token.token_type);

    token = lexer.GetToken(); // Second operand
    instruction->cjmp_inst.operand2_index = location(token);

    return instruction;
}

void update_instructions(InstructionNode* instruction) {
    if (head == nullptr) {
        head = instruction;
    } else {
        current->next = instruction;
    }
    current = instruction;
}

void parse_body() {
    // Parse instructions
    token = lexer.GetToken();
    while (token.token_type != RBRACE) {
        InstructionNode* instruction = new InstructionNode;
        instruction->next = nullptr;
        // Assignment
        if (token.token_type == ID) {
            instruction = parse_assignment_statement();
            update_instructions(instruction);
        }
        // Input
        else if (token.token_type == INPUT) {
            instruction->type = IN;
            token = lexer.GetToken(); // Input variable
            instruction->input_inst.var_index = location(token);
            token = lexer.GetToken(); // Semicolon

            update_instructions(instruction);
        }
        // Output
        else if (token.token_type == OUTPUT) {
            instruction->type = OUT;
            token = lexer.GetToken(); // Output variable
            instruction->output_inst.var_index = location(token);
            token = lexer.GetToken(); // Semicolon

            update_instructions(instruction);
        }
        // If
        else if (token.token_type == IF) {
            instruction = parse_conditional_statement();

            update_instructions(instruction);
            token = lexer.GetToken(); // Left brace
            parse_body(); // Parse the body inside the if statement

            InstructionNode* noopInstruction = new InstructionNode;
            noopInstruction->type = NOOP;
            noopInstruction->next = nullptr;
            instruction->cjmp_inst.target = noopInstruction;
            update_instructions(noopInstruction);
        }
        // While
        else if (token.token_type == WHILE) {
            instruction = parse_conditional_statement();

            update_instructions(instruction);
            token = lexer.GetToken(); // Left brace
            parse_body(); // Parse the body inside the while loop
            
            // Jump back to while condition
            InstructionNode* jumpInstruction = new InstructionNode;
            jumpInstruction->type = JMP;
            jumpInstruction->next = nullptr;
            jumpInstruction->jmp_inst.target = instruction;
            update_instructions(jumpInstruction);

            InstructionNode* noopInstruction = new InstructionNode;
            noopInstruction->type = NOOP;
            noopInstruction->next = nullptr;
            instruction->cjmp_inst.target = noopInstruction;
            update_instructions(noopInstruction);
        }
        // For
        else if (token.token_type == FOR) {
            token = lexer.GetToken(); // Left Parenthesis
            token = lexer.GetToken(); // Left hand side
            InstructionNode* initilization = parse_assignment_statement();
            update_instructions(initilization);

            InstructionNode* condition = parse_conditional_statement();
            update_instructions(condition);

            token = lexer.GetToken(); // Semicolon
            token = lexer.GetToken(); // Left hand side
            InstructionNode* increment = parse_assignment_statement();
            token = lexer.GetToken(); // Right Paraenthesis
            token = lexer.GetToken(); // Left brace

            parse_body(); // Parse the body inside the for loop
            update_instructions(increment);

            // Jump back to for condition
            InstructionNode* jumpInstruction = new InstructionNode;
            jumpInstruction->type = JMP;
            jumpInstruction->next = nullptr;
            jumpInstruction->jmp_inst.target = condition;
            update_instructions(jumpInstruction);

            InstructionNode* noopInstruction = new InstructionNode;
            noopInstruction->type = NOOP;
            noopInstruction->next = nullptr;
            condition->cjmp_inst.target = noopInstruction;
            update_instructions(noopInstruction);
        }
        // Switch
        else if (token.token_type == SWITCH) {
            Token leftSide = lexer.GetToken(); // Left side of condition
            token = lexer.GetToken(); // Left brace
            InstructionNode* noopInstruction = new InstructionNode;
            noopInstruction->next = nullptr;
            noopInstruction->type = NOOP;

            token = lexer.GetToken();
            while (token.token_type != RBRACE) {
                if (token.token_type == CASE) {
                    Token rightSide = lexer.GetToken(); // Right side of condition
                    token = lexer.GetToken(); // Colon
                    token = lexer.GetToken(); // Left brace

                    InstructionNode* condition = new InstructionNode;
                    condition->type = CJMP;
                    condition->next = nullptr;
                    condition->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
                    condition->cjmp_inst.operand1_index = location(leftSide);
                    condition->cjmp_inst.operand2_index = location(rightSide);
                    update_instructions(condition);

                    parse_body();

                    condition->cjmp_inst.target = condition->next;
                    condition->next = nullptr;
                    current->next = noopInstruction;
                    current = condition;
                } else if (token.token_type == DEFAULT) {
                    token = lexer.GetToken(); // Colon
                    token = lexer.GetToken(); // Left brace
                    parse_body();
                }
                token = lexer.GetToken();
            }
            update_instructions(noopInstruction);  
        }
        token = lexer.GetToken();
    }
}

// Parser function
struct InstructionNode * parse_generate_intermediate_representation() {
    varIndex = 0;
    head = nullptr;
    current = nullptr;

    // Parse variable names
    token = lexer.GetToken();
    while (token.token_type != LBRACE) {
        if (token.token_type == ID) {
            location(token);
        }
        token = lexer.GetToken();
    }

    // Parse Instructions
    parse_body();

    // Parse inputs
    token = lexer.GetToken();
    while (token.token_type != END_OF_FILE) {
        if (token.token_type == NUM) {
            inputs.push_back(stoi(token.lexeme));
        }
        token = lexer.GetToken();
    }

    // Return the head of the instructions linked list
    return head;
}