#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "value.h"
#include "vm.h"

VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

void initVM() {
    resetStack();
}

void freeVM() {
}

void push(Value value) {
    *vm.stackTop = value; // store value
    ++vm.stackTop;        // increment 'top' ptr
}

Value pop() {
    --vm.stackTop;        // decrement 'top' ptr
    return *vm.stackTop;  // return popped value
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++) // This macro reads the byte currently pointed at by the instruction pointer and then it increments it
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
        // When this flag is defined the VM disassembles and prints each instruction right before executing it    
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
        uint8_t instruction;
        
        // Decoding (dispatching) the instruction
        switch (instruction = READ_BYTE()) {
            // The body of each case implements that opcode’s behavior.
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                printValue(constant);
                printf("\n");
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(Chunk *chunk) {
    vm.chunk = chunk; // storing the chunk being executed
    vm.ip = vm.chunk->code;
    return run();
}
