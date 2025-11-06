#ifndef _USERLAND_PROCESS_H_
#define _USERLAND_PROCESS_H_

#include <stdint.h>

#define MAX_NAME_LENGTH 20
#define NONPID -1

typedef long PID;

typedef enum
{
    READY = 0,
    RUNNING,
    BLOCKED,
    EXITED,
} ProcessState;

typedef unsigned int Priority;

typedef int (*entryPoint)(int argc, char *argv[]);

typedef struct
{
    char name[MAX_NAME_LENGTH];
    PID pid;
    int argc;
    char **argv;
    Priority priority;
    Priority originalPriority; // For aging: original priority before any boost
    int foreground;
    entryPoint entryPoint;
    ProcessState state;
    uint64_t *stackBase, *stackEnd;
    int waitReturnValue;
    PID waitPid;
    int fds[2]; // uno de lectura otro de escritura
    uint64_t agingCounter; // Counts how many times this process was skipped
} Process;

typedef struct
{
    int argc;
    char **argv;
    char *name;
    Priority priority;
    entryPoint entryPoint;
    int foreground;
    int fds[2];
} creationParameters;

// Free the memory returned by ps()
void freeProcessInfo(Process *processesInfo);

#endif
