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
    int foreground;
    entryPoint entryPoint;
    ProcessState state;
    uint64_t *stackBase, *stackEnd;
    int waitReturnValue;
    PID waitPid;
} Process;

// Free the memory returned by ps()
void freeProcessInfo(Process *processesInfo);

#endif
