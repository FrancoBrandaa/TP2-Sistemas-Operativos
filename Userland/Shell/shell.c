#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <sys.h>
#include <exceptions.h>

#ifdef ANSI_4_BIT_COLOR_SUPPORT
    #include <ansiColors.h>
#endif

// Test utility functions from test_util.c
static uint32_t m_z = 362436069;
static uint32_t m_w = 521288629;

uint32_t GetUint() {
  m_z = 36969 * (m_z & 65535) + (m_z >> 16);
  m_w = 18000 * (m_w & 65535) + (m_w >> 16);
  return (m_z << 16) + m_w;
}

uint32_t GetUniform(uint32_t max) {
  uint32_t u = GetUint();
  return (u + 1.0) * 2.328306435454494e-10 * max;
}

uint8_t memcheck(void *start, uint8_t value, uint32_t size) {
  uint8_t *p = (uint8_t *)start;
  uint32_t i;

  for (i = 0; i < size; i++, p++)
    if (*p != value)
      return 0;

  return 1;
}

int64_t satoi(char *str) {
  uint64_t i = 0;
  int64_t res = 0;
  int8_t sign = 1;

  if (!str)
    return 0;

  if (str[i] == '-') {
    i++;
    sign = -1;
  }

  for (; str[i] != '\0'; ++i) {
    if (str[i] < '0' || str[i] > '9')
      return 0;
    res = res * 10 + str[i] - '0';
  }

  return res * sign;
}

#define MAX_BLOCKS 128

typedef struct MM_rq {
  void *address;
  uint32_t size;
} mm_rq;

static void * const snakeModuleAddress = (void*)0x500000;

#define MAX_BUFFER_SIZE 1024
#define HISTORY_SIZE 10

#define INC_MOD(x, m) x = (((x) + 1) % (m))
#define SUB_MOD(a, b, m) ((a) - (b) < 0 ? (m) - (b) + (a) : (a) - (b))
#define DEC_MOD(x, m) ((x) = SUB_MOD(x, 1, m))

static char buffer[MAX_BUFFER_SIZE];
static int buffer_dim = 0;

int clear(void);
int echo(void);
int exit(void);
int fontdec(void);
int font(void);
int help(void);
int history(void);
int man(void);
int snake(void);
int regs(void);
int time(void);
int memtest(void);
int memstress(void);
int test_mm(void);

static void printPreviousCommand(enum REGISTERABLE_KEYS scancode);
static void printNextCommand(enum REGISTERABLE_KEYS scancode);

static uint8_t last_command_arrowed = 0;

typedef struct {
    char * name;
    int (*function)(void);
    char * description;
} Command;

/* All available commands. Sorted alphabetically by their name */
Command commands[] = {
    { .name = "clear",          .function = (int (*)(void))(unsigned long long)clear,           .description = "Clears the screen" },
    { .name = "divzero",        .function = (int (*)(void))(unsigned long long)_divzero,        .description = "Generates a division by zero exception" },
    { .name = "echo",           .function = (int (*)(void))(unsigned long long)echo ,           .description = "Prints the input string" },
    { .name = "exit",           .function = (int (*)(void))(unsigned long long)exit,            .description = "Command exits w/ the provided exit code or 0" },
    { .name = "font",           .function = (int (*)(void))(unsigned long long)font,            .description = "Increases or decreases the font size.\n\t\t\t\tUse:\n\t\t\t\t\t  + font increase\n\t\t\t\t\t  + font decrease" },
    { .name = "help",           .function = (int (*)(void))(unsigned long long)help,            .description = "Prints the available commands" },
    { .name = "history",        .function = (int (*)(void))(unsigned long long)history,         .description = "Prints the command history" },
    { .name = "invop",          .function = (int (*)(void))(unsigned long long)_invalidopcode,  .description = "Generates an invalid Opcode exception" },
    { .name = "memstress",      .function = (int (*)(void))(unsigned long long)memstress,       .description = "Stress test for dynamic memory allocation" },
    { .name = "memtest",        .function = (int (*)(void))(unsigned long long)memtest,         .description = "Simple test for dynamic memory allocation" },
    { .name = "regs",           .function = (int (*)(void))(unsigned long long)regs,            .description = "Prints the register snapshot, if any" },
    { .name = "man",            .function = (int (*)(void))(unsigned long long)man,             .description = "Prints the description of the provided command" },
    { .name = "snake",          .function = (int (*)(void))(unsigned long long)snake,           .description = "Launches the snake game" },
    { .name = "test_mm",        .function = (int (*)(void))(unsigned long long)test_mm,         .description = "Advanced memory manager test (original test_mm.c)" },
    { .name = "time",           .function = (int (*)(void))(unsigned long long)time,            .description = "Prints the current time" },
};

