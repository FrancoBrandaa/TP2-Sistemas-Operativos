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
#include <string.h>
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

    // This line is never reached, but included for completeness
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

    printf("\e[1;33m[Process %d] Monitoring shell state. Press any key to see shell unblock.\e[0m\n\n", my_pid);

    for (int iteration = 0; iteration < 20; iteration++) // 20 segundos de monitoreo
    {
        Process *processes = getProcessList();

        if (processes != NULL)
        {
            printf("\e[1;36m--- Iteration %d ---\e[0m\n", iteration + 1);

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
 * filter_wrapper - Filtra vocales de stdin (estilo Linux)
 *
 * Usage: filter
 *
 * Lee toda la entrada desde stdin hasta EOF.
 * Filtra todas las vocales (mayúsculas y minúsculas).
 * Imprime solo las consonantes, números y símbolos.
 * Para finalizar la entrada, presionar Ctrl+D.
 */
int filter_wrapper(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int c;

    // Leer todo stdin hasta EOF
    while ((c = getchar()) != EOF)
    {
        // Filtrar vocales (mayúsculas y minúsculas)
        if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' &&
            c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U')
        {
            putchar(c);
        }
    }

    return 0;
}

/**
 * cat_wrapper - Concatena y muestra stdin (estilo Linux)
 *
 * Usage: cat
 *
 * Lee toda la entrada desde stdin hasta EOF.
 * Imprime los caracteres tal como los recibe.
 * Para finalizar la entrada, presionar Ctrl+D.
 */
int cat_wrapper(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int c;

    // Leer todo stdin hasta EOF
    while ((c = getchar()) != EOF)
    {
        putchar(c);
    }

    return 0;
}

/**
 * wc_wrapper - Cuenta líneas, palabras y caracteres (estilo Linux)
 *
 * Usage: wc [-l] [-w] [-c]
 *   -l: Solo contar líneas
 *   -w: Solo contar palabras
 *   -c: Solo contar caracteres
 *   (sin opciones: muestra líneas, palabras y caracteres)
 *
 * Lee toda la entrada desde stdin hasta EOF.
 * Para finalizar la entrada, presionar Ctrl+D.
 */
int wc_wrapper(int argc, char **argv)
{
    int line_count = 0;
    int word_count = 0;
    int char_count = 0;
    int in_word = 0;
    int c;

    // Flags para las opciones
    int show_lines = 0;
    int show_words = 0;
    int show_chars = 0;
    int show_all = 1; // Por defecto muestra todo

    // Parsear argumentos
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            show_all = 0; // Si hay opciones, no mostrar todo por defecto
            for (int j = 1; argv[i][j] != '\0'; j++)
            {
                switch (argv[i][j])
                {
                case 'l':
                    show_lines = 1;
                    break;
                case 'w':
                    show_words = 1;
                    break;
                case 'c':
                    show_chars = 1;
                    break;
                default:
                    printf("wc: invalid option -- '%c'\n", argv[i][j]);
                    printf("Usage: wc [-l] [-w] [-c]\n");
                    return 1;
                }
            }
        }
    }

    // Si se especificó show_all, habilitar todas las opciones
    if (show_all)
    {
        show_lines = 1;
        show_words = 1;
        show_chars = 1;
    }

    // Leer todo stdin hasta EOF
    while ((c = getchar()) != EOF)
    {
        char_count++;

        // Contar líneas
        if (c == '\n')
        {
            line_count++;
            if (in_word)
            {
                word_count++;
                in_word = 0;
            }
        }
        // Contar palabras (separadas por espacios, tabs, newlines)
        else if (c == ' ' || c == '\t' || c == '\r')
        {
            if (in_word)
            {
                word_count++;
                in_word = 0;
            }
        }
        else
        {
            in_word = 1;
        }
    }

    // Si terminó en medio de una palabra, contarla
    if (in_word)
    {
        word_count++;
    }

    // Mostrar resultados según las opciones
    if (show_lines)
    {
        printf("%d lineas\n", line_count);
    }
    if (show_words)
    {
        printf("%d palabras\n", word_count);
    }
    if (show_chars)
    {
        printf("%d caracteres\n", char_count);
    }


    return 0;
}
