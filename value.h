/*
    This module basically implements something similar to (constant pool) in Java.
*/

#ifndef clox_value_h
#define clox_value_h

#include "common.h"

/*
    This typedef abstracts how Lox values are concretely represented in C
*/
typedef double Value;

/* The implemntation for the following is really similar to Chunk implemntation */
typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);

#endif
