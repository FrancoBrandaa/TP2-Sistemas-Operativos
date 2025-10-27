#include <stdint.h>
#include <stdio.h>
#include <libsys.h>
#include "test_util.h"

#define TOTAL_PROCESSES 3

#define LOWEST 1  // MIN_PRIORITY in kernel
#define MEDIUM 3  // Middle priority
#define HIGHEST 5 // MAX_PRIORITY in kernel

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

// Use extern - defined in test_util.c
extern uint64_t max_value;
extern void zero_to_max();

uint64_t test_prio(uint64_t argc, char *argv[])
{
    long pids[TOTAL_PROCESSES];
    char *ztm_argv[] = {0};
    uint64_t i;

    if (argc != 1)
        return -1;

    if ((max_value = satoi(argv[0])) <= 0)
        return -1;

    printf("SAME PRIORITY...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
        pids[i] = my_create_process("zero_to_max", 0, ztm_argv);

    // Expect to see them finish at the same time

    for (i = 0; i < TOTAL_PROCESSES; i++)
        my_wait(pids[i]);

    printf("SAME PRIORITY, THEN CHANGE IT...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
    {
        pids[i] = my_create_process("zero_to_max", 0, ztm_argv);
        my_nice(pids[i], prio[i]);
        printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
    }

    // Expect the priorities to take effect

    for (i = 0; i < TOTAL_PROCESSES; i++)
        my_wait(pids[i]);
    //espera a que terminen los procesos y luego imprime el mensaje
    printf("SAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
    {
        pids[i] = my_create_process("zero_to_max", 0, ztm_argv);
        my_block(pids[i]);
        my_nice(pids[i], prio[i]);
        printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
    }

    for (i = 0; i < TOTAL_PROCESSES; i++){
        my_unblock(pids[i]);
        printf("  PROCESS %d UNBLOCKED\n", pids[i]);
    }
      

    // Expect the priorities to take effect

    for (i = 0; i < TOTAL_PROCESSES; i++)
        my_wait(pids[i]);

    printf("TEST_PRIO COMPLETED!\n");
    return 0;
}
