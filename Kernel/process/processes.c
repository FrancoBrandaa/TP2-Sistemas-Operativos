#include <process.h>
#include <memoryManager.h>
#include <defs.h>
#include <stdint.h>
#include <lib.h>
#include <interrupts.h>
#include <scheduler.h>
#include <videoDriver.h>
#include <fileDescriptors.h>

#define SHELLPID 2

Process processes[MAX_PROCESSES];
PID current;

void setup_process_stack(pcb_t *proc, void (*entry_point)())
{
    uint64_t *stack = (uint64_t *)(proc->stack_base + STACK_SIZE);

    // Falsifica el contexto como si hubiera sido interrumpido
    *(--stack) = 0; // r15
    *(--stack) = 0; // r14
    *(--stack) = 0; // r13
    *(--stack) = 0; // r12
    *(--stack) = 0; // r11
    *(--stack) = 0; // r10
    *(--stack) = 0; // r9
    *(--stack) = 0; // r8
    *(--stack) = 0; // rbp
    *(--stack) = 0; // rdi
    *(--stack) = 0; // rsi
    *(--stack) = 0; // rdx
    *(--stack) = 0; // rcx
    *(--stack) = 0; // rbx
    *(--stack) = 0; // rax

    // Contexto para iretq
    *(--stack) = (uint64_t)entry_point;                     // RIP
    *(--stack) = 0x8;                                       // CS
    *(--stack) = 0x202;                                     // RFLAGS (IF=1)
    *(--stack) = (uint64_t)(proc->stack_base + STACK_SIZE); // RSP
    *(--stack) = 0x10;                                      // SS

    proc->rsp = (uint64_t)stack;
}

/*
 * isValidPID
 * ----------
 * Qué hace:
 *  - Comprueba que un PID está dentro del rango permitido y que el proceso
 *    correspondiente no está en estado DEAD.
 * Uso:
 *  - Parámetro: `PID pid`.
 *  - Devuelve: 1 si el PID es válido y el proceso está activo, 0 en caso
 *    contrario.
 */
int isValidPID(PID pid)
{
    return pid > 0 && pid <= MAX_PID && processes[pid - 1].state != DEAD;
}

/*
 * unblockWaitingProcesses
 * -----------------------
 * Qué hace:
 *  - Recorre la tabla de procesos y desbloquea (llama a `unblockProcess`)
 *    aquellos procesos que estaban bloqueados esperando a que el proceso
 *    con PID == `pid` terminase. También escribe el valor de retorno del
 *    hijo en `childReturnValue`.
 * Uso:
 *  - Parámetros: `PID pid` (PID terminado), `int returnValue` (valor de
 *    retorno del proceso terminado).
 *  - No devuelve valor.
 * Efectos secundarios:
 *  - Modifica `waitingPID` y `childReturnValue` de procesos y cambia su
 *    estado mediante `unblockProcess`.
 */
void unblockWaitingProcesses(PID pid, int returnValue)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (processes[i].waitingPID == pid && processes[i].state == BLOCKED)
        {
            processes[i].waitingPID = NONPID;
            processes[i].childReturnValue = returnValue;
            unblockProcess(processes[i].pid);
        }
    }
}

/*
 * waitProcess
 * -----------
 * Qué hace:
 *  - Hace que el proceso actual espere a que el proceso con PID `pidToWait`
 *    termine. El proceso actual se marca como esperando y se bloquea.
 * Uso:
 *  - Parámetros: `PID pidToWait` (PID del hijo a esperar), `int *wstatus`
 *    (puntero opcional donde se almacenará el valor de retorno del hijo).
 *  - No devuelve valor.
 * Notas/validaciones:
 *  - Si `pidToWait` no es válido, o coincide con el PID del proceso actual,
 *    la función no hace nada.
 */
void waitProcess(PID pidToWait, int *wstatus)
{
    Process *currentProcess = getCurrentProcess();
    if (!isValidPID(pidToWait) || currentProcess->pid == pidToWait || processes[pidToWait - 1].state == DEAD)
        return;

    currentProcess->waitingPID = pidToWait;
    blockProcess(currentProcess->pid);

    if (wstatus != NULL)
        *wstatus = currentProcess->childReturnValue;
}

