#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    /* Every time we allocate an Obj we insert it in the head of the list */
    object->next = vm.objects;
    vm.objects = object;
    return object;
}

ObjClosure* newClosure(ObjFunction* function) {
/*
    When we create an `ObjClosure`, we allocate an upvalue array of the proper size.
*/
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; ++i) {
        upvalues[i] = NULL;
    }    

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    tableSet(&vm.strings, string, NIL_VAL);
    return string;
}

ObjFunction* newFunction() {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjNative* newNative(NativeFn function) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

/*
    This function implements the FNV-1a hash algorithm
*/
static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;  /* Initial hash */
    for (int i = 0; i < length; ++i) {
        hash ^= (uint8_t)key[i];  /* Bitwise XOR */
        hash *= 16777619;
    }
    return hash;
}

ObjString* takeString(char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    
    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

/*
    `newUpvalue` takes the address of the slot where the closed-over variable lives.
*/
ObjUpvalue* newUpvalue(Value* slot) {
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NIL_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

static void printFunction(ObjFunction* function) {
    if (function->name == NULL) {
        printf("<script>");
        return;
    }
    printf("<fn %s>", function->name->chars); 
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_CLOSURE: 
        /*
            Closures display exactly as ObjFunction does. From the user’s perspective, 
            the difference between ObjFunction and ObjClosure is purely a hidden implementation detail. 
        */
            printFunction(AS_CLOSURE(value)->function);
            break;
        case OBJ_FUNCTION: 
            printFunction(AS_FUNCTION(value)); 
            break;
        case OBJ_NATIVE:
            printf("<native fn>");
            break;
        case OBJ_STRING:   
            printf("%s", AS_CSTRING(value)); 
            break;
        case OBJ_UPVALUE:
            printf("upvalue");
            break;
    }
}
