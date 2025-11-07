// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../include/queueADT.h"
#include <stddef.h>

typedef struct node
{
    type data;
    struct node *tail;
} node;

typedef struct node *queueType;

typedef struct queueCDT
{
    queueType first;
    queueType last;
} queueCDT;

queueADT newQueue()
{
    queueADT q = (queueADT)allocMemory(sizeof(queueCDT));
    if (q == NULL)
        return NULL; // Memory allocation failed

    q->first = NULL;
    q->last = NULL;
    return q;
}

void queue(queueADT q, type element)
{
    if (q == NULL)
        return; // Queue not initialized

    queueType newNode = (queueType)allocMemory(sizeof(node));
    if (newNode == NULL)
        return; // Memory allocation failed

    newNode->data = element;
    newNode->tail = NULL;
    if (q->first == NULL)
    {
        q->first = newNode;
        q->last = newNode;
    }
    else
    {
        q->last->tail = newNode;
        q->last = newNode;
    }
}

type dequeue(queueADT q)
{
    if (q == NULL || q->first == NULL)
    {
        return NULL; // Return NULL for empty/invalid queue (type is now void*)
    }
    queueType temp = q->first;
    type data = temp->data;
    q->first = temp->tail;
    if (q->first == NULL)
    {
        q->last = NULL;
    }
    freeMemory(temp);
    return data;
}

uint32_t isEmpty(queueADT q)
{
    if (q == NULL)
        return 1; // NULL queue is considered empty
    return (q->first == NULL);
}

// Remove a specific element from the queue (search and remove)
int removeFromQueue(queueADT q, type element)
{
    if (q == NULL || q->first == NULL)
        return 0; // Queue is empty or invalid

    queueType current = q->first;
    queueType prev = NULL;

    // Search for the element
    while (current != NULL)
    {
        if (current->data == element)
        {
            // Found it - remove from list
            if (prev == NULL)
            {
                // Removing first element
                q->first = current->tail;
                if (q->first == NULL)
                    q->last = NULL; // Queue is now empty
            }
            else
            {
                // Removing middle or last element
                prev->tail = current->tail;
                if (current == q->last)
                    q->last = prev; // Was last element
            }
            freeMemory(current);
            return 1; // Successfully removed
        }
        prev = current;
        current = current->tail;
    }
    return 0; // Element not found
}

void freeQueue(queueADT q)
{
    if (q == NULL)
        return;

    while (!isEmpty(q))
    {
        dequeue(q);
    }
    freeMemory(q);
}