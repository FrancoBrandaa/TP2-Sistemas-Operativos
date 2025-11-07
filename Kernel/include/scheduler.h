#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#define YIELD_DONE 1
#define YIELD_NOT_DONE 0
#define DEFAULT_QUANTUM 3

// Aging configuration
#define AGING_THRESHOLD 15  // After being skipped this many times, boost priority
#define MAX_AGING_BOOST 2  // Maximum priority levels to boost

#include "./process.h"
#include <stdint.h>

/*
 * Initializes the scheduler for the initial process.
 */
void initInitScheduler();

/*
 * Initializes the scheduler system.
 */
void initScheduler();

/*
 * Adds a process to the scheduler queue.
 * Parameters:
 *   pcb - Pointer to the process control block of the process to schedule.
 */
void schedule(Process *pcb);

/*
 * Removes and retrieves the next process from the scheduler queue.
 * Returns a pointer to the process control block of the unscheduled process.
 */
Process *unschedule();

/*
 * Removes a specific process from all scheduler queues.
 * Parameters:
 *   pcb - Pointer to the process control block to remove.
 */
void unscheduleProcess(Process *pcb);

/*
 * Switches the CPU context to the next process.
 * Parameters:
 *   rsp - Pointer to the current stack pointer.
 * Returns the stack pointer of the next process.
 */
uint64_t *switchContext(uint64_t *rsp);

/*
 * Retrieves the process currently being executed.
 * Returns a pointer to the process control block of the current process.
 */
Process *getCurrentProcess();

/*
 * Blocks a process by its PID.
 * Parameters:
 *   pid - The PID of the process to block.
 * Returns 0 on success, or a negative value on failure.
 */
int blockProcess(PID pid);

/*
 * Unblocks a process by its PID.
 * Parameters:
 *   pid - The PID of the process to unblock.
 * Returns 0 on success, or a negative value on failure.
 */
int unblockProcess(PID pid);

/*
 * Sets the yield flag to indicate that the current process should yield.
 */
void setYield();

/*
 * Clears the yield flag.
 */
void clearYield();

/*
 * Retrieves the current state of the yield flag.
 * Returns 1 if the yield flag is set, or 0 otherwise.
 */
char getYield();

/*
 * Causes the current process to yield the CPU voluntarily.
 * Returns 1 on success.
 */
int yield();

#endif