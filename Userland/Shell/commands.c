/**
 * @file commands.c
 * @brief Implementation of shell command functions
 *
 * This file contains process wrappers and test utilities that can be
 * executed as shell commands. These are separated from shell.c to keep
 * the codebase organized and maintainable.
 */

#include <stdio.h>
#include <stdint.h>
#include <libsys.h>
#include <process.h>
#include <syscalls.h>
#include "commands.h"
#include "../tests/test_util.h"

/**
 * counter_wrapper - Counts from 1 to N and exits
 *
 * Usage: counter [max]
 *   - max: Number to count up to (default: 10)
 *
 * This is useful for testing:
 *   - Foreground wait: Shell should wait until counting is done
 *   - Background execution: Can count while you use the shell
 *   - Process completion: Should see "Process completed" message
 */
int counter_wrapper(int argc, char **argv)
{
    int max = 10;
    int64_t pid = getpid();

    if (argc > 1)
    {
        max = satoi(argv[1]);
    }

    printf("\e[0;36m[Process %d] Counting to %d...\e[0m\n", pid, max);

    for (int i = 1; i <= max; i++)
    {
        printf("\e[0;36m[%d]\e[0m %d ", pid, i);
        if (i % 10 == 0)
            printf("\n");
        bussy_wait(200000000); // 200ms delay
    }

    printf("\n\e[0;32m[Process %d] Done! Counted to %d\e[0m\n", pid, max);
    return max; // Return the count as exit status
}

/**
 * loop_wrapper - Prints PID with greeting every N seconds
 *
 * Usage: loop [seconds]
 *   - seconds: Time to wait between prints (default: 2)
 *
 * This function demonstrates:
 *   - Using sleep() instead of busy waiting
 *   - Long-running background processes
 *   - Proper process identification
 */
int loop_wrapper(int argc, char **argv)
{
    int seconds = 2; // Default: 2 seconds
    int64_t pid = getpid();

    if (argc > 1)
    {
        seconds = satoi(argv[1]);
        if (seconds <= 0)
            seconds = 2; // Fallback to default if invalid
    }

    printf("\e[0;35m[Process %d] Starting loop with %d second interval...\e[0m\n", pid, seconds);

    while (1)
    {
        printf("\e[0;35mSoy el proceso %d xd\e[0m\n", pid);
        sleep(seconds * 1000); // Convert seconds to milliseconds
    }

    // no deberia llegar aca creo
    return 0;
}

/**
 * loop_ps_wrapper - Ejecuta ps cada segundo para monitorear el estado de la shell
 *
 * Usage: loop_ps
 *
 * Este comando es útil para verificar que la shell se bloquea correctamente
 * cuando NO estás escribiendo. Ejecuta en background y verás el estado
 * de todos los procesos cada segundo.
 */
int loop_ps_wrapper(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int64_t my_pid = getpid();
    const char *stateNames[] = {"READY", "RUNNING", "BLOCKED", "EXITED"};

    printf("\e[1;33m[Process %d] Monitoring shell state.\e[0m\n\n", my_pid);

    for (int iteration = 0; iteration < 20; iteration++) // 20 segundos de monitoreo
    {
        Process *processes = getProcessList();

        if (processes != NULL)
        {
            // Buscar y mostrar solo el proceso de la shell (PID 2)
            for (int i = 0; processes[i].pid != NONPID; i++)
            {
                if (processes[i].pid == 2) // Shell PID
                {
                    const char *stateName = (processes[i].state >= 0 && processes[i].state <= 3)
                                                ? stateNames[processes[i].state]
                                                : "UNKNOWN";

                    const char *stateColor;
                    switch (processes[i].state)
                    {
                    case RUNNING:
                        stateColor = "\e[1;32m";
                        break;
                    case READY:
                        stateColor = "\e[0;32m";
                        break;
                    case BLOCKED:
                        stateColor = "\e[0;33m";
                        break;
                    default:
                        stateColor = "\e[0m";
                        break;
                    }

                    printf("Shell (PID %d): %s%s\e[0m\n",
                           processes[i].pid, stateColor, stateName);
                    break;
                }
            }

            free(processes);
        }

        bussy_wait(1000000000); // Wait 1 second
    }

    printf("\e[1;32m[Process %d] Monitoring complete.\e[0m\n", my_pid);
    return 0;
}

/**
 * filter_wrapper - Lee una línea de stdin y filtra vocales
 *
 * Usage: filter
 *
 * Lee caracteres desde stdin hasta encontrar un newline o EOF.
 * Filtra todas las vocales (mayúsculas y minúsculas).
 * Imprime solo las consonantes, números y símbolos.
 */
int filter_wrapper(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int c; // Debe ser int, no char, para poder comparar con EOF (-1)

    // Leer caracteres hasta encontrar newline o EOF
    while ((c = getchar()) != EOF && c != '\n')
    {
        // Filtrar vocales (mayúsculas y minúsculas)
        if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' &&
            c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U')
        {
            putchar(c);
        }
    }

    // Imprimir newline al final si no fue EOF
    if (c == '\n')
    {
        putchar('\n');
    }

    return 0;
}

/**
 * cat_wrapper - Lee una línea de stdin y la imprime
 *
 * Usage: cat
 *
 * Lee caracteres desde stdin hasta encontrar un newline o EOF.
 * Imprime los caracteres tal como los recibe.
 */
int cat_wrapper(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int c; // Debe ser int, no char, para poder comparar con EOF (-1)

    // Leer caracteres hasta encontrar newline o EOF
    while ((c = getchar()) != EOF && c != '\n')
    {
        putchar(c);
    }

    // Imprimir newline al final si no fue EOF
    if (c == '\n')
    {
        putchar('\n');
    }

    return 0;
}

/**
 * wc_wrapper - Cuenta caracteres de una línea de entrada
 *
 * Usage: wc
 *
 * Lee caracteres desde stdin hasta encontrar un newline o EOF.
 * Muestra el total de caracteres leídos (sin contar el newline).
 */
int wc_wrapper(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int char_count = 0;
    int c; // Debe ser int, no char, para poder comparar con EOF (-1)

    // Leer caracteres hasta encontrar newline o EOF
    while ((c = getchar()) != EOF && c != '\n')
    {
        char_count++;
    }

    // Mostrar el total de caracteres (sin contar el \n)
    printf("%d\n", char_count);

    return 0;
}
