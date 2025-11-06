#include <stdio.h>
#include <libsys.h>
#include "test_util.h"

enum TestState
{
  TEST_RUNNING,
  TEST_BLOCKED,
  TEST_KILLED
};

typedef struct P_rq
{
  int32_t pid;
  enum TestState TestState;
} p_rq;

extern int endless_loop_wrapper(int argc, char **argv);

int64_t test_processes(uint64_t argc, char *argv[])
{
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes;
  char *argvAux[] = {0};

  if (argc != 1)
    return -1;

  if ((max_processes = satoi(argv[0])) <= 0)
    return -1;

  p_rq p_rqs[max_processes];
  int iteration = 0;

  int default_fds[2] = {0, 1}; // stdin, stdout
  while (1)
  {
    
    alive = 0;

    // Create max_processes processes
    for (rq = 0; rq < max_processes; rq++)
    {
      p_rqs[rq].pid = createProcess("endless_loop", endless_loop_wrapper, 0, argvAux, 2, 0, default_fds); // Background (0)

      if (p_rqs[rq].pid == -1)
      {
        printf("test_processes: ERROR creating process\n");

        // Kill all processes created so far before exiting
        for (int i = 0; i < rq; i++)
        {
          if (p_rqs[i].pid > 0)
          {
            kill(p_rqs[i].pid);
            wait(p_rqs[i].pid, NULL);
          }
        }

        return -1;
      }
      else
      {
        
        p_rqs[rq].TestState = TEST_RUNNING;
        alive++;
      }
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0)
    {

      for (rq = 0; rq < max_processes; rq++)
      {
        action = GetUniform(100) % 2;

        switch (action)
        {
        case 0:
          if (p_rqs[rq].TestState == TEST_RUNNING || p_rqs[rq].TestState == TEST_BLOCKED)
          {
            
            if (kill(p_rqs[rq].pid) == -1)
            {
              printf("test_processes: ERROR killing process PID %d\n", p_rqs[rq].pid);
              return -1;
            }
            //  Wait for the process to free resources
            wait(p_rqs[rq].pid, NULL);
            p_rqs[rq].TestState = TEST_KILLED;
            alive--;
          }
          break;

        case 1:
          if (p_rqs[rq].TestState == TEST_RUNNING)
          {
            
            if (block(p_rqs[rq].pid) == -1)
            {
              printf("test_processes: ERROR blocking process PID %d\n", p_rqs[rq].pid);
              return -1;
            }
            
            p_rqs[rq].TestState = TEST_BLOCKED;
          }
          break;
        }
      }

      // Randomly unblocks processes
      for (rq = 0; rq < max_processes; rq++)
        if (p_rqs[rq].TestState == TEST_BLOCKED && GetUniform(100) % 2)
        {
          
          if (unblock(p_rqs[rq].pid) == -1)
          {
            printf("test_processes: ERROR unblocking process PID %d\n", p_rqs[rq].pid);
            return -1;
          }
          
          p_rqs[rq].TestState = TEST_RUNNING;
        }
    }

    // Wait for system to clean up resources before next iteration
    
  }
}