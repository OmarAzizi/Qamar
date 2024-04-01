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
    For this function you pass in a table and a key. If it finds an entry with that key, it returns true, otherwise it returns false. 
    If the entry exists, the value output parameter points to the resulting value.
*/
bool tableGet(Table* table, ObjString* key, Value* value);

/* 
    This function adds the given key/value pair to the given hash table 
    return -> true if the new entry was added
*/
bool tableSet(Table* table, ObjString* key, Value value);

/*
    This function deletes an entry from the table
*/
bool tableDelete(Table* table, ObjString* key);

/*
    This is a helper method that copies all of the entries of on hach table to another
*/
void tableAddAll(Table* from, Table* to);
ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);

#endif
