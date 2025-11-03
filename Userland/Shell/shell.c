#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "shell.h"
#include "commands.h"
#include <libsys.h>
#include <process.h>
#include "../tests/test.h"

#define DEFAULT_PRIORITY 3
/* ============================================================================
 * INTERNAL STATE
 * ============================================================================ */

static char buffer[MAX_BUFFER_SIZE];
static int buffer_dim = 0;

/* Command history state */
char command_history[HISTORY_SIZE][MAX_BUFFER_SIZE] = {0};
char command_history_buffer[MAX_BUFFER_SIZE] = {0};
uint8_t command_history_last = 0;
static uint8_t last_command_arrowed = 0;

/* Last command exit code (for $? substitution) */
static uint64_t last_command_output = 0;

/* ============================================================================
 * COMMAND FUNCTION DECLARATIONS
 * ============================================================================ */

/* Built-in command implementations */
int clear(int argc, char **argv);
int echo(int argc, char **argv);
int font(int argc, char **argv);
int help(int argc, char **argv);
int man(int argc, char **argv);
int ps(int argc, char **argv);
int regs(int argc, char **argv);
int mem(int argc, char **argv);
int kill_cmd(int argc, char **argv);
int block_cmd(int argc, char **argv);
int nice_cmd(int argc, char **argv);
int test_cmd(int argc, char **argv);

/* ============================================================================
 * KEYBOARD CALLBACK DECLARATIONS
 * ============================================================================ */

static void printPreviousCommand(enum REGISTERABLE_KEYS scancode);
static void printNextCommand(enum REGISTERABLE_KEYS scancode);

// Lista de comandos disponibles
Command commands[] = {
    {.name = "clear",
     .description = "Clears the screen",
     .usage = "clear",
     .type = CMD_BUILTIN,
     .handler.builtin = clear},

    {.name = "echo",
     .description = "Prints the input string",
     .usage = "echo <text>",
     .type = CMD_BUILTIN,
     .handler.builtin = echo},

    {.name = "font",
     .description = "Increases or decreases the font size",
     .usage = "font [increase|decrease]",
     .type = CMD_BUILTIN,
     .handler.builtin = font},

    {.name = "help",
     .description = "Prints the available commands",
     .usage = "help",
     .type = CMD_BUILTIN,
     .handler.builtin = help},

    {.name = "man",
     .description = "Prints the description and usage of a command",
     .usage = "man <command>",
     .type = CMD_BUILTIN,
     .handler.builtin = man},

    {.name = "ps",
     .description = "Prints the list of all processes with their properties",
     .usage = "ps",
     .type = CMD_BUILTIN,
     .handler.builtin = ps},

    {.name = "regs",
     .description = "Prints the register snapshot, if any",
     .usage = "regs",
     .type = CMD_BUILTIN,
     .handler.builtin = regs},

    {.name = "mem",
     .description = "Prints memory status and usage",
     .usage = "mem",
     .type = CMD_BUILTIN,
     .handler.builtin = mem},

    {.name = "kill",
     .description = "Kills a process by PID",
     .usage = "kill <pid>",
     .type = CMD_BUILTIN,
     .handler.builtin = kill_cmd},

    {.name = "block",
     .description = "Toggles process state between blocked and ready",
     .usage = "block <pid>",
     .type = CMD_BUILTIN,
     .handler.builtin = block_cmd},

    {.name = "nice",
     .description = "Changes the priority of a process",
     .usage = "nice <pid> <priority>",
     .type = CMD_BUILTIN,
     .handler.builtin = nice_cmd},

    {.name = "test",
     .description = "Runs system tests",
     .usage = "test [test_name]",
     .type = CMD_BUILTIN,
     .handler.builtin = test_cmd},

    {.name = "counter",
     .description = "Counts from 1 to N and exits",
     .usage = "counter [max_count]",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = counter_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "loop",
     .description = "Prints PID with greeting every N seconds",
     .usage = "loop [seconds]",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = loop_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 1}},

    {.name = "loop_ps",
     .description = "Runs ps in a loop to monitor shell state",
     .usage = "loop_ps",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = loop_ps_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 1}},

    {.name = "filter",
     .description = "Reads from stdin and filters out vowels (use Ctrl+D to end)",
     .usage = "filter",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = filter_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "cat",
     .description = "Reads from stdin and echoes it back (use Ctrl+D to end)",
     .usage = "cat",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = cat_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "wc",
     .description = "Reads from stdin and counts characters (use Ctrl+D to end)",
     .usage = "wc",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = wc_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},
};

