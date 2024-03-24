#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

int main(int argc, char** argv) {
    initVM();

    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2); // will add a constant to the values array of chunk and return its index
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123); // writing the index of the constant to the chunk

    constant = addConstant(&chunk, 3.4);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123); 

    writeChunk(&chunk, OP_ADD, 123);

    constant = addConstant(&chunk, 5.6);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123); 

    writeChunk(&chunk, OP_DIVIDE, 123);

    writeChunk(&chunk, OP_NEGATE, 123);

    // compiled expression => -((1.2 + 3.4) / 5.6) = -0.821429
    writeChunk(&chunk, OP_RETURN, 123); // end of chunk

    interpret(&chunk); // telling the VM interpret a chunk of bytecode 

    freeVM();
    freeChunk(&chunk);
    return 0;
} 
