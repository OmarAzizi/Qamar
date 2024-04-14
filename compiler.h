#ifndef clox_compiler_h
#define clox_compiler_h

#include "value.h"
#include "vm.h"

ObjFunction* compile(const char* source);

#endif
