#include "chunk.h"
#include "common.h"
#include "debug.h"

int main(int argc, char** argv) {
    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2); // will add a constant to the values array of chunk and return its index
    writeChunk(&chunk, OP_CONSTANT);
    writeChunk(&chunk, constant); // writing the index of the constant to the chunk

    writeChunk(&chunk, OP_RETURN);
  
    disassembleChunk(&chunk, "test chunk");

    freeChunk(&chunk);
    return 0;
} 
