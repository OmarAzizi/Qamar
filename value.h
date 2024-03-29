#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef struct Obj Obj; /* This will act as a base class for heap objects */
typedef struct ObjString ObjString; /* The payload for strings is defined here */

typedef enum {
    VAL_BOOL,
    VAL_NIL, 
    VAL_NUMBER,
    VAL_OBJ     /* This will refer to all heap-allocated types */
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool   boolean;
        double number;
        Obj*   obj; /* When Value's type is `VAL_OBJ` the payload is a pointer to heap */
    } as; 
} Value;

#define BOOL_VAL(value)   ((Value) {VAL_BOOL, {.boolean = value}})
#define NIL_VAL           ((Value) {VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value) {VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object)   ((Value) {VAL_OBJ, {.obj = (Obj*)object}})

#define AS_OBJ(value)     ((value).as.obj)
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_OBJ(value)     ((value).type == VAL_OBJ)

/* The implemntation for the following is really similar to Chunk implemntation */
typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);

void printValue(Value value); // printing a clox value

#endif
