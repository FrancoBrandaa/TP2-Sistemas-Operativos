
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



void create_semaphore();

void wait(semaphoreADT sem, PID pid);


void post(semaphoreADT sem);
