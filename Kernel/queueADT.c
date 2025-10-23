#include "../include/queueADT.h"

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

void freeQueue(queueADT q)
{
    while (!isEmpty(q))
    {
        dequeue(q);
    }
    freeMemory(q);
}