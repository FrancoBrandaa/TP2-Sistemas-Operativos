#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <stdlib.h>

// Definiciones para Buddy System
#define MAX_ORDER 20  // Máximo orden (2^20 = 1MB bloques máximos)
#define MIN_BLOCK_SIZE 64  // Tamaño mínimo de bloque (64 bytes para que quepa el header)

// Header que va antes de cada bloque asignado
typedef struct BlockHeader {
    size_t size;        // Tamaño real del bloque asignado
    int order;          // Orden del bloque en Buddy System
    int magic;          // Número mágico para validación (0xDEADBEEF)
} BlockHeader;

// Nodo para las listas libres
typedef struct FreeBlock {
    struct FreeBlock *next; 
    struct FreeBlock *prev;
} FreeBlock;

#define BLOCK_MAGIC 0xDEADBEEF  // Para detectar corrupción

// Estructura principal del Memory Manager (expuesta para las funciones auxiliares)
typedef struct MemoryManagerCDT {
    // Información básica del heap
    char *heapStart;        // Dirección de inicio del heap (después de esta estructura)
    char *heapEnd;          // Dirección final del heap (no inclusive)
    size_t totalSize;       // Tamaño total gestionado (heapEnd - heapStart)

    // Listas libres por orden (Buddy System)
    FreeBlock *freeLists[MAX_ORDER + 1];
    int minOrder;           // Orden mínimo permitido
    int maxOrder;           // Orden máximo permitido

    // Contadores de estado
    size_t usedMemory;      // Memoria total ocupada (incluye headers y padding del buddy)
    size_t freeMemory;      // Memoria libre restante
} MemoryManagerCDT, *MemoryManagerADT;

MemoryManagerADT createMemoryManager(char *heapStart, 
                                    char *heapEnd, 
                                    int minOrder, 
                                    int maxOrder);

void *allocMemory(MemoryManagerADT const restrict memoryManager, const size_t memoryToAllocate);
void freeMemory(MemoryManagerADT const restrict memoryManager, void *ptr);

// Consultas de estado
size_t getFreeMemory(MemoryManagerADT mm);
size_t getUsedMemory(MemoryManagerADT mm);


#endif

/*
- poder consultar el estado de la memoria gestionada
- poder liberar toda la memoria gestionada
- designacion de memoria (buddy y otro propio)
-dependiendo de la compilacion usamos un designador u otro, alternar entre amobos de forma transparente 
*/
