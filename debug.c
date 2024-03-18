#include <stdio.h>
#include "debug.h"

void disassembleChunk(Chunk* chunk, const char *name) {
    printf("== %s ==\n", name); 
    for (int offset = 0; offset < chunk->count;) 
        offset = disassembleInstruction(chunk, offset);
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset); // prints the byte offset of each instruction

    uint8_t instruction = chunk->code[offset]; // reads single byte from bytecode
    switch (instruction) {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

/*
    Example:

    == test chunk ==
    0000 OP_RETURN
*/
