#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

/*
    we call disassembleChunk() to disassemble all of the instructions in the entire chunk. 
    That’s implemented in terms of the other function, which just disassembles a single instruction.
*/
void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif
