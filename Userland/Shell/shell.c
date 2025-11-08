// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#define INIT_PID 1

/* ============================================================================
 * INTERNAL STATE
 * ============================================================================ */

static char buffer[MAX_BUFFER_SIZE];
static int buffer_dim = 0;

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
int mem(int argc, char **argv);
int kill_cmd(int argc, char **argv);
int block_cmd(int argc, char **argv);
int nice_cmd(int argc, char **argv);
int test_cmd(int argc, char **argv);

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
     .description = "Reads from stdin and filters out vowels",
     .usage = "filter  (type anything and use Ctrl+D to end)",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = filter_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "cat",
     .description = "Reads from stdin and echoes it back",
     .usage = "cat (type anything and use Ctrl+D to end)",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = cat_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "wc",
     .description = "Reads from stdin and counts characters",
     .usage = "wc (type anything and use Ctrl+D to end)",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = wc_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "test_mm",
     .description = "Memory manager stress test",
     .usage = "test_mm <max_memory_bytes>",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = test_mm_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "test_processes",
     .description = "Process creation and management test",
     .usage = "test_processes <max_processes>",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = test_processes_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "test_prio",
     .description = "Process priority test",
     .usage = "test_prio <max_value>",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = test_prio_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "test_synchro",
     .description = "Synchronization test WITH semaphore",
     .usage = "test_synchro [iterations]",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = test_synchro_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "test_no_synchro",
     .description = "Synchronization test WITHOUT semaphore",
     .usage = "test_no_synchro [iterations]",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = test_no_synchro_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},

    {.name = "mvar",
     .description = "Multiple readers/writers synchronization problem",
     .usage = "mvar <writers> <readers>",
     .type = CMD_PROCESS,
     .handler.process = {.entrypoint = mvar_wrapper, .priority = DEFAULT_PRIORITY, .is_background = 0}},
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

