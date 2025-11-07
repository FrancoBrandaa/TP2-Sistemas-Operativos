// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

/*
 * applyAging
 * ----------
 * Applies aging mechanism to prevent starvation.
 * Iterates through all priority queues, increments aging counter for waiting processes,
 * and boosts priority if a process has been waiting too long.
 */
void applyAging()
{
    // Iterate through all priority queues
    for (int queueIndex = 0; queueIndex < MAX_PRIORITY; queueIndex++)
    {
        if (isEmpty(priorityQueues[queueIndex]))
        {
            continue;
        }

        // Create temporary queue to store processed elements
        queueADT tempQueue = newQueue();
        if (tempQueue == NULL)
        {
            continue; // Skip this queue if can't allocate temp queue
        }

        // Process all elements in current queue
        while (!isEmpty(priorityQueues[queueIndex]))
        {
            Process *pcb = (Process *)dequeue(priorityQueues[queueIndex]);
            
            if (pcb == NULL)
            {
                break;
            }

            // Skip aging for init (PID 1) and shell (PID 2)
            if (pcb->pid == 1 || pcb->pid == 2)
            {
                queue(tempQueue, (type)pcb);
                continue;
            }

            // Increment aging counter (process is waiting)
            pcb->agingCounter++;

            // Check if process needs priority boost
            if (pcb->agingCounter >= AGING_THRESHOLD && pcb->priority < MAX_PRIORITY)
            {
                // Calculate boost level
                int boostLevel = pcb->agingCounter / AGING_THRESHOLD;
                if (boostLevel > MAX_AGING_BOOST)
                {
                    boostLevel = MAX_AGING_BOOST;
                }

                // Calculate new boosted priority
                Priority boostedPriority = pcb->originalPriority + boostLevel;
                
                // Cap at MAX_PRIORITY
                if (boostedPriority > MAX_PRIORITY)
                {
                    boostedPriority = MAX_PRIORITY;
                }

                // Update priority
                pcb->priority = boostedPriority;
            }

            // Store in temp queue
            queue(tempQueue, (type)pcb);
        }

        // Re-queue all processes to their appropriate priority queues
        while (!isEmpty(tempQueue))
        {
            Process *pcb = (Process *)dequeue(tempQueue);
            if (pcb != NULL)
            {
                int newQueueIndex = pcb->priority - 1;
                queue(priorityQueues[newQueueIndex], (type)pcb);
            }
        }

        // Free temporary queue
        freeQueue(tempQueue);
    }
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
        currentProcess->agingCounter = 0; // Reset aging when process starts running

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
        
        // Restore original priority before rescheduling
        if (currentProcess->priority != currentProcess->originalPriority)
        {
            currentProcess->priority = currentProcess->originalPriority;
        }
        
        schedule(currentProcess); // Put back in its priority queue
    }
    else if (currentProcess->state == BLOCKED || currentProcess->state == EXITED)
    {
        // Blocked or exited processes are NOT rescheduled
        quantumsLeft = 0;
        currentProcess->stackEnd = rsp;
    }

    // Apply aging to all processes in lower priority queues
    applyAging();

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
    currentProcess->agingCounter = 0; // Reset aging counter when process runs
    
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

// Remove a process from all priority queues
void unscheduleProcess(Process *pcb)
{
    if (pcb == NULL)
        return;

    // Search and remove from all priority queues
    for (int i = 0; i < MAX_PRIORITY; i++)
    {
        removeFromQueue(priorityQueues[i], (type)pcb);
    }
}

int yield()
{
    setYield();
    forceSwitchContext();
    return 1;
}