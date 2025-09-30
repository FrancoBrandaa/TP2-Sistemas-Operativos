#include "include/MemoryManager.h"
#include "include/MemoryManagerLib.h"

// La definición completa de MemoryManagerCDT se movió al header para que
// los helpers en MemoryManagerLib.c puedan acceder a sus campos.


MemoryManagerADT createMemoryManager(char *heapStart, 
                                    char *heapEnd, 
                                    int minOrder, 
                                    int maxOrder) {
	// Reservar espacio para la estructura de control AL INICIO del heap
	MemoryManagerADT memoryManager = (MemoryManagerADT) heapStart;
	
	// Ajustar heapStart para que empiece después de la estructura de control
	heapStart += sizeof(MemoryManagerCDT);
	memoryManager->heapStart = heapStart;
	memoryManager->heapEnd = heapEnd;
	memoryManager->totalSize = heapEnd - heapStart;
	memoryManager->minOrder = minOrder;
	memoryManager->maxOrder = maxOrder; 
	
	// Inicializar las listas libres como vacias (solo las que usamos)
	for (int i = minOrder; i <= maxOrder; i++) {
		memoryManager->freeLists[i] = NULL;
	}
	
	// Inicializar contadores
	memoryManager->usedMemory = 0;
	memoryManager->freeMemory = heapEnd - heapStart;
	
	// PASO CRÍTICO: Inicializar el heap con un bloque libre gigante
	size_t heapSize = heapEnd - heapStart;
	int initialOrder = calculateOrder(heapSize);
	
	// Ajustar al orden máximo si el heap es muy grande
	if (initialOrder > maxOrder) {
		initialOrder = maxOrder;
	}
	
	// Crear el bloque libre inicial en el heap
	FreeBlock *initialBlock = (FreeBlock *)heapStart;
	initialBlock->next = NULL;
	initialBlock->prev = NULL;
	
	// Agregarlo a la lista libre del orden apropiado
	memoryManager->freeLists[initialOrder] = initialBlock;
	
	return memoryManager;
}

size_t getFreeMemory(MemoryManagerADT mm) {
    return mm ? mm->freeMemory : 0;
}

size_t getUsedMemory(MemoryManagerADT mm) {
    return mm ? mm->usedMemory : 0;
}



void *allocMemory(MemoryManagerADT const restrict memoryManager, const size_t memoryToAllocate) {
    // Paso 1: Calcular el orden necesario
    int requiredOrder = calculateOrder(memoryToAllocate);
    
    // Verificar que esté dentro del rango válido
    if (requiredOrder < memoryManager->minOrder) {
        requiredOrder = memoryManager->minOrder;
    }
    if (requiredOrder > memoryManager->maxOrder) {
        return NULL; // Pedido demasiado grande
    }
    
    // Paso 2: Buscar un bloque libre del orden exacto o superior
    int searchOrder = requiredOrder;
    while (searchOrder <= memoryManager->maxOrder && 
           memoryManager->freeLists[searchOrder] == NULL) {
        searchOrder++;
    }
    
    // Si no encontramos ningún bloque libre
    if (searchOrder > memoryManager->maxOrder) {
        return NULL; // Sin memoria disponible
    }
    
    // Paso 3: Dividir bloques si es necesario
    while (searchOrder > requiredOrder) {
        splitBlock(memoryManager, searchOrder - 1);
        searchOrder--;
    }
    
    // Paso 4: Tomar el bloque de la lista libre
    FreeBlock *rawBlock = memoryManager->freeLists[requiredOrder];
    memoryManager->freeLists[requiredOrder] = rawBlock->next;
    
    if (rawBlock->next) {
        rawBlock->next->prev = NULL;
    }
    
    // Paso 5: Crear el header al inicio del bloque
    BlockHeader *header = (BlockHeader *)rawBlock;
    header->size = memoryToAllocate;        // Tamaño que pidió el usuario
    header->order = requiredOrder;          // Orden real del bloque
    header->magic = BLOCK_MAGIC;           // Validación
    
    // Paso 6: Actualizar contadores
    size_t allocatedSize = 1 << requiredOrder; // 2^requiredOrder
    memoryManager->usedMemory += allocatedSize;
    memoryManager->freeMemory -= allocatedSize;
    
    // Paso 7: Devolver puntero después del header
    return (void *)(header + 1);
}

void freeMemory(MemoryManagerADT const restrict memoryManager, void *ptr) {
    if (ptr == NULL) {
        return; // No hacer nada si el puntero es NULL
    }
    
    // Paso 1: Obtener el header (está justo antes del puntero del usuario)
    BlockHeader *header = ((BlockHeader *)ptr) - 1;
    
    // Paso 2: Validar el header
    if (header->magic != BLOCK_MAGIC) {
        return; // Header corrupto o puntero inválido
    }
    
    // Verificar que el bloque esté dentro del heap
    if ((char *)header < memoryManager->heapStart || 
        (char *)header >= memoryManager->heapEnd) {
        return; // Puntero fuera del heap
    }
    
    // Paso 3: Obtener información real del bloque
    int order = header->order;
    size_t blockSize = 1 << order;
    
    // Paso 4: Limpiar el header (marcar como libre)
    header->magic = 0;  // Invalidar para detectar double-free
    
    // Paso 5: Convertir el bloque de vuelta a FreeBlock
    FreeBlock *freeBlock = (FreeBlock *)header;
    
    // Paso 6: Agregar a la lista libre
    addToFreeList(memoryManager, freeBlock, order);
    
    // Paso 7: Intentar hacer merge con el buddy
    mergeWithBuddy(memoryManager, freeBlock, order);
    
    // Paso 8: Actualizar contadores
    memoryManager->usedMemory -= blockSize;
    memoryManager->freeMemory += blockSize;
}