/*
 * processLoader
 * -------------
 * Qué hace:
 *  - Función wrapper que se instala como punto de inicio de un proceso.
 *    Llama al `entry` proporcionado pasando `argc` y `argv`, recoge su valor
 *    de retorno, desbloquea procesos que esperaban por este PID y finalmente
 *    termina (llamando a `kill`) al proceso actual.
 * Uso:
 *  - Parámetros: `int argc`, `char *argv[]`, `entryPoint entry`.
 *  - Devuelve el valor de retorno de `entry`.
 * Notas:
 *  - Se espera que `entry` sea la función principal del proceso (tipo
 *    `entryPoint`). Esta función no debe llamarse manualmente desde fuera
 *    del subsistema de procesos; se usa al crear y arrancar procesos.
 */
extern pcb_t *current_process;
int processLoader(int argc, char *argv[], entryPoint entry) void sys_exit()
{
    current_process->state = EXITED;
    asm volatile("int $0x20");
}

void sys_block()
{
    current_process->state = BLOCKED;
    asm volatile("int $0x20");
}

void sys_yield()
{
    asm volatile("int $0x20");
}
{
    int returnValue = entry(argc, argv);
    PID processPid = getpid();
    unblockWaitingProcesses(processPid, returnValue);
    kill(processPid);
    return returnValue;
}

/*
 * initProcesses
 * -------------
 * Qué hace:
 *  - Inicializa la tabla global `processes`, asignando PIDs secuenciales y
 *    marcando todos los procesos como `DEAD`. También inicializa la
 *    variable global `current`.
 * Uso:
 *  - Llamar durante la inicialización del kernel antes de crear procesos.
 *  - Devuelve 0 (actualmente) como código de éxito.
 */
PID initProcesses(void)
{
    current = 1;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        void idle_function()
        {
            while (1)
            {
                asm volatile("hlt");
            }
        }

        pcb_t idle_pcb;
        extern pcb_t *idle_process;

        void init_idle_process()
        {
            idle_pcb.stack_base = (uint8_t *)malloc(STACK_SIZE);
            idle_pcb.pid = 0;
            idle_pcb.state = READY;
            setup_process_stack(&idle_pcb, idle_function);
            idle_process = &idle_pcb;
        }
        processes[i].pid = i + 1;
        processes[i].state = DEAD;
        processes[i].argv = NULL;
        processes[i].argc = 0;
        processes[i].waitingPID = NONPID;
    }
    return 0;
}

int checkName(const char *name)
{
    return name != NULL && strlen(name) <= MAX_NAME_LENGTH;
}

/*
 * getFreeProcess
 * --------------
 * Qué hace:
 *  - Busca una entrada libre en la tabla `processes` (un proceso con estado
 *    DEAD) y devuelve su índice.
 * Uso:
 *  - Sin parámetros. Devuelve el índice del slot libre o -1 si no hay
 *    ninguno.
 */
int getFreeProcess()
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (processes[i].state == DEAD)
        {
            return i;
        }
    }
    return -1;
}

/*
 * createProcess
 * -------------
 * Qué hace:
 *  - Crea y prepara un nuevo proceso a partir de los `creationParameters`
 *    proporcionados: reserva pila, copia los argumentos, inicializa la PCB
 *    (`Process`) y lo pone en estado READY, además de agendarlo con
 *    `schedule`.
 * Uso:
 *  - Parámetro: `creationParameters *params` que debe contener nombre,
 *    prioridad, argc/argv, entryPoint, fds y flag `foreground`.
 *  - Devuelve: PID del proceso creado, o -1 en caso de error (parámetros
 *    inválidos o falta de memoria/slots).
 * Errores comunes:
 *  - `params == NULL`, prioridad fuera de rango, nombre demasiado largo,
 *    `entryPoint == NULL`, o agotamiento de slots/memoria -> devuelve -1.
 * Efectos secundarios:
 *  - Reserva memoria para la pila y para las copias de argv; en caso de
 *    error libera lo reservado.
 */