char command_history[HISTORY_SIZE][MAX_BUFFER_SIZE] = {0};
char command_history_buffer[MAX_BUFFER_SIZE] = {0};
uint8_t command_history_last = 0;

static uint64_t last_command_output = 0;

int main() {
    clear();

    registerKey(KP_UP_KEY, printPreviousCommand);
    registerKey(KP_DOWN_KEY, printNextCommand);

	while (1) {
        printf("\e[0mshell \e[0;32m$\e[0m ");

        signed char c;

        while(buffer_dim < MAX_BUFFER_SIZE && (c = getchar()) != '\n'){
            command_history_buffer[buffer_dim] = c;
            buffer[buffer_dim++] = c;
        }

        buffer[buffer_dim] = 0;
        command_history_buffer[buffer_dim] = 0;

        if(buffer_dim == MAX_BUFFER_SIZE){
            perror("\e[0;31mShell buffer overflow\e[0m\n");
            buffer[0] = buffer_dim = 0;
            while (c != '\n') c = getchar();
            continue ;
        };

        buffer[buffer_dim] = 0;
        
        char * command = strtok(buffer, " ");
        int i = 0;

        for (; i < sizeof(commands) / sizeof(Command); i++) {
            if (strcmp(commands[i].name, command) == 0) {
                last_command_output = commands[i].function();
                strncpy(command_history[command_history_last], command_history_buffer, 255);
                command_history[command_history_last][buffer_dim] = '\0';
                INC_MOD(command_history_last, HISTORY_SIZE);
                last_command_arrowed = command_history_last;
                break;
            }
        }

        // If the command is not found, ignore \n
        if ( i == sizeof(commands) / sizeof(Command) ) {
            if (command != NULL && *command != '\0') {
                fprintf(FD_STDERR, "\e[0;33mCommand not found:\e[0m %s\n", command);
            } else if (command == NULL) {
                printf("\n");
            }
        }
    
        buffer[0] = buffer_dim = 0;
    }

    __builtin_unreachable();
    return 0;
}

static void printPreviousCommand(enum REGISTERABLE_KEYS scancode) {
    clearInputBuffer();
    last_command_arrowed = SUB_MOD(last_command_arrowed, 1, HISTORY_SIZE);
    if (command_history[last_command_arrowed][0] != 0) {
        fprintf(FD_STDIN, command_history[last_command_arrowed]);
    }
}

static void printNextCommand(enum REGISTERABLE_KEYS scancode) {
    clearInputBuffer();
    last_command_arrowed = (last_command_arrowed + 1) % HISTORY_SIZE;
    if (command_history[last_command_arrowed][0] != 0) {
        fprintf(FD_STDIN, command_history[last_command_arrowed]);
    }
}

int history(void) {
    uint8_t last = command_history_last;
    DEC_MOD(last, HISTORY_SIZE);
    uint8_t i = 0;
    while (i < HISTORY_SIZE && command_history[last][0] != 0) {
        printf("%d. %s\n", i, command_history[last]);
        DEC_MOD(last, HISTORY_SIZE);
        i++;
    }
    return 0;
}

int time(void){
	int hour, minute, second;
    getDate(&hour, &minute, &second);
    printf("Current time: %xh %xm %xs\n", hour, minute, second);
    return 0;
}

