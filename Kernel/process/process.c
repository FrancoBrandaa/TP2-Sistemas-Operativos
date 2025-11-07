// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <process.h>
#include <memoryManager.h>
#include <defs.h>
#include <stdint.h>
#include <lib.h>
#include <interrupts.h>
#include <scheduler.h>
#include <video.h>
#include <lib.h>
#include <fds.h>

#define SHELLPID 2

Process processes[MAX_PROCESSES];
PID current;

/*
 * isValidPID
 * ----------
 * Qué hace:
 *  - Comprueba que un PID está dentro del rango permitido y que el proceso
 *    correspondiente no está en estado EXITED.
 * Uso:
 *  - Parámetro: `PID pid`.
 *  - Devuelve: 1 si el PID es válido y el proceso está activo, 0 en caso
 *    contrario.
 */
int isValidPID(PID pid)
{
    return pid > 0 && pid <= MAX_PID && processes[pid - 1].state != EXITED;
}

/*
 * initProcesses
 * -------------
 * Qué hace:
 *  - Inicializa la tabla global `processes`, asignando PIDs secuenciales y
 *    marcando todos los procesos como `EXITED`. También inicializa la
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
        processes[i].pid = i + 1;
        processes[i].state = EXITED;
        processes[i].argv = NULL;
        processes[i].argc = 0;
        processes[i].waitPid = NONPID;
        processes[i].fds[0] = 0; // Initialize stdin
        processes[i].fds[1] = 1; // Initialize stdout
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
 *    EXITED) y devuelve su índice.
 * Uso:
 *  - Sin parámetros. Devuelve el índice del slot libre o -1 si no hay
 *    ninguno.
 */
int getFreeProcess()
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (processes[i].state == EXITED)
        {
            return i;
        }
    }
    return -1;
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
 *
 * VER DE MEJORAR EL ESTILO REMOVIENDO ESTA FUNCION
 */
int processLoader(int argc, char *argv[], entryPoint entry)
{
    int returnValue = entry(argc, argv);
    PID processPid = getpid();
    Process *proc = getProcess(processPid);
    proc->waitReturnValue = returnValue;
    // unblockWaitingProcesses(processPid, returnValue);
    kill(processPid);
    return returnValue;
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
        return -1;

    void *stackLimit = allocMemory(STACK_SIZE);
    char **args = NULL; // ensure NULL when argc == 0
    if (stackLimit == NULL || (params->argc != 0 && (args = allocMemory(params->argc * sizeof(char *))) == NULL))
    {
        freeMemory(stackLimit);
        freeMemory(args);
        return -1;
    }

    // Para cada argumento, reserva memoria y copia el contenido.
    // Si alguna reserva falla, libera todo lo reservado y retorna -1.
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

    int allocatedProcess = getFreeProcess();
    if (allocatedProcess == -1)
    {
        freeMemory(stackLimit);
        for (int i = 0; i < params->argc; i++)
            freeMemory(args[i]);

        freeMemory(args);
        return -1;
    }

    memcpy(processes[allocatedProcess].name, params->name, strlen(params->name) + 1);

    processes[allocatedProcess].argc = params->argc;
    processes[allocatedProcess].argv = args;
    processes[allocatedProcess].priority = params->priority;
    processes[allocatedProcess].originalPriority = params->priority; // For aging
    processes[allocatedProcess].agingCounter = 0; // Initialize aging counter
    processes[allocatedProcess].entryPoint = params->entryPoint;
    processes[allocatedProcess].foreground = params->foreground;
    processes[allocatedProcess].waitPid = NONPID;
    processes[allocatedProcess].state = READY;
    processes[allocatedProcess].stackBase = stackLimit + STACK_SIZE; // donde aputa rsp

    // lo mando a asm
    processes[allocatedProcess].stackEnd = setupStack(params->argc, args, params->entryPoint, processes[allocatedProcess].stackBase, (entryPoint)processLoader);
    // rdi = argc , rsi = argv , rdx = entryPoint,

    memcpy(processes[allocatedProcess].fds, params->fds, sizeof(int) * 2);
    schedule(&(processes[allocatedProcess])); // agrego a la cola el proceso creado
    return processes[allocatedProcess].pid;
}

/*
 * getProcessesCount
 * -----------------
 * Qué hace:
 *  - Cuenta cuántos procesos hay activos (estado distinto de EXITED) en la
 *    tabla `processes`.
 * Uso:
 *  - Sin parámetros. Devuelve el número de procesos activos.
 */