PID createProcess(creationParameters *params)
{
    if (params == NULL || !checkPriority(params->priority) || params->argc < 0 || params->entryPoint == NULL || !checkName(params->name) || current > MAX_PID)
    {
        return -1;
    }
    void *stackLimit = allocMemory(STACK_SIZE);
    char **args;
    if (stackLimit == NULL || (params->argc != 0 && (args = allocMemory(params->argc * sizeof(char *))) == NULL))
    {
        freeMemory(stackLimit);
        freeMemory(args);
        return -1;
    }

    for (int i = 0; i < params->argc; i++)
    {
        int len = strlen(params->argv[i]);
        if ((args[i] = allocMemory(len + 1)) == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                freeMemory(args[j]);
            }
            freeMemory(args);
            freeMemory(stackLimit);
            return -1;
        }

        memcpy(args[i], params->argv[i], len + 1);
    }

    Process *currentProcess;
    int allocatedProcess = getFreeProcess();
    if (allocatedProcess == -1)
    {
        freeMemory(stackLimit);
        for (int i = 0; i < params->argc; i++)
        {
            freeMemory(args[i]);
        }
        freeMemory(args);
        return -1;
    }

    memcpy(processes[allocatedProcess].name, params->name, strlen(params->name) + 1);
    processes[allocatedProcess].parentpid = (currentProcess = getCurrentProcess()) == NULL ? 0 : currentProcess->pid;
    processes[allocatedProcess].waitingPID = NONPID;
    processes[allocatedProcess].argc = params->argc;
    processes[allocatedProcess].argv = args;
    processes[allocatedProcess].priority = params->priority;
    processes[allocatedProcess].entryPoint = params->entryPoint;
    processes[allocatedProcess].foreground = params->foreground;
    processes[allocatedProcess].state = READY;
    processes[allocatedProcess].stackBase = stackLimit + STACK_SIZE;
    processes[allocatedProcess].stackEnd = setupStack(params->argc, args, params->entryPoint, processes[allocatedProcess].stackBase, (entryPoint)processLoader);
    memcpy(processes[allocatedProcess].fds, params->fds, sizeof(int) * 2);

    schedule(&(processes[allocatedProcess]));
    return processes[allocatedProcess].pid;
}

/*
 * getProcessesCount
 * -----------------
 * Qué hace:
 *  - Cuenta cuántos procesos hay activos (estado distinto de DEAD) en la
 *    tabla `processes`.
 * Uso:
 *  - Sin parámetros. Devuelve el número de procesos activos.
 */
int getProcessesCount()
{
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (processes[i].state != DEAD)
        {
            count++;
        }
    }
    return count;
}

/*
 * getpid
 * ------
 * Qué hace:
 *  - Devuelve el PID del proceso actual mediante `getCurrentProcess()`.
 * Uso:
 *  - Sin parámetros. Devuelve el PID actual.
 */
PID getpid(void)
{
    return getCurrentProcess()->pid;
}

/*
 * getppid
 * -------
 * Qué hace:
 *  - Devuelve el PID del padre (`parentpid`) del proceso actual.
 * Uso:
 *  - Sin parámetros. Devuelve el PID del proceso padre.
 */
PID getppid(void)
{
    return getCurrentProcess()->parentpid;
}

/*
 * getProcess
 * ----------
 * Qué hace:
 *  - Devuelve un puntero a la estructura `Process` correspondiente al PID
 *    proporcionado si está activa (no DEAD), o NULL en caso contrario.
 * Uso:
 *  - Parámetro: `PID pid`.
 *  - Devuelve: `Process*` o NULL.
 */
Process *getProcess(PID pid)
{
    if (processes[pid - 1].state != DEAD)
    {
        return &processes[pid - 1];
    }
    return NULL;
}

/*
 * getProcessesInformation
 * -----------------------
 * Qué hace:
 *  - Crea y devuelve una copia en memoria (allocMemory) de todas las
 *    estructuras `Process` activas. El array devuelto se termina poniendo
 *    `ans[count].pid = NONPID` como marcador de fin.
 * Uso:
 *  - Sin parámetros. Devuelve `Process*` a la copia. El llamador es
 *    responsable de liberar la memoria usando `freeProcessesInformation`.
 */