Command *find_command(const char *name)
{
    for (int i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcmp(commands[i].name, name) == 0)
        {
            return &commands[i];
        }
    }
    return NULL;
}

static int parse_arguments(char *buffer, int *argc, char **argv)
{
    *argc = 0;
    char *token = strtok(buffer, " ");

    while (token != NULL && *argc < MAX_ARGS)
    {
        argv[(*argc)++] = token;
        token = strtok(NULL, " ");
    }

    argv[*argc] = NULL;
    return 0;
}

static int execute_command(Command *cmd, int argc, char **argv, int run_in_background)
{
    if (cmd->type == CMD_BUILTIN)
    {
        // Execute built-in command directly
        return cmd->handler.builtin(argc, argv);
    }

    else if (cmd->type == CMD_PROCESS)
    {
        // Determine if process should run in background
        // User can override with & at the end of the command
        int is_background = run_in_background || cmd->handler.process.is_background;

        // Create a new process
        int pid = createProcess(
            cmd->name,
            cmd->handler.process.entrypoint,
            argc,
            argv,
            cmd->handler.process.priority,
            !is_background); // foreground parameter is inverted

        if (pid < 0)
        {
            fprintf(FD_STDERR, "\e[0;31mError: Could not create process for '%s'\e[0m\n", cmd->name);
            return -1;
        }

        printf("Process '%s' created with PID: %d (%s)\n",
               cmd->name,
               pid,
               is_background ? "background" : "foreground");

        // Wait for foreground processes to complete
        if (!is_background)
        {
            int32_t status;
            wait(pid, &status);

            // Clear any input that was typed during process execution
            clearInputBuffer();
            buffer_dim = 0;
            buffer[0] = '\0';

            printf("\e[0;32mProcess '%s' (PID %d) completed with status: %d\e[0m\n",
                   cmd->name, pid, status);
        }

        return 0;
    }

    return -1;
}

// KEYBOARD HISTORY
static void printPreviousCommand(enum REGISTERABLE_KEYS scancode)
{
    clearInputBuffer();
    last_command_arrowed = SUB_MOD(last_command_arrowed, 1, HISTORY_SIZE);

    if (command_history[last_command_arrowed][0] != 0)
        fprintf(FD_STDIN, command_history[last_command_arrowed]);
}

static void printNextCommand(enum REGISTERABLE_KEYS scancode)
{
    clearInputBuffer();
    last_command_arrowed = (last_command_arrowed + 1) % HISTORY_SIZE;

    if (command_history[last_command_arrowed][0] != 0)
        fprintf(FD_STDIN, command_history[last_command_arrowed]);
}

