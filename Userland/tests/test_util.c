#include <stdint.h>
#include <stdio.h>
#include "libsys.h"
#include "test_util.h"

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

void endless_loop_print(uint64_t wait)
{
  int64_t pid = getpid();
  printf("endless_loop_print: %d\n", pid);
  while (1)
  {
    printf("%d ", pid);
    bussy_wait(wait);
  }
}

// Wrappers for test_prio compatibility
int64_t my_getpid()
{
  return getpid();
}

int64_t my_create_process(char *name, uint32_t priority, char **argv)
{
  extern int zero_to_max_wrapper(int argc, char **argv);
  return createProcess(name, zero_to_max_wrapper, 0, argv, priority, 1);
}

int32_t my_nice(int32_t pid, int32_t priority)
{
  return changePriority(pid, priority);
}

int32_t my_kill(int32_t pid)
{
  return kill(pid);
}

int32_t my_block(int32_t pid)
{
  return block(pid);
}

int32_t my_unblock(int32_t pid)
{
  return unblock(pid);
}

int32_t my_wait(int32_t pid)
{
  return wait(pid, NULL);
}

// Global variable for test_prio
uint64_t max_value = 0;

void zero_to_max()
{
  uint64_t value = 0;
  while (value++ != max_value)
    ;
  printf("PROCESS %d DONE!\n", getpid());
}

int zero_to_max_wrapper(int argc, char **argv)
{
  zero_to_max();
  return 0;
}
