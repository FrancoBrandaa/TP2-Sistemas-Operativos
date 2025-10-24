#include "semaphoreADT.h"
#include "lib.h"
//modificar los nombres y cheqeuo de error en el get pid

semaphoreADT sem_init(int val){
    semaphoreADT sem = (semaphoreADT)allocMemory(sizeof(semaphoreCDT));
    sem->value = val;
    sem->lock = 0;
    sem->waitingQueue = newQueue();
    return sem;
}


void sem_wait(semaphoreADT sem , PID pid){
    Process * proc = getProcess(pid); 
    acquire_spinlock(&sem->lock);
    if(sem->value > 0){
        sem->value--; //yo debo protejer esto
    }else{
       queue(sem->waitingQueue, proc);
       blockProcess(proc);
    }
    release_spinlock(&sem->lock);
}


void sem_post(semaphoreADT sem ){
    acquire_spinlock(&sem->lock);
    if (!isEmpty(sem->waitingQueue)) {
        PID pid = dequeue(sem->waitingQueue);
        unblockProcess(pid);
    } else {
        sem->value++;
    }
    release_spinlock(&sem->lock);
}

void sem_destroy(semaphoreADT sem){
    freeQueue(sem->waitingQueue);
    freeMemory(sem);
}

