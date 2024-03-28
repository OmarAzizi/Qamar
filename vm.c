#include <stdio.h>
#include <stdarg.h>

#include "compiler.h"
#include "chunk.h"
#include "debug.h"
#include "value.h"
#include "vm.h"

VM vm;

static void resetStack() { 
    vm.stackTop = vm.stack; 
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

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

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

/*
    Here i'm following the rule in Ruby that `nil` and `false` are falsey and every other value behaves like `true`
*/
static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++) // This macro reads the byte currently pointed at by the instruction pointer and then it increments it
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(valueType(a op b)); \
    } while (false)

    for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
        // Stack Tracing (Printing contents of the VM's stack from bottom up)
        printf("            ");
        for (Value* slot = vm.stack; slot < vm.stackTop; ++slot) {
            printf("[");
            printValue(*slot);
            printf("]");
        }
        printf("\n");

        // When this flag is defined the VM disassembles and prints each instruction right before executing it    
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
        uint8_t instruction;
        
        // Decoding (dispatching) the instruction
        switch (instruction = READ_BYTE()) {
            // The body of each case implements that opcode’s behavior.
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NIL:        push(NIL_VAL); break;
            case OP_TRUE:       push(BOOL_VAL(true)); break;
            case OP_FALSE:      push(BOOL_VAL(false)); break;
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:    BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:       BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD:        BINARY_OP(NUMBER_VAL, +); break;
            case OP_SUBTRACT:   BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:   BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:     BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT:        push(BOOL_VAL(isFalsey(pop()))); break;
            case OP_NEGATE:     
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
    Chunk chunk;
    initChunk(&chunk);
   
/*
    We create a new empty chunk and pass it over to the compiler. 
    The compiler will take the user’s program and fill up the chunk with bytecode.
*/
    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }
    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    /* Then we send the chunk to the VM to be executed */
    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}

