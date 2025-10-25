#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "shell.h"
#include "command.h"
#include <libsys.h>
#include <exceptions.h>
#include "../tests/test.h"

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
int exit_shell(int argc, char **argv);
int font(int argc, char **argv);
int help(int argc, char **argv);
int man(int argc, char **argv);
int regs(int argc, char **argv);

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

    {.name = "divzero",
     .description = "Generates a division by zero exception",
     .usage = "divzero",
     .type = CMD_BUILTIN,
     .handler.builtin = (int (*)(int, char **))_divzero},

    {.name = "echo",
     .description = "Prints the input string",
     .usage = "echo <text>",
     .type = CMD_BUILTIN,
     .handler.builtin = echo},

    {.name = "exit",
     .description = "Exits the shell with the provided exit code or 0",
     .usage = "exit [code]",
     .type = CMD_BUILTIN,
     .handler.builtin = exit_shell},

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

    {.name = "invop",
     .description = "Generates an invalid Opcode exception",
     .usage = "invop",
     .type = CMD_BUILTIN,
     .handler.builtin = (int (*)(int, char **))_invalidopcode},

    {.name = "man",
     .description = "Prints the description and usage of a command",
     .usage = "man <command>",
     .type = CMD_BUILTIN,
     .handler.builtin = man},

    {.name = "regs",
     .description = "Prints the register snapshot, if any",
     .usage = "regs",
     .type = CMD_BUILTIN,
     .handler.builtin = regs},
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

static int execute_command(Command *cmd, int argc, char **argv)
{
    if (cmd->type == CMD_BUILTIN)
    {
        // Execute built-in command directly
        return cmd->handler.builtin(argc, argv);
    }

    else if (cmd->type == CMD_PROCESS)
    {
        // Create a new process
        int pid = createProcess(
            cmd->name,
            cmd->handler.process.entrypoint,
            argc,
            argv,
            cmd->handler.process.priority,
            cmd->handler.process.is_background);

        if (pid < 0)
        {
            fprintf(FD_STDERR, "\e[0;31mError: Could not create process for '%s'\e[0m\n", cmd->name);
            return -1;
        }

        printf("Process '%s' created with PID: %d (%s)\n",
               cmd->name,
               pid,
               cmd->handler.process.is_background ? "background" : "foreground");

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
    
    // // MY_TEST_PROCESSES - Version mejorada con colores y delays
    // printf("\e[1;33m=== INICIANDO MY_TEST_PROCESSES ===\e[0m\n");
    // printf("Creando 3 procesos que imprimen su PID constantemente...\n");
    // printf("El test los crea, bloquea, desbloquea y mata aleatoriamente.\n\n");

    // char *test_argv[] = {"3"}; // Crear 3 procesos
    // my_test_processes(1, test_argv);
    // // ========================================================================

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

        if (parse_arguments(command_buffer, &argc, argv) != 0 || argc == 0)
        {
            buffer[0] = buffer_dim = 0;
            continue;
        }

        // Search and execute command
        Command *cmd = find_command(argv[0]);

        if (cmd != NULL)
        {
            // Execute command
            last_command_output = execute_command(cmd, argc, argv);

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

int exit_shell(int argc, char **argv)
{
    int exit_code = 0;

    if (argc > 1)
    {
        sscanf(argv[1], "%d", &exit_code);
    }

    return exit_code;
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
