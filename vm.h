#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"

typedef struct {
    Chunk* chunk; // storing the chunk of bytecode 
    uint8_t* ip;  // this pointer will keep track of where we are in the bytecode array (instruction pointer OR program counter)
} VM;

/*
    When the VM interprets a chunk of bytecode it will return one of these 
*/
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();

/*
    The 'interpret' function will be the VM's entrypoint. 
    The VM runs the chunk and then responds with an enum value.
*/
InterpretResult interpret(Chunk* chunk); 

#endif
