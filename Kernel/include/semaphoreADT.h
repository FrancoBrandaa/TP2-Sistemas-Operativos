
#include "queueADT.h"
#include "process.h"
#include "scheduler.h"

typedef struct semaphoreCDT*semaphoreADT;

typedef struct semaphoreCDT
{
    int32_t lock;
    int32_t value;
    queueADT waitingQueue;

} semaphoreCDT;



semaphoreADT sem_init(int value);

void sem_wait(semaphoreADT sem, PID pid);


void sem_post(semaphoreADT sem);