int main()
{
    clear(0, NULL);

    // // TEST_SYNC - Test de sincronización con semáforos
    // printf("\e[1;35m=== INICIANDO TEST_SYNC ===\e[0m\n");
    // printf("Este test verifica la sincronización con semáforos.\n");
    // printf("Sin semáforo: resultado impredecible debido a race conditions.\n");
    // printf("Con semáforo: resultado deterministico (debería ser 0).\n\n");

    // //Test sin semáforo
    // printf("\e[1;31m--- Test SIN semaforo ---\e[0m\n");
    // char *sync_argv_no_sem[] = {"100", "0"}; // 100 iteraciones, sin semáforo
    // test_sync(2, sync_argv_no_sem);
    // printf("\n");

    // // Test con semáforo
    // printf("\e[1;32m--- Test CON semaforo ---\e[0m\n");
    // char *sync_argv_with_sem[] = {"10", "1"}; // 100 iteraciones, con semáforo
    // test_sync(2, sync_argv_with_sem);
    // printf("\n\e[1;32m=== TEST_SYNC FINALIZADO ===\e[0m\n\n");

    // // TEST_PRIO - Test de prioridades
    // printf("\e[1;36m=== INICIANDO TEST_PRIO ===\e[0m\n");
    // printf("Este test crea 3 procesos con diferentes prioridades.\n");
    // printf("Deberías ver diferencias en el orden de finalización.\n\n");

    // char *prio_argv[] = {"100000000"}; // Max value para contar
    // test_prio(1, prio_argv);
    // printf("\n\e[1;32m=== TEST_PRIO FINALIZADO ===\e[0m\n\n");
    // // ========================================================================

    // // MY_TEST_PROCESSES - Version mejorada con colores y delays
    // printf("\e[1;33m=== INICIANDO MY_TEST_PROCESSES ===\e[0m\n");
    // printf("Creando 3 procesos que imprimen su PID constantemente...\n");
    // printf("El test los crea, bloquea, desbloquea y mata aleatoriamente.\n\n");

    // char *test_argv[] = {"3"}; // Crear 3 procesos
    // my_test_processes(1, test_argv);
    // // ========================================================================

    // ps(NULL, NULL);

    registerKey(KP_UP_KEY, printPreviousCommand);
    registerKey(KP_DOWN_KEY, printNextCommand);

    while (1)
    {
        printf("\e[0mshell \e[0;32m$\e[0m ");

        signed char c;

        while (buffer_dim < MAX_BUFFER_SIZE && (c = getchar()) != '\n')
        {
            command_history_buffer[buffer_dim] = c;
            buffer[buffer_dim++] = c;
        }

        buffer[buffer_dim] = 0;
        command_history_buffer[buffer_dim] = 0;

        if (buffer_dim == MAX_BUFFER_SIZE)
        {
            perror("\e[0;31mShell buffer overflow\e[0m\n");
            buffer[0] = buffer_dim = 0;
            while (c != '\n')
                c = getchar();
            continue;
        };

        buffer[buffer_dim] = 0;

        // Skip empty commands
        if (buffer_dim == 0 || buffer[0] == '\0')
        {
            buffer[0] = buffer_dim = 0;
            continue;
        }

        // Parse arguments
        char *argv[MAX_ARGS];
        int argc = 0;
        char command_buffer[MAX_BUFFER_SIZE];
        strncpy(command_buffer, buffer, MAX_BUFFER_SIZE);

        // Check for background execution (&)
        int run_in_background = 0;
        int len = strlen(command_buffer);
        if (len > 0 && command_buffer[len - 1] == '&')
        {
            run_in_background = 1;
            command_buffer[len - 1] = '\0'; // Remove the '&'
            // Trim trailing whitespace
            len--;
            while (len > 0 && (command_buffer[len - 1] == ' ' || command_buffer[len - 1] == '\t'))
            {
                command_buffer[len - 1] = '\0';
                len--;
            }
        }

        if (parse_arguments(command_buffer, &argc, argv) != 0 || argc == 0)
        {
            buffer[0] = buffer_dim = 0;
            continue;
        }

        // Search and execute command
        Command *cmd = find_command(argv[0]);

        if (cmd != NULL)
        {
            // Execute command (pass background flag)
            last_command_output = execute_command(cmd, argc, argv, run_in_background);

            // Save to history
            strncpy(command_history[command_history_last], command_history_buffer, MAX_BUFFER_SIZE - 1);
            command_history[command_history_last][buffer_dim] = '\0';
            INC_MOD(command_history_last, HISTORY_SIZE);
            last_command_arrowed = command_history_last;
        }
        else
        {
            // Command not found
            fprintf(FD_STDERR, "\e[0;33mCommand not found:\e[0m %s\n", argv[0]);
        }

        buffer[0] = buffer_dim = 0;
    }

    __builtin_unreachable();
    return 0;
}

// COMANDOS BUILT-IN

int echo(int argc, char **argv)
{
    // Print all arguments starting from argv[1]
    for (int i = 1; i < argc; i++)
    {
        printf("%s", argv[i]);
        if (i < argc - 1)
        {
            printf(" ");
        }
    }
    printf("\n");
    return 0;
}

int help(int argc, char **argv)
{
    (void)argc; // Unused
    (void)argv; // Unused

    printf("Available commands:\n");
    for (int i = 0; i < COMMAND_COUNT; i++)
    {
        printf("  %s\t- %s\n", commands[i].name, commands[i].description);
    }
    printf("\nUse 'man <command>' for more information.\n");
    return 0;
}

int clear(int argc, char **argv)
{
    (void)argc; // Unused
    (void)argv; // Unused

    clearScreen();
    return 0;
}

int font(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(FD_STDERR, "Usage: %s\n", commands[4].usage);
        return 1;
    }

    if (strcasecmp(argv[1], "increase") == 0)
    {
        return increaseFontSize();
    }
    else if (strcasecmp(argv[1], "decrease") == 0)
    {
        return decreaseFontSize();
    }

    fprintf(FD_STDERR, "Invalid argument: %s\n", argv[1]);
    fprintf(FD_STDERR, "Usage: %s\n", commands[4].usage);
    return 1;
}

