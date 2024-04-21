#include <stdlib.h>
#include "memory.h"
#include "vm.h"

/*
    This function is the single function we will use for all dynamic memory menagement in clogc 
    -- allocating memory, freeing it, and changing the size if current allocations

parameters:

    oldSize	        newSize	                Operation
    0	            Non‑zero	            Allocate new block.
    Non‑zero	    0	                    Free allocation.
    Non‑zero	    Smaller than oldSize	Shrink existing allocation.
    Non‑zero	    Larger than oldSize	    Grow existing allocation.
*/
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);
    return result;
}

static void freeObject(Obj* object) {
    switch (object->type) {
        case OBJ_CLOSURE: {
        /*
            We free only the ObjClosure itself, not the ObjFunction. That’s because the closure doesn’t own the function.
        */
            FREE(ObjClosure, object);
            break;
        }
        case OBJ_FUNCTION: {
        /*
            This switch case is responsible for freeing the ObjFunction itself as well as any other memory it owns. 
            Functions own their chunk, so we call Chunk’s destructor-like function.
        */
            ObjFunction* function = (ObjFunction*)object;
            freeChunk(&function->chunk);
            FREE(OBJ_FUNCTION, object);
            break;
        }
        case OBJ_NATIVE:    
            FREE(ObjNative, object);
            break;
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, object);
            break;
        }
    }
}

void freeObjects() {
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}
