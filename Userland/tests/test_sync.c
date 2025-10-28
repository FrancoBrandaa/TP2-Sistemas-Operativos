#include <stdint.h>
#include <stdio.h>
#include "test_util.h"

#define SEM_ID "test_sync"
#define TOTAL_PAIR_PROCESSES 2

int64_t global; // shared memory

void slowInc(int64_t *p, int64_t inc) {
  uint64_t aux = *p;
  yield(); // This makes the race condition highly probable , pues en ese intante cede cpu a otros procesos si nhaber incrementado, pero como 
  aux += inc; //tiene capturado el sem no pasa nada
  *p = aux;
}

uint64_t my_process_inc(uint64_t argc, char *argv[]) {
  uint64_t n;
  int8_t inc;
  int8_t use_sem;
  int32_t sem_index;

  if (argc != 3)
    return -1;

  if ((n = satoi(argv[0])) <= 0){
    printf("me meti en error 1");
    return -1;
  }
    
  if ((inc = satoi(argv[1])) == 0){
    printf("me meti en error 2");
    return -1;
  }
    
  if ((use_sem = satoi(argv[2])) < 0){
    printf("me meti en error 3");
    return -1;
  }
    

  if (use_sem)
    if ((sem_index = semOpen(SEM_ID, 1)) < 0) { 
      printf("test_sync: ERROR opening semaphore\n");
      return -1;
    }

  uint64_t i;
  for (i = 0; i < n; i++) {
    if (use_sem){
      semWait(sem_index);
    }
    slowInc(&global, inc);
    if (use_sem){
      semPost(sem_index);
    }
      
  }

  if (use_sem){
    semClose(sem_index);
  }
    

  return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[]) { //{n, use_sem, 0}
  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 2)
    return -1;

  char *argvDec[] = {argv[0], "-1", argv[1], NULL};
  char *argvInc[] = {argv[0], "1", argv[1], NULL};

  global = 0;

  uint64_t i;
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    // Crear procesos decrementadores
    pids[i] = createProcess("my_process_inc", (int (*)(int, char **))my_process_inc, 3, argvDec, 1, 0);
    // Crear procesos incrementadores
    pids[i + TOTAL_PAIR_PROCESSES] = createProcess("my_process_inc", (int (*)(int, char **))my_process_inc, 3, argvInc, 1, 0);
  } //crea cuatro procesos, dos que incrementan y dos que decrementan

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    wait(pids[i], NULL);
    wait(pids[i + TOTAL_PAIR_PROCESSES], NULL);
  }

  int sem_index = semOpen(SEM_ID, 1);
  semDestroy(sem_index);

  printf("Final value: %d\n", global);

  return 0;
}
