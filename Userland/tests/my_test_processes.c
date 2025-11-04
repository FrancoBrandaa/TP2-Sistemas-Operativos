#include <stdio.h>
#include <libsys.h>
#include "test_util.h"

int endless_loop_print_wrapper(int argc, char *argv[])
{
  uint64_t wait = 100000000; // default value
  if (argc > 0)
  {
    wait = satoi(argv[0]);
  }
  endless_loop_print(wait);
  return 0;
}

enum TestProcessState
{
  TEST_RUNNING,
  TEST_BLOCKED,
  TEST_KILLED
};

typedef struct P_rq
{
  int32_t pid;
  enum TestProcessState state;
} p_rq;

int64_t my_test_processes(uint64_t argc, char *argv[])
{
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes;
  char *argvAux[] = {"100000000"}; // Wait parameter para endless_loop_print

  if (argc != 1)
    return -1;

  if ((max_processes = satoi(argv[0])) <= 0)
    return -1;

  p_rq p_rqs[max_processes];

  int round = 0;

  while (1)
  {
    round++;
    printf("\n\e[1;36m========== ROUND %d ==========\e[0m\n", round);
    printf("Creating %d processes...\n", max_processes);

    // Create max_processes processes
    for (rq = 0; rq < max_processes; rq++)
    {
      int fds[2] = {0, 1}; // STDIN, STDOUT
      p_rqs[rq].pid = createProcess("endless_loop", endless_loop_print_wrapper, 1, argvAux, 1, 0, fds);
      if (p_rqs[rq].pid == -1)
      {
        printf("\e[0;31mERROR creating process\e[0m\n");
        return -1;
      }
      else
      {
        p_rqs[rq].state = TEST_RUNNING;
        alive++;
        printf("  \e[0;32m[+]\e[0m Created process with PID: %d (RUNNING)\n", p_rqs[rq].pid);
      }
    }

    printf("\n\e[1;33mProcesses are printing their PIDs...\e[0m\n");
    bussy_wait(500000000); // Dar tiempo para ver los procesos imprimiendo

    printf("\n\e[1;35mStarting random operations (block/unblock/kill)...\e[0m\n\n");

    int iteration = 0;
    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0)
    {
      iteration++;

      for (rq = 0; rq < max_processes; rq++)
      {
        action = GetUniform(100) % 3; // Cambiado a 3 opciones: kill, block, nada

        switch (action)
        {
        case 0: // KILL
          if (p_rqs[rq].state == TEST_RUNNING || p_rqs[rq].state == TEST_BLOCKED)
          {
            printf("  \e[0;31m[KILL]\e[0m    PID %d\n", p_rqs[rq].pid);
            if (kill(p_rqs[rq].pid) == -1)
            {
              printf("\e[0;31mERROR killing process %d\e[0m\n", p_rqs[rq].pid);
              return -1;
            }
            p_rqs[rq].state = TEST_KILLED;
            alive--;
          }
          break;

        case 1: // BLOCK
          if (p_rqs[rq].state == TEST_RUNNING)
          {
            printf("  \e[0;33m[BLOCK]\e[0m   PID %d\n", p_rqs[rq].pid);
            if (block(p_rqs[rq].pid) == -1)
            {
              printf("\e[0;31mERROR blocking process %d\e[0m\n", p_rqs[rq].pid);
              return -1;
            }
            p_rqs[rq].state = TEST_BLOCKED;
          }
          break;

        case 2: // NADA - solo para que no todo sea tan rápido
          break;
        }
      }

      // Randomly unblocks processes
      for (rq = 0; rq < max_processes; rq++)
      {
        if (p_rqs[rq].state == TEST_BLOCKED && GetUniform(100) % 2)
        {
          printf("  \e[0;32m[UNBLOCK]\e[0m PID %d\n", p_rqs[rq].pid);
          if (unblock(p_rqs[rq].pid) == -1)
          {
            printf("\e[0;31mERROR unblocking process %d\e[0m\n", p_rqs[rq].pid);
            return -1;
          }
          p_rqs[rq].state = TEST_RUNNING;
        }
      }

      // Pequeño delay entre iteraciones para poder ver qué pasa
      bussy_wait(100000000);
    }

    printf("\n\e[0;31mAll processes killed. Alive: %d\e[0m\n", alive);
    printf("\e[1;33mWaiting before next round...\e[0m\n");
    bussy_wait(1000000000); // Pausa antes del siguiente round
  }
}