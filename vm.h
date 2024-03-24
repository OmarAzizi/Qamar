#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk; // storing the chunk of bytecode 
    uint8_t* ip;  // this pointer will keep track of where we are in the bytecode array (instruction pointer OR program counter)
    Value stack[STACK_MAX];
    Value* stackTop;
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

/* Defining the stack protocol for the VM */
void push(Value value);
Value pop();

#endif
