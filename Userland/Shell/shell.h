#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include "commands.h"

#define MAX_BUFFER_SIZE 1024 /**< Maximum size for command input buffer */
#define MAX_ARGS 64          /**< Maximum number of arguments per command */

/**
 * @brief Calculates the number of commands in the commands array
 */
#define COMMAND_COUNT (sizeof(commands) / sizeof(Command))

/**
 * @brief Searches for a command by name in the commands array
 *
 * Performs a linear search through the global commands array to find
 * a command matching the given name (case-sensitive).
 *
 * @param name Name of the command to search for
 * @return Pointer to the Command structure if found, NULL otherwise
 */
Command *find_command(const char *name);

/**
 * @brief Global array of all available shell commands
 *
 * This array should be defined in shell.c and contains all registered
 * commands (both built-in and process commands).
 *
 * Commands should be sorted alphabetically by name for consistency.
 */
extern Command commands[];

#endif // SHELL_H
