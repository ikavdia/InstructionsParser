#include <iostream>
#include <cstdlib>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include <map>
#include <unordered_map>
#include "compiler.h"
#include "lexer.h"

using namespace std;

LexicalAnalyzer g_lexer;
unordered_map<string, int> g_variableMap;
Token g_currentToken;
InstructionNode* g_headInstruction;
InstructionNode* g_currentInstruction;
int g_variableIndex;

Token getNextToken();
int getVariableLocation(Token token);
void updateInstructions(InstructionNode* instruction);
InstructionNode* createInstruction(InstructionType type);
ArithmeticOperatorType getArithmeticOperator(TokenType tokenType);
ConditionalOperatorType getConditionalOperator(TokenType tokenType);
void parseVariables();
struct InstructionNode* parseAssignment();
struct InstructionNode* parseConditional();
struct InstructionNode* parseInput();
struct InstructionNode* parseOutput();
struct InstructionNode* parseIfStatement();
struct InstructionNode* parseWhileStatement();
struct InstructionNode* parseForStatement();
void parseSwitchStatement();
void parseBody();
void parseInputs();

Token getNextToken() {
    g_currentToken = g_lexer.GetToken();
    return g_currentToken;
}

int getVariableLocation(Token token) {
    string variableName = token.lexeme;
    auto result = g_variableMap.emplace(variableName, g_variableIndex);
    if (result.second) {
        g_variableIndex++;

        if (token.token_type == NUM) {
            mem[result.first->second] = stoi(variableName);
        }
    }
    return result.first->second;
}

void updateInstructions(InstructionNode* instruction) {
    if (g_headInstruction == nullptr) {
        g_headInstruction = instruction;
    } else {
        g_currentInstruction->next = instruction;
    }
    g_currentInstruction = instruction;
}

InstructionNode* createInstruction(InstructionType type) {
    InstructionNode* instruction = new InstructionNode;
    instruction->type = type;
    instruction->next = nullptr;
    return instruction;
}

