#include "../include/memoryManager.h"
#include "../include/scheduler.h"
#include "../include/syscallDispatcher.h"
#include "../include/video.h"
#include "../include/queueADT.h"
#include <interrupts.h>

Quantum quantumsLeft = 0;
Process *currentProcess = NULL;
char isYield = YIELD_NOT_DONE;

// Priority-based scheduler: one queue per priority level
queueADT priorityQueues[MAX_PRIORITY]; // Index 0 = priority 1, Index 4 = priority 5

void initScheduler()
{
    // Initialize all priority queues
    for (int i = 0; i < MAX_PRIORITY; i++)
    {
        priorityQueues[i] = newQueue();
    }
    currentProcess = NULL;
}

void schedule(Process *pcb) // Add process to the appropriate priority queue
{
    if (pcb == NULL || pcb->priority < MIN_PRIORITY || pcb->priority > MAX_PRIORITY)
    {
        return; // Invalid process or priority
    }

    // Add process to the queue corresponding to its priority
    // priority 1 goes to index 0, priority 5 goes to index 4
    int queueIndex = pcb->priority - 1;
    queue(priorityQueues[queueIndex], (type)pcb);
}

Process *unschedule() // Get next process from highest priority queue
{
    // Search from highest priority (index MAX_PRIORITY-1) to lowest (index 0)
    for (int i = MAX_PRIORITY - 1; i >= 0; i--)
    {
        if (!isEmpty(priorityQueues[i]))
        {
            Process *pcb = (Process *)dequeue(priorityQueues[i]);
            return pcb;
        }
    }
    return NULL; // No ready processes
}

uint64_t *switchContext(uint64_t *rsp)
{
    if (currentProcess == NULL)
    {
        // First time: get initial process from queues
        currentProcess = unschedule();
        if (currentProcess == NULL)
        {
            return rsp; // No processes to run
        }
        quantumsLeft = currentProcess->priority - 1;
        currentProcess->state = RUNNING;

        return currentProcess->stackEnd;
    }

    if (currentProcess->state == RUNNING)
    {
        // Check if process still has quantum left and hasn't yielded
        if (quantumsLeft > 0 && getYield() != YIELD_DONE)
        {
            (quantumsLeft)--;
            return rsp; // Continue running current process
        }

        // Process exhausted quantum or yielded
        currentProcess->stackEnd = rsp;
        currentProcess->state = READY;
        schedule(currentProcess); // Put back in its priority queue
    }
    else if (currentProcess->state == BLOCKED || currentProcess->state == EXITED)
    {
        // Blocked or exited processes are NOT rescheduled
        quantumsLeft = 0;
        currentProcess->stackEnd = rsp;
    }

    // Get next process from highest priority queue
    do
    {
        currentProcess = unschedule();
        if (currentProcess == NULL)
        {
            return rsp; // No ready processes
        }

        // Skip blocked/exited processes (shouldn't be in queue, but safety check)
    } while (currentProcess->state == BLOCKED || currentProcess->state == EXITED);

    clearYield();
    quantumsLeft = currentProcess->priority - 1;
    currentProcess->state = RUNNING;
    return currentProcess->stackEnd;
}

Process *getCurrentProcess()
{
    return currentProcess;
}

int blockProcess(PID pid)
{
    Process *pcb;
    if ((pcb = getProcess(pid)) == NULL)
        return 1;

    pcb->state = BLOCKED;

    if (pcb->pid == currentProcess->pid)
    {
        return yield(); // Force context switch
    }
    return 0;
}

int unblockProcess(PID pid)
{
    Process *pcb = getProcess(pid);
    if (pcb == NULL || pcb->state != BLOCKED)
        return -1;

    pcb->state = READY;
    schedule(pcb); // Add back to appropriate priority queue
    return 0;
}

void setYield()
{
    isYield = YIELD_DONE;
}
void clearYield()
{
    isYield = YIELD_NOT_DONE;
}
char getYield()
{
    return isYield;
}

int yield()
{
    setYield();
    forceSwitchContext();
    return 1;
}