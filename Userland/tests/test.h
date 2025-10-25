#ifndef TEST_H
#define TEST_H

#include <stdint.h>
#include "../tests/test_util.h"

// Declaraciones de funciones de test_mm.c
//uint64_t test_mm(uint64_t argc, char *argv[]);

// Declaraciones de funciones de test_prio.c
//uint64_t test_prio(void);

// Declaraciones de funciones de test_processes.c (original de la catedra)
int64_t test_processes(uint64_t argc, char *argv[]);

// Declaraciones de funciones de my_test_processes.c (version mejorada)
int64_t my_test_processes(uint64_t argc, char *argv[]);

// Declaraciones de funciones de test_sync.c
//uint64_t test_sync(uint64_t argc, char *argv[]);

// Declaraciones de funciones de test_memory_status.c
int test_memory_status(int argc, char *argv[]);

#endif // TEST_H