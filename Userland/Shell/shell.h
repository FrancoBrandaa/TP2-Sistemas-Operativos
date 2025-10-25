#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include "command.h"

#define MAX_BUFFER_SIZE 1024 /**< Maximum size for command input buffer */
#define HISTORY_SIZE 10      /**< Number of commands to keep in history */
#define MAX_ARGS 64          /**< Maximum number of arguments per command */

/**
 * @brief Increments a value modulo m (circular increment)
 * @param x Value to increment
 * @param m Modulo value
 */
#define INC_MOD(x, m) x = (((x) + 1) % (m))

/**
 * @brief Subtracts two values modulo m (circular subtraction)
 * @param a First value
 * @param b Second value (to subtract)
 * @param m Modulo value
 * @return (a - b) mod m
 */
#define SUB_MOD(a, b, m) ((a) - (b) < 0 ? (m) - (b) + (a) : (a) - (b))

/**
 * @brief Decrements a value modulo m (circular decrement)
 * @param x Value to decrement
 * @param m Modulo value
 */
#define DEC_MOD(x, m) ((x) = SUB_MOD(x, 1, m))

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