int getProcessesCount()
{
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (processes[i].state != EXITED)
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
 * getProcess
 * ----------
 * Qué hace:
 *  - Devuelve un puntero a la estructura `Process` correspondiente al PID
 *    proporcionado si está activa (no EXITED), o NULL en caso contrario.
 * Uso:
 *  - Parámetro: `PID pid`.
 *  - Devuelve: `Process*` o NULL.
 */
Process *getProcess(PID pid)
{
    if (pid <= 0 || pid > MAX_PID)
    {
        return NULL;
    }
    if (processes[pid - 1].state != EXITED)
    {
        return &processes[pid - 1];
    }
    return NULL;
}

/*
 * checkPriority
 * -------------
 * Qué hace:
 *  - Valida que una prioridad esté dentro del rango permitido.
 * Uso:
 *  - Parámetro: `Priority priority`.
 *  - Devuelve 1 si la prioridad es válida, 0 si no lo es.
 */
int checkPriority(Priority priority)
{
    return priority >= MIN_PRIORITY && priority <= MAX_PRIORITY;
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
    if (ans == NULL)
    {
        return NULL;
    }

    // Initialize all memory to zero to avoid garbage data
    for (int i = 0; i <= count; i++)
    {
        ans[i].pid = NONPID;
        ans[i].state = EXITED;
        ans[i].name[0] = '\0';
        ans[i].argc = 0;
        ans[i].argv = NULL;
        ans[i].priority = 0;
        ans[i].foreground = 0;
        ans[i].entryPoint = NULL;
        ans[i].stackBase = NULL;
        ans[i].stackEnd = NULL;
        ans[i].waitReturnValue = 0;
        ans[i].waitPid = NONPID;
        ans[i].fds[0] = 0;
        ans[i].fds[1] = 1;
    }

    for (int i = 0; i < MAX_PROCESSES && ansIndex < count; i++)
    {
        if (processes[i].state != EXITED)
        {
            memcpy(&(ans[ansIndex++]), &(processes[i]), sizeof(Process));
        }
    }
    return ans;
}

/*
 * waitProcess
 * -----------
 * Bloquea el proceso actual hasta que el proceso con PID `pidToWait`
 * termine. Si `wstatus` no es NULL, almacena el valor de retorno del
 * proceso esperado.
 */
void waitProcess(PID pidToWait, int *wstatus)
{
    if (pidToWait <= 0 || pidToWait > MAX_PID)
        return;

    PID mypid = getpid();
    Process *processToWait = &processes[pidToWait - 1];

    // Si el proceso ya hizo exit, solo retorno
    if (processToWait->state == EXITED)
    {
        if (wstatus != NULL)
        {
            *wstatus = 0; // Proceso ya hizo exit
        }
        return;
    }

    processToWait->waitPid = mypid;

    // Bloquea el proceso y fuerza el cambio de contexto
    blockProcess(mypid);
    forceSwitchContext();

    // Cuando se despierta leo el valor de retorno
    if (wstatus != NULL)
    {
        *wstatus = processToWait->waitReturnValue;
    }
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

int kill(PID pid)
{
    if (pid <= INITPID || pid > MAX_PID)
        return -1;
    Process *pcb = &processes[pid - 1];
    if (pcb->state == EXITED)
    {
        return -1;
    }

    // Remove from scheduler queues BEFORE freeing memory
    extern void unscheduleProcess(Process * pcb);
    unscheduleProcess(pcb);

    freeMemory(((void *)pcb->stackBase - STACK_SIZE));
    if (pcb->argc > 0)
    {
        for (int i = 0; i < pcb->argc; i++)
        {
            freeMemory(pcb->argv[i]);
        }
        freeMemory(pcb->argv);
    }
    pcb->state = EXITED;
    pcb->argv = NULL;
    pcb->argc = 0;
    closeFD(pcb->fds[0]);
    closeFD(pcb->fds[1]);

    // unblockWaitingProcesses(pid, 0);
    if (pcb->waitPid != NONPID)
    {
        unblockProcess(pcb->waitPid);
        pcb->waitPid = NONPID;
    }

    if (getCurrentProcess()->pid == pid)
    {
        forceSwitchContext();
    }
    return 0;
}

int changeProccessPriority(PID pid, Priority priority)
{
    if (!isValidPID(pid) || !checkPriority(priority))
    {
        return -1;
    }

    Process *pcb = &processes[pid - 1];

    // If priority hasn't changed, nothing to do
    if (pcb->originalPriority == priority)
    {
        return 0;
    }

    pcb->priority = priority;
    pcb->originalPriority = priority; // Update original priority
    pcb->agingCounter = 0; // Reset aging counter when priority is manually changed

    // If the process is currently running, yield to reschedule it with new priority
    Process *currentProc = getCurrentProcess();
    if (currentProc != NULL && pcb->pid == currentProc->pid)
    {
        yield(); // Force reschedule with new priority
    }

    // Si el proceso esta READY, el cambio de prioridad va a tener efecto la proxima vez
    // que se lo agende en la cola. No podemos moverlo entre colas sin hacer
    //  una operacion de remover de la cola actual.

    // Si el proceso esta BLOCKED, la nueva prioridad va a ser usada cuando este sea desbloqueado

    return 0;
}

Process *getTerminalForegroundProcess()
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (processes[i].foreground && processes[i].state != EXITED &&
            processes[i].pid != INITPID && processes[i].pid != SHELLPID)
        {
            return &processes[i];
        }
    }
    return NULL;
}

int getFileDescriptors(int *fds)
{
    Process *currentProcess = getCurrentProcess();
    if (currentProcess == NULL)
    {
        return -1;
    }
    fds[0] = currentProcess->fds[0];
    fds[1] = currentProcess->fds[1];
    return 0;
}

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
//         if (processes[i].foreground && processes[i].state != EXITED && processes[i].pid != INITPID && processes[i].pid != SHELLPID)
//         {
//             return & processes[i];
//         }
//     }
//     return NULL;
// }