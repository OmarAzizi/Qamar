/*
    This module will define our code representation.
    We will use "chunk" to refer to sequences of bytecode.
*/


#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

/*
    each instruction has a one-byte operation code 
    (universally shortened to opcode)
*/
typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,    // logical not (!true == false)
    OP_NEGATE, // Unary negation (a = 12 | -a == -12)
    OP_PRINT,
    OP_RETURN, // return instruction
} OpCode;

/*
    Bytecode is a series of instructions. Eventually, 
    we’ll store some other data along with the instruction
*/
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines; // This array will keep track of line information
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);

/* This is a convinence method to add a new constant to the chunk */
int addConstant(Chunk* chunk, Value value);

#endif