ArithmeticOperatorType getArithmeticOperator(TokenType tokenType) {
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

ConditionalOperatorType getConditionalOperator(TokenType tokenType) {
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

void parseVariables() {
    while (getNextToken().token_type != LBRACE) {
        switch (g_currentToken.token_type) {
            case ID:
                getVariableLocation(g_currentToken);
                break;
            default:
                break;
        }
    }
}

struct InstructionNode* parseAssignment() {
    InstructionNode* instruction = createInstruction(ASSIGN);
    instruction->assign_inst.left_hand_side_index = getVariableLocation(g_currentToken);
    getNextToken();
    instruction->assign_inst.operand1_index = getVariableLocation(getNextToken());
    if (getNextToken().token_type == SEMICOLON) {
        instruction->assign_inst.op = OPERATOR_NONE;
    } else {
        instruction->assign_inst.op = getArithmeticOperator(g_currentToken.token_type);
        instruction->assign_inst.operand2_index = getVariableLocation(getNextToken());
        getNextToken();
    }
    return instruction;
}

struct InstructionNode* parseConditional() {
    InstructionNode* instruction = createInstruction(CJMP);
    instruction->cjmp_inst.operand1_index = getVariableLocation(getNextToken());
    instruction->cjmp_inst.condition_op = getConditionalOperator(getNextToken().token_type);
    instruction->cjmp_inst.operand2_index = getVariableLocation(getNextToken());
    return instruction;
}

struct InstructionNode* parseInput() {
    InstructionNode* instruction = createInstruction(IN);
    instruction->input_inst.var_index = getVariableLocation(getNextToken());
    getNextToken();
    return instruction;
}

struct InstructionNode* parseOutput() {
    InstructionNode* instruction = createInstruction(OUT);
    instruction->output_inst.var_index = getVariableLocation(getNextToken());
    getNextToken();
    return instruction;
}

struct InstructionNode* parseIfStatement() {
    InstructionNode* conditional = parseConditional();
    updateInstructions(conditional);
    getNextToken();
    parseBody();
    InstructionNode* noopInstruction = createInstruction(NOOP);
    conditional->cjmp_inst.target = noopInstruction;
    updateInstructions(noopInstruction);
    return conditional;
}

struct InstructionNode* parseWhileStatement() {
    InstructionNode* conditional = parseConditional();
    updateInstructions(conditional);
    getNextToken();
    parseBody();
    InstructionNode* jumpInstruction = createInstruction(JMP);
    jumpInstruction->jmp_inst.target = conditional;
    updateInstructions(jumpInstruction);
    InstructionNode* noopInstruction = createInstruction(NOOP);
    conditional->cjmp_inst.target = noopInstruction;
    updateInstructions(noopInstruction);
    return conditional;
}

struct InstructionNode* parseForStatement() {
    getNextToken();
    getNextToken();
    InstructionNode* initialization = parseAssignment();
    updateInstructions(initialization);
    InstructionNode* condition = parseConditional();
    updateInstructions(condition);
    getNextToken();
    getNextToken();
    InstructionNode* increment = parseAssignment();
    getNextToken();
    getNextToken();
    parseBody();
    updateInstructions(increment);
    InstructionNode* jumpInstruction = createInstruction(JMP);
    jumpInstruction->jmp_inst.target = condition;
    updateInstructions(jumpInstruction);
    InstructionNode* noopInstruction = createInstruction(NOOP);
    condition->cjmp_inst.target = noopInstruction;
    updateInstructions(noopInstruction);
    return initialization;
}

void parseSwitchStatement() {
    Token leftSide = getNextToken(); 
    getNextToken(); 
    InstructionNode* noopInstruction = createInstruction(NOOP);
    getNextToken();
    while (g_currentToken.token_type != RBRACE) {
        if (g_currentToken.token_type == CASE) {
            Token rightSide = getNextToken(); 
            getNextToken(); 
            getNextToken(); 
            InstructionNode* condition = createInstruction(CJMP);
            condition->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
            condition->cjmp_inst.operand1_index = getVariableLocation(leftSide);
            condition->cjmp_inst.operand2_index = getVariableLocation(rightSide);
            updateInstructions(condition);
            parseBody();
            condition->cjmp_inst.target = condition->next;
            condition->next = nullptr;
            g_currentInstruction->next = noopInstruction;
            g_currentInstruction = condition;
        } else if (g_currentToken.token_type == DEFAULT) {
            getNextToken(); 
            getNextToken(); 
            parseBody();
        }
        getNextToken();
    }
    updateInstructions(noopInstruction);
}

void parseBody() {
    getNextToken();
    while (g_currentToken.token_type != RBRACE) {
        switch (g_currentToken.token_type) {
            case ID:
                {
                    InstructionNode* instruction = parseAssignment();
                    updateInstructions(instruction);
                    break;
                }
            case INPUT:
                {
                    InstructionNode* instruction = parseInput();
                    updateInstructions(instruction);
                    break;
                }
            case OUTPUT:
                {
                    InstructionNode* instruction = parseOutput();
                    updateInstructions(instruction);
                    break;
                }
            case IF:
                {
                    parseIfStatement();
                    break;
                }
            case WHILE:
                {
                    parseWhileStatement();
                    break;
                }
            case FOR:
                {
                    parseForStatement();
                    break;
                }
            case SWITCH:
                {
                    parseSwitchStatement();
                    break;
                }
            default:
                break;
        }
        getNextToken();
    }
}

void parseInputs() {
    for (getNextToken(); g_currentToken.token_type != END_OF_FILE; getNextToken()) {
        if (g_currentToken.token_type == NUM) {
            inputs.push_back(stoi(g_currentToken.lexeme));
        }
    }
}

void initializeValues() {
    g_headInstruction = nullptr;
    g_currentInstruction = nullptr;
    g_variableIndex = 0;
}

struct InstructionNode* parse_generate_intermediate_representation() {
    initializeValues();
    parseVariables();
    parseBody();
    parseInputs();
    return g_headInstruction;
}