// Function to parse pipe command and split into left and right parts
static int parse_pipe_command(char *command_buffer, char *left_cmd, char *right_cmd)
{
    // Find the pipe character
    char *pipe_pos = strchr(command_buffer, '|');

    if (pipe_pos == NULL)
    {
        return 0; // No pipe found
    }

    // Calculate lengths
    int left_len = pipe_pos - command_buffer;

    // Copy left command (before pipe)
    strncpy(left_cmd, command_buffer, left_len);
    left_cmd[left_len] = '\0';

    // Copy right command (after pipe)
    strcpy(right_cmd, pipe_pos + 1);

    // Trim whitespace from both commands
    // Trim left command (trailing spaces)
    while (left_len > 0 && (left_cmd[left_len - 1] == ' ' || left_cmd[left_len - 1] == '\t'))
    {
        left_cmd[--left_len] = '\0';
    }

    // Trim right command (leading whitespace)
    char *right_start = right_cmd;
    while (*right_start == ' ' || *right_start == '\t')
    {
        right_start++;
    }
    if (right_start != right_cmd)
    {
        memmove(right_cmd, right_start, strlen(right_start) + 1);
    }

    // Validate both commands are not empty
    if (strlen(left_cmd) == 0 || strlen(right_cmd) == 0)
    {
        return -1; // Invalid pipe syntax
    }

    return 1; // Pipe found and parsed successfully
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

        // Get current process FDs to inherit
        int fds[2] = {0, 1}; // Default STDIN, STDOUT
        getFD(fds);          // Try to get actual FDs, fallback to defaults if it fails (CHEQUIAR ESTO) puede cambiar segun diseño

        // Create a new process
        int pid = createProcess(
            cmd->name,
            cmd->handler.process.entrypoint,
            argc,
            argv,
            cmd->handler.process.priority,
            !is_background, // foreground parameter is inverted
            fds);

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

// Function to execute a piped command
static int execute_pipe(char *left_cmd, char *right_cmd)
{
    // Parse left command arguments
    char *left_argv[MAX_ARGS];
    int left_argc = 0;
    char left_buffer[MAX_BUFFER_SIZE];
    strncpy(left_buffer, left_cmd, MAX_BUFFER_SIZE);
    left_buffer[MAX_BUFFER_SIZE - 1] = '\0';

    if (parse_arguments(left_buffer, &left_argc, left_argv) != 0 || left_argc == 0)
    {
        fprintf(FD_STDERR, "\e[0;31mError: Invalid left command in pipe\e[0m\n");
        return -1;
    }

    // Parse right command arguments
    char *right_argv[MAX_ARGS];
    int right_argc = 0;
    char right_buffer[MAX_BUFFER_SIZE];
    strncpy(right_buffer, right_cmd, MAX_BUFFER_SIZE);
    right_buffer[MAX_BUFFER_SIZE - 1] = '\0';

    if (parse_arguments(right_buffer, &right_argc, right_argv) != 0 || right_argc == 0)
    {
        fprintf(FD_STDERR, "\e[0;31mError: Invalid right command in pipe\e[0m\n");
        return -1;
    }

    // Find both commands
    Command *left_command = find_command(left_argv[0]);
    Command *right_command = find_command(right_argv[0]);

    if (left_command == NULL)
    {
        fprintf(FD_STDERR, "\e[0;31mError: Left command '%s' not found\e[0m\n", left_argv[0]);
        return -1;
    }

    if (right_command == NULL)
    {
        fprintf(FD_STDERR, "\e[0;31mError: Right command '%s' not found\e[0m\n", right_argv[0]);
        return -1;
    }

    // For now, only support process-to-process pipes (not built-ins)
    if (left_command->type != CMD_PROCESS)
    {
        fprintf(FD_STDERR, "\e[0;31mError: Left command '%s' must be a process (built-ins not supported in pipes yet)\e[0m\n", left_argv[0]);
        return -1;
    }

    if (right_command->type != CMD_PROCESS)
    {
        fprintf(FD_STDERR, "\e[0;31mError: Right command '%s' must be a process (built-ins not supported in pipes yet)\e[0m\n", right_argv[0]);
        return -1;
    }

    // Create pipe (get pipe file descriptors)
    int pipe_fds[2];
    if (openPipe(pipe_fds) != 0)
    {
        fprintf(FD_STDERR, "\e[0;31mError: Could not create pipe\e[0m\n");
        return -1;
    }

    // Create left process (foreground, stdout = pipe write end)
    int left_fds[2] = {FD_STDIN, pipe_fds[1]}; // stdin=FD_STDIN, stdout=pipe_write
    int left_pid = createProcess(
        left_command->name,
        left_command->handler.process.entrypoint,
        left_argc,
        left_argv,
        left_command->handler.process.priority,
        1, // foreground (foreground=true) - FIXED!
        left_fds);

    if (left_pid < 0)
    {
        fprintf(FD_STDERR, "\e[0;31mError: Could not create left process '%s'\e[0m\n", left_command->name);
        closeFD(pipe_fds[0]);
        closeFD(pipe_fds[1]);
        return -1;
    }

    // Create right process (background, stdin = pipe read end)
    int right_fds[2] = {pipe_fds[0], FD_STDOUT}; // stdin=pipe_read, stdout=FD_STDOUT
    int right_pid = createProcess(
        right_command->name,
        right_command->handler.process.entrypoint,
        right_argc,
        right_argv,
        right_command->handler.process.priority,
        0, // background (foreground=false) - filter reads from pipe, not stdin
        right_fds);

    if (right_pid < 0)
    {
        fprintf(FD_STDERR, "\e[0;31mError: Could not create right process '%s'\e[0m\n", right_command->name);
        kill(left_pid); // Clean up left process
        closeFD(pipe_fds[0]);
        closeFD(pipe_fds[1]);
        return -1;
    }

    // DON'T close pipe FDs in shell yet - let processes start first!
    // The processes will close them when they terminate

    // Wait for both processes to complete
    int32_t left_status, right_status;

    // Wait for the foreground process (right) first
    wait(right_pid, &right_status);

    // Wait for the background process (left)
    wait(left_pid, &left_status);

    // Now it's safe to close pipe FDs in shell (if they weren't auto-closed)
    closeFD(pipe_fds[0]);
    closeFD(pipe_fds[1]);

    // Clear any input that was typed during process execution
    clearInputBuffer();

    return 0;
}

int main()
{
    clear(0, NULL);

    while (1)
    {
        printf("\e[0mshell \e[0;32m$\e[0m ");

        signed char c;

        while (buffer_dim < MAX_BUFFER_SIZE && (c = getchar()) != '\n')
        {
            buffer[buffer_dim++] = c;
        }

        buffer[buffer_dim] = 0;

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

        // FIRST: Check if it's a pipe command (BEFORE parse_arguments)
        // Use static buffers to avoid stack overflow
        static char left_cmd[MAX_BUFFER_SIZE];
        static char right_cmd[MAX_BUFFER_SIZE];
        static char pipe_buffer[MAX_BUFFER_SIZE];

        // Make a copy for pipe parsing (since parse_pipe_command modifies the string)
        strncpy(pipe_buffer, command_buffer, MAX_BUFFER_SIZE);
        pipe_buffer[MAX_BUFFER_SIZE - 1] = '\0'; // Ensure null termination

        int pipe_result = parse_pipe_command(pipe_buffer, left_cmd, right_cmd);

        if (pipe_result == 1)
        {
            // Valid pipe found, execute it
            last_command_output = execute_pipe(left_cmd, right_cmd);
        }
        else if (pipe_result == -1)
        {
            // Invalid pipe syntax
            fprintf(FD_STDERR, "\e[0;31mError: Invalid pipe syntax\e[0m\n");
        }
        else
        {
            // No pipe found, parse as normal command
            if (parse_arguments(command_buffer, &argc, argv) != 0 || argc == 0)
            {
                buffer[0] = buffer_dim = 0;
                continue;
            }

            // Search for individual command
            Command *cmd = find_command(argv[0]);

            if (cmd != NULL)
            {
                // Execute command (pass background flag)
                last_command_output = execute_command(cmd, argc, argv, run_in_background);
            }
            else
            {
                // Command not found
                fprintf(FD_STDERR, "\e[0;33mCommand not found:\e[0m %s\n", argv[0]);
            }
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
    if (pid == INIT_PID)
    {
        fprintf(FD_STDERR, "Error: Cannot kill init process (PID 1) - system critical\n");
        return 1;
    }

    // Protect shell itself
    if (pid == getpid())
    {
        fprintf(FD_STDERR, "Error: Cannot kill the shell itself\n");
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

    // Protect init process
    if (pid == INIT_PID)
    {
        fprintf(FD_STDERR, "Error: Cannot block init process (PID 1) - system critical\n");
        return 1;
    }

    // Protect shell from blocking itself
    if (pid == getpid())
    {
        fprintf(FD_STDERR, "Error: Cannot block the shell itself\n");
        return 1;
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
