#include <stdio.h>
#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char *name) {
    printf("== %s ==\n", name); 
    for (int offset = 0; offset < chunk->count;) 
        offset = disassembleInstruction(chunk, offset);
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1]; // accessing index of the constant
    printf("%-16s %d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");

    return offset + 2;
}

/*
    disassembleInstruction returns a number to tell the caller the 
    offset of the beginning of the next instruction
*/
int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset); // prints the byte offset of each instruction
    
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
        printf("    | "); // we show a '|' for any instruction that comes from the same source line as the preceding one
    else printf("%4d ", chunk->lines[offset]);

    uint8_t instruction = chunk->code[offset]; // reads single byte from bytecode
    switch (instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
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
