#include <stdint.h>
#include <libsys.h>

typedef int (*entryPoint)(int argc, char *argv[]);

uint32_t GetUint();
uint32_t GetUniform(uint32_t max);
uint8_t memcheck(void *start, uint8_t value, uint32_t size);
int64_t satoi(char *str);
void bussy_wait(uint64_t n);
void endless_loop();
void endless_loop_print(uint64_t wait);

extern uint64_t max_value;
void zero_to_max();
int zero_to_max_wrapper(int argc, char **argv);

// Wrapper functions for tests
long my_getpid();
long my_create_process(char *name, uint32_t priority, char **argv);
int32_t my_nice(long pid, int32_t priority);
int32_t my_kill(int32_t pid);
int32_t my_block(long pid);
int32_t my_unblock(long pid);
int32_t my_wait(long pid);
