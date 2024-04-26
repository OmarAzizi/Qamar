#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

/*
    So for each function call that hasn’t returned yet we need to track where on the stack that function’s locals begin, 
    and where the caller should resume. We’ll put this, along with some other stuff, in a new struct.

    This struct represent a single ongoing function    
*/
typedef struct {
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;   /* This will point the the VM's value stack at the first slot the function can use */
} CallFrame;

typedef struct {
/*
    This array replaces the `chunk` and `ip` fields we used to have directly in the VM. Now each CallFrame has its own `ip` 
    and its own pointer to the ObjFunction that it’s executing. From there, we can get to the function’s chunk.
*/
    CallFrame frames[FRAMES_MAX];
    int frameCount; /* Stores the current height of the `CallFrame` stak */

    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;
    ObjUpvalue* openUpvalues;
    Obj* objects;   /* The VM stors a pointer to the head of the Obj's list */
} VM;

/*
    When the VM interprets a chunk of bytecode it will return one of these 
*/
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();

/*
    The 'interpret' function will be the VM's entrypoint. 
    The VM runs the chunk and then responds with an enum value.
*/
InterpretResult interpret(const char* source); 

/* Defining the stack protocol for the VM */
void push(Value value);
Value pop();

#endif
