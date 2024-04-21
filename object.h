#ifndef clox_object_h
#define clox_object_h

/*
    This module implements all heap-allocated object 
    in the language such as strings, instances, functions, etc...
*/

#include "common.h"
#include "value.h"
#include "chunk.h"

/* This macro that extracts the object type tag from a given Value. */
#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

#define IS_CLOSURE(value)   isObjType(value, OBJ_CLOSURE)
#define AS_CLOSURE(value)   ((ObjClosure*)AS_OBJ(value))

#define IS_FUNCTION(value)  isObjType(value, OBJ_FUNCION)
#define AS_FUNCTION(value)  ((ObjFunction*)AS_OBJ(value))

#define IS_NATIVE(value)    isObjType(value, OBJ_NATIVE)
#define AS_NATIVE(value)    (((ObjNative*)AS_OBJ(value))->function)

/* When we cast an `Obj*` to `ObjString*` we need to make sure it points to an `obj` field of an actial `ObjString` */
#define IS_STRING(value)    isObjType(value, OBJ_STRING)

/*
    These two macros take a Value that is expected to contain a pointer to a valid ObjString on the heap. 
    The first one returns the ObjString* pointer. The second one steps through that to return the character array itself.
*/
#define AS_STRING(value)    ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)   (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;   /* The Obj iself will be a linked-list (it helps with garbage collection) */
};

typedef struct {
    Obj obj;            
    int arity;          /* Number of parameters the function expects */
    Chunk chunk;        /* Each function will have it's own chunk of Bytecode */
    ObjString* name;
} ObjFunction;

/*
    The native function takes the argument count and a pointer to the first. It accesses the arguments through that pointer
*/
typedef Value (*NativeFn)(int argCount,  Value* args);

typedef struct {
    Obj obj;
    NativeFn function;  /* A pointer to the C function that implements the native behaviour */
} ObjNative;

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;      /* Each ObjString will store a hash, this will help in the implementation of hash tables*/
};

/*
    We’ll wrap every function in an ObjClosure, even if the function doesn’t actually close over and capture any surrounding local variables
*/
typedef struct {
    Obj obj;
    ObjFunction* function;
} ObjClosure;

ObjClosure*  newClosure(ObjFunction* function);
ObjFunction* newFunction();
ObjNative*   newNative(NativeFn function);

ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