int man(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(FD_STDERR, "Usage: man <command>\n");
        return 1;
    }

    for (int i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcasecmp(commands[i].name, argv[1]) == 0)
        {
            printf("Command: %s\n", commands[i].name);
            printf("Usage: %s\n", commands[i].usage);
            printf("Description: %s\n", commands[i].description);
            printf("Type: %s\n", commands[i].type == CMD_BUILTIN ? "Built-in" : "Process");
            return 0;
        }
    }

    fprintf(FD_STDERR, "Command not found: %s\n", argv[1]);
    return 1;
}

int ps(int argc, char **argv)
{
    (void)argc; // Unused
    (void)argv; // Unused

    Process *processes = getProcessList();

    if (processes == NULL)
    {
        fprintf(FD_STDERR, "Error: Could not retrieve process information\n");
        return 1;
    }

    const char *stateNames[] = {"READY", "RUNNING", "BLOCKED", "EXITED"};

    printf("\n\e[1;36m========== PROCESS LIST ==========\e[0m\n");

    for (int i = 0; processes[i].pid != NONPID; i++)
    {
        const char *stateName = (processes[i].state >= 0 && processes[i].state <= 3)
                                    ? stateNames[processes[i].state]
                                    : "UNKNOWN";

        // Color code based on state
        const char *stateColor;
        switch (processes[i].state)
        {
        case RUNNING:
            stateColor = "\e[1;32m"; // Bright green
            break;
        case READY:
            stateColor = "\e[0;32m"; // Green
            break;
        case BLOCKED:
            stateColor = "\e[0;33m"; // Yellow
            break;
        case EXITED:
            stateColor = "\e[0;31m"; // Red
            break;
        default:
            stateColor = "\e[0m";
            break;
        }

        // Print each process as a block
        printf("\n\e[1;34mPID:\e[0m          %d\n", processes[i].pid);
        printf("\e[1;34mName:\e[0m         %s\n", processes[i].name);
        printf("\e[1;34mPriority:\e[0m     %d\n", processes[i].priority);
        printf("\e[1;34mState:\e[0m        %s%s\e[0m\n", stateColor, stateName);
        printf("\e[1;34mStack Base:\e[0m   0x%016lx\n", (uint64_t)processes[i].stackBase);
        printf("\e[1;34mStack Ptr:\e[0m    0x%016lx\n", (uint64_t)processes[i].stackEnd);
        printf("\e[1;34mExecution:\e[0m    %s\n", processes[i].foreground ? "Foreground" : "Background");
        printf("----------------------------------\n");
    }

    printf("\n");
    free(processes);
    return 0;
}

int regs(int argc, char **argv)
{
    (void)argc; // Unused
    (void)argv; // Unused

    const static char *register_names[] = {
        "rax", "rbx", "rcx", "rdx", "rbp", "rdi", "rsi", "r8 ", "r9 ", "r10", "r11", "r12", "r13", "r14", "r15", "rsp", "rip", "rflags"};

    int64_t registers[18];

    uint8_t aux = getRegisterSnapshot(registers);

    if (aux == 0)
    {
        perror("No register snapshot available\n");
        return 1;
    }

    printf("Latest register snapshot:\n");

    for (int i = 0; i < 18; i++)
    {
        printf("\e[0;34m%s\e[0m: %x\n", register_names[i], registers[i]);
    }

    return 0;
}

int mem(int argc, char **argv)
{
    (void)argc; // Unused
    (void)argv; // Unused

    MemoryStatus memStatus;

    int32_t result = getMemoryStatus(&memStatus);

    if (result != 0)
    {
        fprintf(FD_STDERR, "Error: Could not retrieve memory status\n");
        return 1;
    }

    // Print memory status
    printf("\n\e[1;36m========== MEMORY STATUS ==========\e[0m\n\n");

    printf("\e[1;34mTotal Memory:\e[0m   %d bytes (%d KB)\n", memStatus.total, memStatus.total / 1024);

    printf("\e[1;34mUsed Memory:\e[0m    %d bytes (%d KB)\n", memStatus.used, memStatus.used / 1024);
    printf("\e[1;34mFree Memory:\e[0m    %d bytes (%d KB)\n", memStatus.free, memStatus.free / 1024);

    printf("\e[1;34mBase Address:\e[0m   %x\n", (uint64_t)memStatus.base);
    // printf("\e[1;34mEnd Address:\e[0m    %x\n", (uint64_t)memStatus.end);

    // Calculate usage percentage
    if (memStatus.total > 0)
    {
        uint32_t usagePercent = (memStatus.used * 100) / memStatus.total;
        // printf("\e[1;34mUsage:\e[0m          %d%%\n", usagePercent);

        // Visual bar
        // printf("\e[1;34mUsage Bar:\e[0m     [");
        printf("\e[1;34mUsage:\e[0m     [");
        int barWidth = 40;
        int filledWidth = (usagePercent * barWidth) / 100;

        for (int i = 0; i < barWidth; i++)
        {
            if (i < filledWidth)
                printf("\e[1;32m=\e[0m"); // Green for used
            else
                printf("\e[0;37m-\e[0m"); // Gray for free
        }
        printf("] %d%%\n", usagePercent);
    }

    printf("\n");
    return 0;
}

