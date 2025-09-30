#ifndef MEMORY_MANAGER_LIB_H
#define MEMORY_MANAGER_LIB_H

#include "MemoryManager.h"
#include <stddef.h>

// Funciones auxiliares para cálculos
int calculateOrder(size_t size);

// Funciones auxiliares para manejo de bloques
void splitBlock(MemoryManagerADT mm, int order);
void *findBuddy(void *block, int order);
void mergeWithBuddy(MemoryManagerADT mm, void *block, int order);

// Funciones auxiliares para listas enlazadas
void removeFromFreeList(MemoryManagerADT mm, void *block, int order);
void addToFreeList(MemoryManagerADT mm, void *block, int order);

// Funciones auxiliares para debugging/validación
int isValidBlock(MemoryManagerADT mm, void *block, int order);
int isValidUserPointer(MemoryManagerADT mm, void *ptr);
void printFreeListsState(MemoryManagerADT mm);

#endif