Process *getProcessesInformation()
{
    int count = getProcessesCount(), ansIndex = 0;
    Process *ans = allocMemory((count + 1) * sizeof(Process));
    ans[count].pid = NONPID;

    for (int i = 0; i < MAX_PROCESSES && ansIndex != count; i++)
    {
        if (processes[i].state != DEAD)
        {
            memcpy(&(ans[ansIndex++]), &(processes[i]), sizeof(Process));
        }
    }
    return ans;
}

/*
 * freeProcessesInformation
 * ------------------------
 * Qué hace:
 *  - Libera la memoria retornada por `getProcessesInformation`.
 * Uso:
 *  - Parámetro: `Process *processesInfo` devuelto por
 *    `getProcessesInformation`.
 */
void freeProcessesInformation(Process *processesInfo)
{
    freeMemory(processesInfo);
}

// int kill(PID pid)
// {

//     if (pid <= INITPID || pid > MAX_PID)
//         return -1;
//     Process *pcb = &processes[pid - 1];
//     if (pcb->state == DEAD)
//     {
//         return -1;
//     }

//     freeMemory(((void *)pcb->stackBase - STACK_SIZE));
//     if (pcb->argc > 0)
//     {
//         for (int i = 0; i < pcb->argc; i++)
//         {
//             freeMemory(pcb->argv[i]);
//         }
//         freeMemory(pcb->argv);
//     }
//     pcb->state = DEAD;
//     pcb->argv = NULL;
//     pcb->argc = 0;
//     closeFD(pcb->fds[0]);
//     closeFD(pcb->fds[1]);
//     garbageCollect();

//     unblockWaitingProcesses(pid, 0);

//     if (getCurrentProcess()->pid == pid)
//     {
//         forceSwitchContent();
//     }
//     return 0;
// }

// int killAllChildren(PID pid)
// {
//     if (pid <= INITPID || pid > MAX_PID)
//         return -1;

//     for (int i = 0; i < MAX_PROCESSES; i++)
//     {
//         if (processes[i].parentpid == pid && processes[i].state != DEAD)
//         {
//             kill(processes[i].pid);
//         }
//     }
//     return 0;
// }

// int changeProccessPriority(PID pid, Priority priority)
// {
//     if (!isValidPID(pid) || !checkPriority(priority))
//     {
//         return -1;
//     }
//     processes[pid - 1].priority = priority;
//     return 0;
// }

// int getFileDescriptors(int *fds){

//     Process *currentProcess = getCurrentProcess();
//     if (currentProcess == NULL)
//     {
//         return -1;
//     }
//     fds[0] = currentProcess->fds[0];
//     fds[1] = currentProcess->fds[1];
//     return 0;
// }

// /*
//  * getTerminalForegroundProcess
//  * ----------------------------
//  * Qué hace:
//  *  - Recorre la tabla de procesos y devuelve un puntero al proceso que
//  *    está marcado como "foreground" del terminal. Excluye procesos muertos y
//  *    los procesos de init y shell.
//  * Uso:
//  *  - Llamar sin parámetros. Devuelve `Process*` al proceso en foreground o
//  *    NULL si no hay ninguno.
//  * Notas:
//  *  - No cambia el estado de los procesos.
//  */
// Process*  getTerminalForegroundProcess(){
//     for(int i = 0;i < MAX_PROCESSES; i++)
//     {
//         if (processes[i].foreground && processes[i].state != DEAD && processes[i].pid != INITPID && processes[i].pid != SHELLPID)
//         {
//             return & processes[i];
//         }
//     }
//     return NULL;
// }

// /*
//  * checkPriority
//  * -------------
//  * Qué hace:
//  *  - Valida que una prioridad esté dentro del rango permitido.
//  * Uso:
//  *  - Parámetro: `Priority priority`.
//  *  - Devuelve 1 si la prioridad es válida, 0 si no lo es.
//  */
// int checkPriority(Priority priority)
// {
//     return priority >= MIN_PRIORITY && priority <= MAX_PRIORITY;
// }