int kill_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(FD_STDERR, "Usage: kill <pid>\n");
        return 1;
    }

    long pid = satoi(argv[1]);

    if (pid <= 0)
    {
        fprintf(FD_STDERR, "Error: Invalid PID '%s'\n", argv[1]);
        return 1;
    }

    // Protect init process
    if (pid == 1)
    {
        fprintf(FD_STDERR, "Error: Cannot kill init process (PID 1) - system critical\n");
        return 1;
    }

    // Protect shell itself
    if (pid == getpid())
    {
        fprintf(FD_STDERR, "Error: Cannot kill the shell itself\n");
        // preguntar si deberia poder dejar al usuario hacer esto
        return 1;
    }

    int32_t result = kill(pid);

    if (result == 0)
    {
        printf("Process %d killed successfully\n", pid);
        return 0;
    }

    else
    {
        fprintf(FD_STDERR, "Error: Could not kill process %d (code: %d)\n", pid, result);
        return 1;
    }
}

int block_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: block <pid>\n");
        printf("Toggles process state between blocked and ready\n");
        return -1;
    }

    long pid = satoi(argv[1]);

    if (pid <= 0)
    {
        printf("Error: Invalid PID\n");
        return -1;
    }

    // Get current process list to check state
    Process *processes = getProcessList();
    if (processes == NULL)
    {
        printf("Error: Could not get process list\n");
        return -1;
    }

    // Find the process and check its state
    int found = 0;
    int is_blocked = 0;
    for (int i = 0; processes[i].pid != 0; i++)
    {
        if (processes[i].pid == pid)
        {
            found = 1;
            is_blocked = (processes[i].state == BLOCKED);
            break;
        }
    }

    if (!found)
    {
        printf("Error: Process with PID %d not found\n", pid);
        return -1;
    }

    int result;
    if (is_blocked)
    {
        // Unblock the process
        result = unblock(pid);
        if (result == 0)
        {
            printf("Process %d unblocked successfully\n", pid);
        }
        else
        {
            printf("Error: Could not unblock process %d\n", pid);
            return -1;
        }
    }
    else
    {
        // Block the process
        result = block(pid);
        if (result == 0)
        {
            printf("Process %d blocked successfully\n", pid);
        }
        else
        {
            printf("Error: Could not block process %d\n", pid);
            return -1;
        }
    }

    return 0;
}

int nice_cmd(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: nice <pid> <priority>\n");
        printf("Changes the priority of a process (1-5, where 5 is highest)\n");
        return -1;
    }

    long pid = satoi(argv[1]);
    int32_t priority = satoi(argv[2]);

    if (pid <= 0)
    {
        printf("Error: Invalid PID\n");
        return -1;
    }

    if (priority < 1 || priority > 5)
    {
        printf("Error: Priority must be between 1 and 5\n");
        return -1;
    }

    int result = nice(pid, priority);

    if (result == 0)
    {
        printf("Process %d priority changed to %d successfully\n", pid, priority);
    }
    else
    {
        printf("Error: Could not change priority of process %d\n", pid);
        return -1;
    }

    return 0;
}

int test_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Available tests:\n");
        printf("  mm         - Memory manager test\n");
        printf("  prio       - Process priority test\n");
        printf("  processes  - Process creation test (original)\n");
        printf("  myproc     - Process creation test (improved)\n");
        printf("  sync       - Synchronization test\n");
        printf("  memstatus  - Memory status test\n");
        printf("\nUsage: test <test_name>\n");
        return 0;
    }

    const char *test_name = argv[1];

    // TODO: Implement test execution
    printf("Test '%s' - Not implemented yet :(\nlo hacemos a lo ultimo esto\n", test_name);

    return 0;
}
