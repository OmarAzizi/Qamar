#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "memory.h"
#include "object.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);

/* 
    This function adds the given key/value pair to the given hash table 

    return -> true if the new entry was added
*/
bool tableSet(Table* table, ObjString* key, Value value);

#endif
