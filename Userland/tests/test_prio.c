// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <stdio.h>

#include <libsys.h>
#include "test_util.h"

#define TOTAL_PROCESSES 3

#define LOWEST 2
#define MEDIUM 3
#define HIGHEST 4

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

extern uint64_t max_value; // Declared in test_util.c

extern int zero_to_max_wrapper(int argc, char **argv);

uint64_t test_prio(uint64_t argc, char *argv[])
{
  int fds[2];
  getFD(fds);
  int output_fd = fds[1];
  
  int64_t pids[TOTAL_PROCESSES];
  char *ztm_argv[] = {0};
  uint64_t i;

  if (argc != 1)
    return -1;

  if ((max_value = satoi(argv[0])) <= 0)
    return -1;

  fprintf(output_fd, "SAME PRIORITY...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = createProcess("zero_to_max", zero_to_max_wrapper, 0, ztm_argv, 1, 1, fds);

  // Expect to see them finish at the same time

  for (i = 0; i < TOTAL_PROCESSES; i++)
    wait(pids[i], NULL);

  fprintf(output_fd, "SAME PRIORITY, THEN CHANGE IT...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
  {
    pids[i] = createProcess("zero_to_max", zero_to_max_wrapper, 0, ztm_argv, 1, 1, fds);
    nice(pids[i], prio[i]);
    fprintf(output_fd, "  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
  }

  // Expect the priorities to take effect

  for (i = 0; i < TOTAL_PROCESSES; i++)
    wait(pids[i], NULL);

  fprintf(output_fd, "SAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
  {
    pids[i] = createProcess("zero_to_max", zero_to_max_wrapper, 0, ztm_argv, 1, 1, fds);
    block(pids[i]);
    nice(pids[i], prio[i]);
    fprintf(output_fd, "  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
  }

  for (i = 0; i < TOTAL_PROCESSES; i++)
    unblock(pids[i]);

  // Expect the priorities to take effect

  for (i = 0; i < TOTAL_PROCESSES; i++)
    wait(pids[i], NULL);

  return 0;
}