int echo(void){
    for (int i = strlen("echo") + 1; i < buffer_dim; i++) {
        switch (buffer[i]) {
            case '\\':
                switch (buffer[i + 1]) {
                    case 'n':
                        printf("\n");
                        i++;
                        break;
                    case 'e':
                    #ifdef ANSI_4_BIT_COLOR_SUPPORT
                        i++;
                        parseANSI(buffer, &i); 
                    #else 
                        while (buffer[i] != 'm') i++; // ignores escape code, assumes valid format
                        i++;
                    #endif
                        break;
                    case 'r': 
                        printf("\r");
                        i++;
                        break;
                    case '\\':
                        i++;
                    default:
                        putchar(buffer[i]);
                        break;
                }
                break;
            case '$':
                if (buffer[i + 1] == '?'){
                    printf("%d", last_command_output);
                    i++;
                    break;
                }
            default:
                putchar(buffer[i]);
                break;
        }
    }
    printf("\n");
    return 0;
}

int help(void){
    printf("Available commands:\n");
    for (int i = 0; i < sizeof(commands) / sizeof(Command); i++) {
        printf("%s%s\t ---\t%s\n", commands[i].name, strlen(commands[i].name) < 4 ? "\t" : "", commands[i].description);
    }
    printf("\n");
    return 0;
}

int clear(void) {
    clearScreen();
    return 0;
}

int exit(void) {
    char * buffer = strtok(NULL, " ");
    int aux = 0;
    sscanf(buffer, "%d", &aux);
    return aux;
}

int font(void) {
    char * arg = strtok(NULL, " ");
    if (strcasecmp(arg, "increase") == 0) {
        return increaseFontSize();
    } else if (strcasecmp(arg, "decrease") == 0) {
        return decreaseFontSize();
    }
    
    perror("Invalid argument\n");
    return 0;
}

int man(void) {
    char * command = strtok(NULL, " ");

    if (command == NULL) {
        perror("No argument provided\n");
        return 1;
    }

    for (int i = 0; i < sizeof(commands) / sizeof(Command); i++) {
        if (strcasecmp(commands[i].name, command) == 0) {
            printf("Command: %s\nInformation: %s\n", commands[i].name, commands[i].description);
            return 0;
        }
    }

    perror("Command not found\n");
    return 1;
}

int regs(void) {
    const static char * register_names[] = {
        "rax", "rbx", "rcx", "rdx", "rbp", "rdi", "rsi", "r8 ", "r9 ", "r10", "r11", "r12", "r13", "r14", "r15", "rsp", "rip", "rflags"
    };

    int64_t registers[18];

    uint8_t aux = getRegisterSnapshot(registers);
    
    if (aux == 0) {
        perror("No register snapshot available\n");
        return 1;
    }

    printf("Latest register snapshot:\n");

    for (int i = 0; i < 18; i++) {
        printf("\e[0;34m%s\e[0m: %x\n", register_names[i], registers[i]);
    }

    return 0;
}

int snake(void) {
    return exec(snakeModuleAddress);
}

int memtest(void) {
    printf("Testing dynamic memory allocation...\n");
    
    // Test 1: Simple allocation and deallocation
    printf("Test 1: Simple allocation\n");
    void *ptr1 = malloc(100);
    if (ptr1 == NULL) {
        printf("  ERROR: malloc(100) failed\n");
        return 1;
    }
    printf("  SUCCESS: Allocated 100 bytes at %p\n", ptr1);
    
    // Write some data
    char *data = (char*)ptr1;
    for (int i = 0; i < 100; i++) {
        data[i] = (char)(i % 256);
    }
    
    // Verify data
    int corruption = 0;
    for (int i = 0; i < 100; i++) {
        if (data[i] != (char)(i % 256)) {
            corruption = 1;
            break;
        }
    }
    
    if (corruption) {
        printf("  ERROR: Data corruption detected\n");
        return 1;
    }
    printf("  SUCCESS: Data integrity verified\n");
    
    free(ptr1);
    printf("  SUCCESS: Memory freed\n");
    
    // Test 2: Multiple allocations
    printf("Test 2: Multiple allocations\n");
    void *ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = malloc(50 + i * 10);
        if (ptrs[i] == NULL) {
            printf("  ERROR: malloc failed on iteration %d\n", i);
            return 1;
        }
    }
    printf("  SUCCESS: Allocated 10 blocks\n");
    
    // Free all
    for (int i = 0; i < 10; i++) {
        free(ptrs[i]);
    }
    printf("  SUCCESS: All blocks freed\n");
    
    printf("Memory test completed successfully!\n");
    return 0;
}

