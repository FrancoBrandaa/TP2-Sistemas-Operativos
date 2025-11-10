// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <stdio.h>
#include "libsys.h"
#include "test_util.h"

// Default values for test commands
#define DEFAULT_TEST_MM_MEMORY "1024"   // 1 KB
#define DEFAULT_TEST_PRIO_VALUE "1000000" // 1000000 iterations
#define DEFAULT_TEST_PROCESSES_MAX "5"  // 5 processes

#define DEFAULT_TEST_SYNC_ITERS "1000" // 1000 iterations
#define DEFAULT_TEST_SYNC_USE_SEM "1"  // Use semaphore by default

// Random
static uint32_t m_z = 362436069;
static uint32_t m_w = 521288629;

uint32_t GetUint()
{
  m_z = 36969 * (m_z & 65535) + (m_z >> 16);
  m_w = 18000 * (m_w & 65535) + (m_w >> 16);
  return (m_z << 16) + m_w;
}

uint32_t GetUniform(uint32_t max)
{
  uint32_t u = GetUint();
  return (u + 1.0) * 2.328306435454494e-10 * max;
}

// Memory
uint8_t memcheck(void *start, uint8_t value, uint32_t size)
{
  uint8_t *p = (uint8_t *)start;
  uint32_t i;

  for (i = 0; i < size; i++, p++)
    if (*p != value)
      return 0;

  return 1;
}

// Parameters
int64_t satoi(char *str)
{
  uint64_t i = 0;
  int64_t res = 0;
  int8_t sign = 1;

  if (!str)
    return 0;

  if (str[i] == '-')
  {
    i++;
    sign = -1;
  }

  for (; str[i] != '\0'; ++i)
  {
    if (str[i] < '0' || str[i] > '9')
      return 0;
    res = res * 10 + str[i] - '0';
  }

  return res * sign;
}

// Dummies
void bussy_wait(uint64_t n)
{
  uint64_t i;
  for (i = 0; i < n; i++)
    ;
}

void endless_loop()
{
  while (1)
    ;
}

// Global variable for test_prio
uint64_t max_value = 0;

void zero_to_max()
{
  int fds[2];
  getFD(fds);
  int output_fd = fds[1];
  
  uint64_t value = 0;
  fprintf(output_fd, "PROCESS %d STARTED! (counting to %d)\n", getpid(), max_value);
  while (value++ != max_value)
    ;
  fprintf(output_fd, "PROCESS %d DONE!\n", getpid());
}

int zero_to_max_wrapper(int argc, char **argv)
{
  zero_to_max();
  return 0;
}

// Wrapper for endless_loop
int endless_loop_wrapper(int argc, char **argv)
{
  endless_loop();
  return 0;
}

// Wrapper for test_mm
int test_mm_wrapper(int argc, char **argv)
{
  extern uint64_t test_mm(uint64_t argc, char *argv[]);

  char *test_argv[1];

  if (argc < 2)
  {
    int fds[2];
    getFD(fds);
    int output_fd = fds[1];
    // No arguments, use default
    test_argv[0] = DEFAULT_TEST_MM_MEMORY;
    fprintf(output_fd, "test_mm: Using default max_memory = %s bytes\n", DEFAULT_TEST_MM_MEMORY);
    return (int)test_mm(1, test_argv);
  }

  // Skip command name: argv[0] is "test_mm", so pass &argv[1]
  return (int)test_mm((uint64_t)(argc - 1), &argv[1]);
}

// Wrapper for test_prio
int test_prio_wrapper(int argc, char **argv)
{
  extern uint64_t test_prio(uint64_t argc, char *argv[]);

  char *test_argv[1];

  if (argc < 2)
  {
    int fds[2];
    getFD(fds);
    int output_fd = fds[1];
    // No arguments, use default
    test_argv[0] = DEFAULT_TEST_PRIO_VALUE;
    fprintf(output_fd, "test_prio: Using default max_value = %s\n", DEFAULT_TEST_PRIO_VALUE);
    return (int)test_prio(1, test_argv);
  }

  return (int)test_prio((uint64_t)(argc - 1), &argv[1]);
}

// Wrapper for test_sync
int test_sync_wrapper(int argc, char **argv)
{
  extern uint64_t test_sync(uint64_t argc, char *argv[]);

  char *default_args[2];

  if (argc < 3)
  {
    int fds[2];
    getFD(fds);
    int output_fd = fds[1];
    // Not enough arguments, use defaults
    default_args[0] = DEFAULT_TEST_SYNC_ITERS;
    default_args[1] = DEFAULT_TEST_SYNC_USE_SEM;
    fprintf(output_fd, "test_sync: Using defaults: iterations=%s, use_semaphore=%s\n",
           DEFAULT_TEST_SYNC_ITERS, DEFAULT_TEST_SYNC_USE_SEM);
    return (int)test_sync(2, default_args);
  }

  return (int)test_sync((uint64_t)(argc - 1), &argv[1]);
}

// Wrapper for test_synchro (with semaphore)
int test_synchro_wrapper(int argc, char **argv)
{
  extern uint64_t test_sync(uint64_t argc, char *argv[]);

  char *default_args[2];

  if (argc < 2)
  {
    int fds[2];
    getFD(fds);
    int output_fd = fds[1];
    // No iterations argument provided, use default
    default_args[0] = DEFAULT_TEST_SYNC_ITERS;
    default_args[1] = "1"; // Always use semaphore
    fprintf(output_fd, "test_synchro: Using defaults: iterations=%s, use_semaphore=1\n",
           DEFAULT_TEST_SYNC_ITERS);
    return (int)test_sync(2, default_args);
  }

  // User provided iterations, use it but force semaphore to 1
  char *sync_args[2];
  sync_args[0] = argv[1]; // User's iteration count
  sync_args[1] = "1";     // Force semaphore on
  return (int)test_sync(2, sync_args);
}

// Wrapper for test_no_synchro (without semaphore)
int test_no_synchro_wrapper(int argc, char **argv)
{
  extern uint64_t test_sync(uint64_t argc, char *argv[]);

  char *default_args[2];

  if (argc < 2)
  {
    int fds[2];
    getFD(fds);
    int output_fd = fds[1];
    // No iterations argument provided, use default
    default_args[0] = DEFAULT_TEST_SYNC_ITERS;
    default_args[1] = "0"; // Never use semaphore
    fprintf(output_fd, "test_no_synchro: Using defaults: iterations=%s, use_semaphore=0\n",
           DEFAULT_TEST_SYNC_ITERS);
    return (int)test_sync(2, default_args);
  }

  // User provided iterations, use it but force semaphore to 0
  char *sync_args[2];
  sync_args[0] = argv[1]; // User's iteration count
  sync_args[1] = "0";     // Force semaphore off
  return (int)test_sync(2, sync_args);
}

// Wrapper for test_processes
int test_processes_wrapper(int argc, char **argv)
{
  extern int64_t test_processes(uint64_t argc, char *argv[]);

  char *test_argv[1];

  if (argc < 2)
  {
    int fds[2];
    getFD(fds);
    int output_fd = fds[1];
    // No arguments, use default
    test_argv[0] = DEFAULT_TEST_PROCESSES_MAX;
    fprintf(output_fd, "test_processes: Using default max_processes = %s\n", DEFAULT_TEST_PROCESSES_MAX);
    return (int)test_processes(1, test_argv);
  }

  return (int)test_processes((uint64_t)(argc - 1), &argv[1]);
}