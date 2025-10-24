#include "semaphoreADT.h"
#include "lib.h"


semaphoreADT create_semaphore(int val){
    semaphoreADT sem = (semaphoreADT)allocMemory(sizeof(semaphoreCDT));
    sem->value = val;
    sem->lock = 0;
    sem->waitingQueue = newQueue();
    return sem;
}


void wait(semaphoreADT sem , PID pid){
    Process * proc = getProcess(pid); 
    acquire_spinlock(&sem->lock);
    if(sem->value > 0){
        sem->value--; //yo debo protejer esto
    }else{
       enqueue(sem->waitingQueue, proc);
       blockProcess(proc);
    }
    release_spinlock(&sem->lock);
}


void post(semaphoreADT sem ){
    acquire_spinlock(&sem->lock);
    if (!isEmpty(sem->waitingQueue)) {
        PID pid = dequeue(sem->waitingQueue);
        unblockProcess(pid);
    } else {
        sem->value++;
    }
    release_spinlock(&sem->lock);
}

