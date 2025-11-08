#include <stdint.h>
#include <libsys.h>

typedef int (*entryPoint)(int argc, char *argv[]);

uint32_t GetUint();
uint32_t GetUniform(uint32_t max);
uint8_t memcheck(void *start, uint8_t value, uint32_t size);
int64_t satoi(char *str);
void bussy_wait(uint64_t n);
void endless_loop();

extern uint64_t max_value;
void zero_to_max();
int zero_to_max_wrapper(int argc, char **argv);
int endless_loop_wrapper(int argc, char **argv);

// Test wrappers
int test_mm_wrapper(int argc, char **argv);
int test_prio_wrapper(int argc, char **argv);
int test_sync_wrapper(int argc, char **argv);
int test_synchro_wrapper(int argc, char **argv);
int test_no_synchro_wrapper(int argc, char **argv);
int test_processes_wrapper(int argc, char **argv);
// int process_inc_wrapper(int argc, char **argv);
