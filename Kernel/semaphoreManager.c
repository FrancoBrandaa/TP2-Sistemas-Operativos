// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../include/semaphoreManager.h"

//TODO:
// El sistema registra qué procesos abrieron el semáforo,
// pero no restringe el uso de las operaciones de sincronización a esos procesos.

//Para mi, deberiamos cada vez que se hace un wait o aluna operacion con un semaforo en particular
// chequiar si el proceso lo abrio antes;

sem_t sems[MAX_SEMS];

void initSemManager(){

    for (int i = 0; i < MAX_SEMS; i++) {
        sems[i].used = 0;
        sems[i].locked = 0;
        sems[i].waiting = NULL;
    }
}

static int findFreeSem() {
    for (int i = 0; i < MAX_SEMS; i++) {
        if (sems[i].used == 0) {
            return i;
        }
    }
    return -1;
}

int findSemByName(const char * name){
    for (int i = 0; i < MAX_SEMS; i++) {
        if ((sems[i].used == 1) && strcmp(sems[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int semExists(int semId) {
    if (semId < 0 || semId >= MAX_SEMS || sems[semId].used == 0) {
        return 0;
    }
    return 1;
}

int semCreate(const char * name, int value){
    int nameLen = strlen(name);
    if (nameLen > S_MAX_NAME_LENGTH || findSemByName(name) != -1){
        return -1;
    }

    int semId;
    semId = findFreeSem();
    if (semId < 0) {
        return -1;
    }
    
    sems[semId].value = value;
    sems[semId].used = 1;
    sems[semId].waiting = newQueue();
    memcpy(sems[semId].name, name, nameLen);

    for (int i = 0; i < MAX_PROCESSES; i++){
        sems[semId].openedBy[i] = 0;
    } //ningun proceso abrio todavia el semaforo
    sems[semId].openedBy[getpid()] = 1;

    return semId;
}

int semOpen(const char * name){
    
    int semId = findSemByName(name);

    if (semId < 0 || sems[semId].used == 0) {
        return -1;
    }

    sems[semId].openedBy[getpid()] = 1;
    
    return semId;
}

int semClose(int semId){

    if (semId < 0 || sems[semId].used == 0 || sems[semId].openedBy[getpid()] == 0) {
        return -1;
    }

    sems[semId].openedBy[getpid()] = 0;

    return 0;
}
//
int semWait(int semId){
    
    if (semId < 0) {
        return -1;
    }

    acquire_spinlock(&sems[semId].locked);

    if (sems[semId].used == 0) {
        release_spinlock(&sems[semId].locked);
        return -1;
    }

    while (sems[semId].value == 0) {
        PID currentProcess = getpid();
        queue(sems[semId].waiting, (void*)currentProcess);

        release_spinlock(&sems[semId].locked);

        blockProcess(currentProcess);
    
        acquire_spinlock(&sems[semId].locked);
    }

    sems[semId].value--;
    
    release_spinlock(&sems[semId].locked);

    return 0;
}
//
int semPost(int semId){

    
    if (semId < 0 || semId >= MAX_SEMS) {
        return -1;
    }
    acquire_spinlock(&sems[semId].locked);
    
    if (sems[semId].used == 0) {
        release_spinlock(&sems[semId].locked);
        return -1;
    }

    sems[semId].value++;
    
    while (!isEmpty(sems[semId].waiting)) {
        PID pid = (PID)dequeue(sems[semId].waiting);
        unblockProcess(pid);
    }

    release_spinlock(&sems[semId].locked);

    return 0;
}

int semValue(int semId){
    if (!semExists(semId)){
        return -1;
    }
    return sems[semId].value;
}

int semDestroy(int semId){

    if (semId < 0) {
        release_spinlock(&sems[semId].locked);
        return -1;
    }

    acquire_spinlock(&sems[semId].locked);

    sems[semId].used = 0; //permite reutilizar el semaforo

    while (!isEmpty(sems[semId].waiting)) {
        PID pid = (PID)dequeue(sems[semId].waiting);
        unblockProcess(pid);
    }

    freeQueue(sems[semId].waiting);

    release_spinlock(&sems[semId].locked);

    return 0;
}

// Set a semaphore to an absolute value. If increasing, wake up waiting processes.
// If decreasing below current value, we just reduce the counter (never negative). If newValue < 0 returns -1.
// Waking strategy: when raising value, we unblock as many waiting processes as allowed by the new capacity.
int semSetValue(int semId, int newValue){
    if (semId < 0 || semId >= MAX_SEMS || newValue < 0) {
        return -1;
    }

    acquire_spinlock(&sems[semId].locked);

    if (sems[semId].used == 0) {
        release_spinlock(&sems[semId].locked);
        return -1;
    }

    int oldValue = sems[semId].value;
    sems[semId].value = newValue;

    // If we increased the value, unblock up to (newValue - oldValue) processes or until queue empties
    if (newValue > oldValue) {
        int available = newValue - oldValue;
        while (available > 0 && !isEmpty(sems[semId].waiting)) {
            PID pid = (PID)dequeue(sems[semId].waiting);
            unblockProcess(pid);
            available--;
        }
    }

    release_spinlock(&sems[semId].locked);
    return 0;
}

