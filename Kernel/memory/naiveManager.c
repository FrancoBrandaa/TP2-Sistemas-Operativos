// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * NAIVE MEMORY MANAGER
 * ====================
 * Implementa un algoritmo simple de gestión de memoria basado en:
 * - Lista doblemente enlazada de bloques
 * - Algoritmo First-Fit para asignación
 * - Coalescencia (fusión) automática de bloques libres adyacentes
 * 
 * Características:
 * - Complejidad O(n) para asignación (debe recorrer la lista)
 * - Complejidad O(1) para liberación + coalescencia
 * - Fragmentación externa moderada (se mitiga con coalescencia)
 * - Overhead: ~17 bytes por bloque (header + alineación)
 */

#include "../include/memoryManager.h"
#include <stdint.h>

/*
 * Estructura que representa cada bloque de memoria en el heap
 * Cada bloque contiene un header seguido del área de datos del usuario
 * 
 * Layout en memoria:
 * [Block Header][User Data][Block Header][User Data]...
 */
typedef struct Block {
    uint32_t       size;    // Tamaño del área de datos (sin contar este header)
    uint8_t        free;    // Estado: 1 = libre, 0 = ocupado
    struct Block  *next;    // Puntero al siguiente bloque en la lista enlazada
    struct Block  *prev;    // Puntero al bloque anterior (lista doblemente enlazada)
} Block;

// Variables globales que mantienen el estado del memory manager
static Block   *firstBlock = NULL;    // Puntero al primer bloque de la lista enlazada
static uint32_t memoryPoolSize = 0;   // Tamaño total del pool de memoria gestionado

// Macros auxiliares para cálculos y conversiones
#define BLOCK_HEADER_SIZE  ((uint32_t)ALIGN(sizeof(Block)))  // Tamaño alineado del header
#define TO_BYTE_PTR(ptr)   ((uint8_t *)(ptr))               // Convierte puntero a bytes

/*
 * Macro que calcula la dirección física del siguiente bloque contiguo en memoria
 * Se usa para verificar si dos bloques están físicamente adyacentes (para fusión)
 */
#define NEXT_PHYSICAL_BLOCK(block) \
    ((Block *)(TO_BYTE_PTR(block) + BLOCK_HEADER_SIZE + (block)->size))

/*
 * Determina si un bloque puede dividirse sin crear fragmentos demasiado pequeños
 * Un bloque se puede dividir si queda espacio suficiente para:
 * - Los datos solicitados
 * - Un nuevo header de bloque
 * - Alineación mínima
 */
static uint32_t hasRoomForSplit(Block *block, uint32_t requestedSize) {
    return block->size >= requestedSize + BLOCK_HEADER_SIZE + ALIGNMENT;
}

/*
 * INICIALIZACIÓN DEL MEMORY MANAGER
 * =================================
 * Configura el heap inicial con un único bloque libre que ocupa toda la memoria.
 * Este bloque se irá dividiendo a medida que se hagan asignaciones.
 */
void createMemoryManager(void *memoryStartAddress, uint32_t memorySize) {
    // Validaciones básicas
    if (!memoryStartAddress || memorySize <= BLOCK_HEADER_SIZE)
        return;

    // Inicializa el primer bloque en el inicio del heap
    firstBlock = (Block *)memoryStartAddress;
    firstBlock->size = memorySize - BLOCK_HEADER_SIZE;  // Resta espacio del header
    firstBlock->free = 1;                               // Marca como libre
    firstBlock->next = NULL;                            // No hay más bloques aún
    firstBlock->prev = NULL;                            // Es el primer bloque

    memoryPoolSize = memorySize;                        // Guarda tamaño total
}


/*
 * BÚSQUEDA DE BLOQUE ADECUADO (First-Fit)
 * Recorre la lista buscando el primer bloque libre suficientemente grande.
 */
static Block *findSuitableBlock(uint32_t requestedSize) {
    // Recorre la lista enlazada desde el primer bloque
    for (Block *block = firstBlock; block; block = block->next)
        if (block->free && block->size >= requestedSize)  // Libre Y suficientemente grande
            return block;
    return NULL;  // No encontró ningún bloque adecuado
}

/*
 * DIVISIÓN DE BLOQUES
 * Divide un bloque grande en dos: uno del tamaño solicitado y otro con el sobrante.
 */
