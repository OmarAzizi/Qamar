#include <stdint.h>
#include <stdlib.h>

#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.69 /* This will manage the table's load factor */

void initTable(Table *table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table *table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

/*
    This function’s job is to take a key and figure out which bucket in the array it should go in. 
    It returns a pointer to that bucket—the address of the Entry in the array.
*/
static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
    uint32_t index = key->hash % capacity;
    for (;;)   {
        Entry* entry = &entries[index]; 
        if (entry->key == key || entry->key == NULL) return entry;
        index = (index + 1) % capacity;
    }
}

bool tableSet(Table *table, ObjString *key, Value value) {
    /* We grow the array when it becomes at least 75% full */
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    if (isNewKey) ++table->count;
    
    /* If the key already exists the value will overwrite the old one */
    entry->key = key;
    entry->value = value;
    return isNewKey;
}
