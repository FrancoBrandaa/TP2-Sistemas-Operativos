#include "include/MemoryManagerLib.h"
#include "include/MemoryManager.h"

// ============================================================================
// FUNCIONES AUXILIARES PARA CÁLCULOS
// ============================================================================

int calculateOrder(size_t size) {
    // Agregar espacio para el header
    size_t totalSize = size + sizeof(BlockHeader);
    
    int order = 0;
    size_t blockSize = 1;
    
    // Encontrar la potencia de 2 más pequeña >= totalSize
    while (blockSize < totalSize) {
        blockSize <<= 1;  // blockSize *= 2
        order++;
    }
    
    return order;
}

// ============================================================================
// FUNCIONES AUXILIARES PARA MANEJO DE BLOQUES
// ============================================================================

void splitBlock(MemoryManagerADT mm, int order) {
    // Tomar un bloque del orden superior
    FreeBlock *largeBlock = mm->freeLists[order + 1];
    if (largeBlock == NULL) {
        return; // No hay bloques para dividir
    }
    
    // Quitarlo de la lista del orden superior
    mm->freeLists[order + 1] = largeBlock->next;
    if (largeBlock->next) {
        largeBlock->next->prev = NULL;
    }
    
    // Dividirlo en dos bloques del orden inferior
    size_t blockSize = 1 << order; // 2^order
    
    FreeBlock *buddy1 = largeBlock;
    FreeBlock *buddy2 = (FreeBlock *)((char *)largeBlock + blockSize);
    
    // Agregar ambos buddies a la lista del orden inferior
    buddy1->next = buddy2;
    buddy1->prev = NULL;
    buddy2->next = mm->freeLists[order];
    buddy2->prev = buddy1;
    
    if (mm->freeLists[order]) {
        mm->freeLists[order]->prev = buddy2;
    }
    
    mm->freeLists[order] = buddy1;
}

void *findBuddy(void *block, int order) {
    // El buddy está a distancia 2^order del bloque actual
    size_t blockSize = 1 << order;
    size_t blockAddress = (size_t)block;
    
    // Si la dirección del bloque es divisible por 2^(order+1), 
    // entonces el buddy está hacia adelante, sino hacia atrás
    if ((blockAddress / blockSize) % 2 == 0) {
        return (void *)(blockAddress + blockSize);  // Buddy hacia adelante
    } else {
        return (void *)(blockAddress - blockSize);  // Buddy hacia atrás
    }
}

void mergeWithBuddy(MemoryManagerADT mm, void *block, int order) {
    // No intentar hacer merge si ya estamos en el orden máximo
    if (order >= mm->maxOrder) {
        return;
    }
    
    // Encontrar el buddy de este bloque
    void *buddy = findBuddy(block, order);
    
    // Verificar que el buddy esté dentro del heap
    if ((char *)buddy < mm->heapStart || (char *)buddy >= mm->heapEnd) {
        return;
    }
    
    // Verificar si el buddy está libre (buscarlo en la lista libre del mismo orden)
    FreeBlock *current = mm->freeLists[order];
    int buddyIsFree = 0;
    
    while (current != NULL) {
        if (current == (FreeBlock *)buddy) {
            buddyIsFree = 1;
            break;
        }
        current = current->next;
    }
    
    // Si el buddy está libre, hacer merge
    if (buddyIsFree) {
        // Quitar el buddy de su lista libre actual
        removeFromFreeList(mm, buddy, order);
        
        // Quitar el bloque actual de su lista libre
        removeFromFreeList(mm, block, order);
        
        // Determinar cuál bloque viene primero en memoria (será el bloque merged)
        void *mergedBlock = (block < buddy) ? block : buddy;
        
        // Agregar el bloque merged a la lista del siguiente orden
        addToFreeList(mm, mergedBlock, order + 1);
        
        // Intentar hacer merge recursivamente con el nuevo buddy
        mergeWithBuddy(mm, mergedBlock, order + 1);
    }
}

// ============================================================================
// FUNCIONES AUXILIARES PARA LISTAS ENLAZADAS
// ============================================================================

void removeFromFreeList(MemoryManagerADT mm, void *block, int order) {
    FreeBlock *freeBlock = (FreeBlock *)block;
    
    // Si es el primer elemento de la lista
    if (mm->freeLists[order] == freeBlock) {
        mm->freeLists[order] = freeBlock->next;
    }
    
    // Actualizar enlaces
    if (freeBlock->prev) {
        freeBlock->prev->next = freeBlock->next;
    }
    if (freeBlock->next) {
        freeBlock->next->prev = freeBlock->prev;
    }
}

void addToFreeList(MemoryManagerADT mm, void *block, int order) {
    FreeBlock *freeBlock = (FreeBlock *)block;
    
    // Agregar al inicio de la lista
    freeBlock->next = mm->freeLists[order];
    freeBlock->prev = NULL;
    
    if (mm->freeLists[order]) {
        mm->freeLists[order]->prev = freeBlock;
    }
    
    mm->freeLists[order] = freeBlock;
}

// ============================================================================
// FUNCIONES AUXILIARES PARA DEBUGGING/VALIDACIÓN
// ============================================================================

int isValidBlock(MemoryManagerADT mm, void *block, int order) {
    // Verificar que el bloque esté dentro del heap
    if ((char *)block < mm->heapStart || (char *)block >= mm->heapEnd) {
        return 0;
    }
    
    // Verificar alineación
    size_t blockSize = 1 << order;
    if (((size_t)block - (size_t)mm->heapStart) % blockSize != 0) {
        return 0;
    }
    
    return 1;
}

void printFreeListsState(MemoryManagerADT mm) {
    // TODO: Implementar cuando tengamos printf en el kernel
    (void)mm;
}

// Nueva función: Validar que un puntero de usuario sea válido
int isValidUserPointer(MemoryManagerADT mm, void *ptr) {
    if (ptr == NULL) {
        return 0;
    }
    
    // Obtener el header
    BlockHeader *header = ((BlockHeader *)ptr) - 1;
    
    // Verificar que el header esté dentro del heap
    if ((char *)header < mm->heapStart || (char *)header >= mm->heapEnd) {
        return 0;
    }
    
    // Verificar el número mágico
    if (header->magic != BLOCK_MAGIC) {
        return 0;
    }
    
    // Verificar que el orden sea válido
    if (header->order < mm->minOrder || header->order > mm->maxOrder) {
        return 0;
    }
    
    return 1;
}