static void splitBlock(Block *block, uint32_t requestedSize) {
    // Calcula dónde comenzará el nuevo bloque (después de los datos actuales)
    Block *newBlock = (Block *)(TO_BYTE_PTR(block) + BLOCK_HEADER_SIZE + requestedSize);

    // Configura el nuevo bloque con el espacio sobrante
    newBlock->size = block->size - requestedSize - BLOCK_HEADER_SIZE;  // Resta el espacio usado
    newBlock->free = 1;                    // El nuevo bloque queda libre
    newBlock->next = block->next;          // Hereda las conexiones del bloque original
    newBlock->prev = block;                // Su anterior es el bloque que se dividió

    // Actualiza los enlaces de la lista enlazada
    if (block->next)
        block->next->prev = newBlock;      // El siguiente ahora apunta al nuevo bloque

    block->next = newBlock;                // El bloque original apunta al nuevo
    block->size = requestedSize;           // Reduce el tamaño del bloque original
}

/*
 * ASIGNACIÓN DE MEMORIA
 * Busca un bloque libre, lo divide si es necesario y lo marca como ocupado.
 */
void *allocMemory(uint32_t size) {
    if (size == 0)
        return NULL;

    uint32_t alignedSize = ALIGN(size);              // Alinea el tamaño para optimizar accesos
    Block *block = findSuitableBlock(alignedSize);  // Busca un bloque libre
    if (!block)
        return NULL;                                 // No hay memoria disponible

    // Si el bloque es mucho más grande que lo necesario, lo divide
    if (hasRoomForSplit(block, alignedSize))
        splitBlock(block, alignedSize);

    block->free = 0;                                 // Marca el bloque como ocupado
    return TO_BYTE_PTR(block) + BLOCK_HEADER_SIZE;   // Retorna puntero a los datos (después del header)
}

/*
 * FUSIÓN CON BLOQUE SIGUIENTE
 * Une el bloque actual con el siguiente si ambos están libres y son adyacentes.
 */
static void mergeWithNext(Block *block) {
    Block *nextBlock = block->next;
    if (nextBlock && nextBlock->free && nextBlock == NEXT_PHYSICAL_BLOCK(block)) {
        block->size += BLOCK_HEADER_SIZE + nextBlock->size;
        block->next  = nextBlock->next;
        if (nextBlock->next)
            nextBlock->next->prev = block;
    }
}

/*
 * FUSIÓN CON BLOQUE ANTERIOR
 * Une el bloque anterior con el actual si ambos están libres y son adyacentes.
 */
static void mergeWithPrevious(Block *block) {
    Block *previousBlock = block->prev;
    if (previousBlock && previousBlock->free && block == NEXT_PHYSICAL_BLOCK(previousBlock)) {
        previousBlock->size += BLOCK_HEADER_SIZE + block->size;
        previousBlock->next  = block->next;
        if (block->next)
            block->next->prev = previousBlock;
    }
}

/*
 * COALESCENCIA
 * Intenta fusionar el bloque con sus vecinos libres (previo y siguiente).
 */
static void coalesce(Block *block) {
    mergeWithNext(block);
    mergeWithPrevious(block);
}

/*
 * LIBERACIÓN DE MEMORIA
 * Marca el bloque como libre e intenta fusionarlo con bloques adyacentes.
 */
void freeMemory(void *memorySegment) {
    if (!memorySegment)
        return;

    Block *block = (Block *)(TO_BYTE_PTR(memorySegment) - BLOCK_HEADER_SIZE);
    if (block->free) return;

    block->free = 1;
    coalesce(block);
}


/*
 * ESTADÍSTICAS DE MEMORIA
 * Recorre la lista y calcula el uso actual de memoria.
 */
void getMemoryStatus(MemoryStatus *status) {
    if (!status)
        return;

    uint32_t usedBytes = 0;
    uint32_t freeBytes = 0;

    for (Block *block = firstBlock; block; block = block->next) {
        if (block->free)
            freeBytes += block->size;
        else
            usedBytes += block->size;
    }

    status->total = memoryPoolSize;
    status->used  = usedBytes;
    status->free  = freeBytes;

    status->base  = (void *)firstBlock;
    status->end   = (void *)(TO_BYTE_PTR(firstBlock) + memoryPoolSize);
}

