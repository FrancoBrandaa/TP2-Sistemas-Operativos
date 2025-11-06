#ifndef __QUEUEADT_H
#define __QUEUEADT_H

#include <stdint.h>
#include "../include/memoryManager.h"

typedef void *type; // long antes
typedef struct queueCDT *queueADT;

/*
 * Creates a new queue.
 * Returns a pointer to the newly created queue.
 */
queueADT newQueue();

/*
 * Adds an element to the queue.
 * Parameters:
 *   q - Pointer to the queue.
 *   element - The element to add to the queue.
 */
void queue(queueADT q, type element);

/*
 * Removes and retrieves the front element from the queue.
 * Parameters:
 *   q - Pointer to the queue.
 * Returns the front element of the queue, or a default value if the queue is empty.
 */
type dequeue(queueADT q);

/*
 * Checks if the queue is empty.
 * Parameters:
 *   q - Pointer to the queue.
 * Returns 1 if the queue is empty, or 0 otherwise.
 */
uint32_t isEmpty(queueADT q);

/*
 * Removes a specific element from the queue.
 * Parameters:
 *   q - Pointer to the queue.
 *   element - The element to remove.
 * Returns 1 if found and removed, 0 otherwise.
 */
int removeFromQueue(queueADT q, type element);

/*
 * Frees the memory allocated for the queue.
 * Parameters:
 *   q - Pointer to the queue to free.
 */
void freeQueue(queueADT q);

#endif