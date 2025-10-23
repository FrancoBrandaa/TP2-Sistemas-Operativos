#include <libsys/libsys.h>
#include <libc/stdio.h>
#include "../include/test.h"

// Definición de MemoryStatus (copiada de defs.h del kernel)
typedef struct {
    uint32_t total;  
    uint32_t used;   
    uint32_t free;    
    void    *base;    
    void    *end;     
} MemoryStatus;

int test_memory_status(int argc, char *argv[]) {
    MemoryStatus memStatus;
    
    printf("=== Test Memory Status ===\n");
    
    // Llamar la syscall para obtener el estado de memoria
    int result = getMemoryStatus(&memStatus);
    
    if (result == 0) {
        printf("Memory Status:\n");
        printf("  Total: %d bytes\n", memStatus.total);
        printf("  Used:  %d bytes\n", memStatus.used);
        printf("  Free:  %d bytes\n", memStatus.free);
        printf("  Base:  %p\n", memStatus.base);
        printf("  End:   %p\n", memStatus.end);
        printf("  Usage: %d%%\n", (memStatus.used * 100) / memStatus.total);
        
        // Test allocation impact
        printf("\n--- Testing allocation impact ---\n");
        void *ptr1 = malloc(1024);
        printf("Allocated 1024 bytes at %p\n", ptr1);
        
        getMemoryStatus(&memStatus);
        printf("After allocation:\n");
        printf("  Used:  %d bytes\n", memStatus.used);
        printf("  Free:  %d bytes\n", memStatus.free);
        
        free(ptr1);
        printf("Freed pointer\n");
        
        getMemoryStatus(&memStatus);
        printf("After freeing:\n");
        printf("  Used:  %d bytes\n", memStatus.used);
        printf("  Free:  %d bytes\n", memStatus.free);
        
        return 0;
    } else {
        printf("Error getting memory status: %d\n", result);
        return -1;
    }
}