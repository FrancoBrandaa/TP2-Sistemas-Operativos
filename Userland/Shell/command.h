#ifndef COMMAND_H
#define COMMAND_H


typedef enum
{
    CMD_BUILTIN, /**< Command executes directly in shell context (synchronous) */
    CMD_PROCESS, /**< Command creates and runs as a new process */
} CommandType;


typedef struct
{
    char *name;        /**< Command name used for invocation */
    char *description; /**< Short description shown in help */
    char *usage;       /**< Usage syntax (e.g., "cmd [args]") */
    CommandType type;  /**< Execution type (BUILTIN or PROCESS) */
    union
    {
        /**
         * @brief Handler for built-in commands
         * @param argc Argument count
         * @param argv Argument vector (argv[0] is the command name)
         * @return Exit code (0 for success, non-zero for error)
         */
        int (*builtin)(int argc, char **argv);

        /**
         * @brief Configuration for process-spawning commands
         */
        struct
        {
            int (*entrypoint)(int argc, char **argv); /**< Process entry point */
            int priority;                             /**< Process priority (higher = more CPU time) */
            int is_background;                        /**< 1 = background, 0 = foreground */
        } process;
    } handler;
} Command;

#endif // COMMAND_H
