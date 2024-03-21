#include <stdio.h>

#include "chunk.h"
#include "value.h"
#include "vm.h"

VM vm;

void initVM() {
}

void freeVM() {
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++) // This macro reads the byte currently pointed at by the instruction pointer and then it increments it
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for (;;) {
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