int memstress(void) {
    printf("Stress testing dynamic memory allocation...\n");
    printf("This may take a while...\n");
    
    #define MAX_ALLOCS 50
    void *ptrs[MAX_ALLOCS];
    int allocated = 0;
    
    for (int iteration = 0; iteration < 5; iteration++) {
        printf("Iteration %d/5\n", iteration + 1);
        
        // Allocate random sizes
        for (int i = 0; i < MAX_ALLOCS; i++) {
            int size = 32 + (i * 17) % 512; // Pseudo-random sizes
            ptrs[i] = malloc(size);
            if (ptrs[i] != NULL) {
                allocated++;
                // Fill with pattern
                char *data = (char*)ptrs[i];
                for (int j = 0; j < size; j++) {
                    data[j] = (char)((i + j) % 256);
                }
            }
        }
        
        printf("  Allocated %d blocks\n", allocated);
        
        // Verify some blocks randomly
        for (int i = 0; i < MAX_ALLOCS; i += 5) {
            if (ptrs[i] != NULL) {
                int size = 32 + (i * 17) % 512;
                char *data = (char*)ptrs[i];
                for (int j = 0; j < size; j++) {
                    if (data[j] != (char)((i + j) % 256)) {
                        printf("  ERROR: Data corruption in block %d\n", i);
                        return 1;
                    }
                }
            }
        }
        
        // Free all allocated blocks
        for (int i = 0; i < MAX_ALLOCS; i++) {
            if (ptrs[i] != NULL) {
                free(ptrs[i]);
                ptrs[i] = NULL;
            }
        }
        
        allocated = 0;
        printf("  All blocks freed\n");
    }
    
    printf("Stress test completed successfully!\n");
    return 0;
}

int test_mm(void) {
    mm_rq mm_rqs[MAX_BLOCKS];
    uint8_t rq;
    uint32_t total;
    uint64_t max_memory;
    
    // Get max_memory from user input or use default
    char *arg = strtok(NULL, " ");
    if (arg != NULL) {
        max_memory = satoi(arg);
        if (max_memory <= 0) {
            printf("Invalid memory size. Using default (1024 bytes).\n");
            max_memory = 1024;
        }
    } else {
        max_memory = 1024; // Default value
    }
    
    printf("Starting advanced memory manager test...\n");
    printf("Max memory per iteration: %ld bytes\n", max_memory);
    printf("Press Ctrl+C to stop the test\n");
    
    int iterations = 0;
    int max_iterations = 10; // Limit iterations to avoid infinite loop
    
    while (iterations < max_iterations) {
        printf("Iteration %d/%d\n", iterations + 1, max_iterations);
        rq = 0;
        total = 0;

        // Request as many blocks as we can
        while (rq < MAX_BLOCKS && total < max_memory) {
            mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
            mm_rqs[rq].address = malloc(mm_rqs[rq].size);

            if (mm_rqs[rq].address) {
                total += mm_rqs[rq].size;
                rq++;
            } else {
                break; // No more memory available
            }
        }

        printf("  Allocated %d blocks, total: %d bytes\n", rq, total);

        // Set
        uint32_t i;
        for (i = 0; i < rq; i++)
            if (mm_rqs[i].address)
                memset(mm_rqs[i].address, i, mm_rqs[i].size);

        // Check
        for (i = 0; i < rq; i++)
            if (mm_rqs[i].address)
                if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size)) {
                    printf("  test_mm ERROR: Memory corruption detected!\n");
                    return -1;
                }

        printf("  Memory integrity check passed\n");

        // Free
        for (i = 0; i < rq; i++)
            if (mm_rqs[i].address)
                free(mm_rqs[i].address);

        printf("  Memory freed successfully\n");
        iterations++;
    }
    
    printf("Advanced memory manager test completed successfully!\n");
    printf("Completed %d iterations without errors\n", iterations);
    return 0;
}
