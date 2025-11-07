// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <stdio.h>
#include "test_util.h"
#include <libsys.h>

#define SEM_ID "test_sync"
#define TOTAL_PAIR_PROCESSES 2

int64_t global; // shared memory

void slowInc(int64_t *p, int64_t inc)
{
  uint64_t aux = *p;
  yield();    // This makes the race condition highly probable , pues en ese intante cede cpu a otros procesos si nhaber incrementado, pero como
  aux += inc; // tiene capturado el sem no pasa nada
  *p = aux;
}
uint64_t process_inc(uint64_t argc, char *argv[])
{
  uint64_t n;
  int8_t inc;
  int8_t use_sem;
  int64_t sem_index;

  if (argc != 3)
    return -1;

  if ((n = satoi(argv[0])) <= 0)
  {
    printf("me meti en error 1");
    return -1;
  }

  if ((inc = satoi(argv[1])) == 0)
  {
    printf("me meti en error 2");
    return -1;
  }

  if ((use_sem = satoi(argv[2])) < 0)
  {
    printf("me meti en error 3");
    return -1;
  }

  if (use_sem)
    if ((sem_index = semOpen(SEM_ID, 1)) < 0)
    {
      printf("test_sync: ERROR opening semaphore\n");
      return -1;
    }

  uint64_t i;
  for (i = 0; i < n; i++)
  {
    if (use_sem)
    {
      semWait(sem_index);
    }
    slowInc(&global, inc);
    if (use_sem)
    {
      semPost(sem_index);
    }
  }

  if (use_sem)
  {
    semClose(sem_index);
  }

  printf("Process %d done, and global = %u\n", getpid(), global);
  return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[])
{ //{n, use_sem, 0}
  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 2)
    return -1;

  char *argvDec[] = {argv[0], "-1", argv[1], NULL};
  char *argvInc[] = {argv[0], "1", argv[1], NULL};

  global = 0;

  uint64_t i;
  int default_fds[2] = {0, 1}; // stdin, stdout
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    // Crear procesos decrementadores
    extern int process_inc_wrapper(int argc, char **argv);
    pids[i] = createProcess("process_inc", process_inc_wrapper, 3, argvDec, 5, 0, default_fds);
    printf("Created decrementing process with PID %d\n", pids[i]);
    // Crear procesos incrementadores
    pids[i + TOTAL_PAIR_PROCESSES] = createProcess("process_inc", process_inc_wrapper, 3, argvInc, 5, 0, default_fds);
    printf("Created incrementing process with PID %d\n", pids[i + TOTAL_PAIR_PROCESSES]);
  } // crea cuatro procesos, dos que incrementan y dos que decrementan

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    wait(pids[i], NULL);
    wait(pids[i + TOTAL_PAIR_PROCESSES], NULL);
  }

  int sem_index = semOpen(SEM_ID, 1);
  semDestroy(sem_index);

  printf("Final value: %u\n", global);

  return 0;
}
