#include <libsys.h>
#include <syscalls.h>
#include <process.h>

void startBeep(uint32_t nFrequence)
{
    sys_start_beep(nFrequence);
}

void stopBeep(void)
{
    sys_stop_beep();
}

void setTextColor(uint32_t color)
{
    sys_fonts_text_color(color);
}

void setBackgroundColor(uint32_t color)
{
    sys_fonts_background_color(color);
}

uint8_t increaseFontSize(void)
{
    return sys_fonts_increase_size();
}

uint8_t decreaseFontSize(void)
{
    return sys_fonts_decrease_size();
}

uint8_t setFontSize(uint8_t size)
{
    return sys_fonts_set_size(size);
}

void getDate(int *hour, int *minute, int *second)
{
    sys_hour(hour);
    sys_minute(minute);
    sys_second(second);
}

void clearScreen(void)
{
    sys_clear_screen();
}

void clearInputBuffer(void)
{
    sys_clear_input_buffer();
}

void drawCircle(uint32_t color, long long int topleftX, long long int topLefyY, long long int diameter)
{
    sys_circle(color, topleftX, topLefyY, diameter);
}

void drawRectangle(uint32_t color, long long int width_pixels, long long int height_pixels, long long int initial_pos_x, long long int initial_pos_y)
{
    sys_rectangle(color, width_pixels, height_pixels, initial_pos_x, initial_pos_y);
}

void fillVideoMemory(uint32_t hexColor)
{
    sys_fill_video_memory(hexColor);
}

int32_t exec(int32_t (*fnPtr)(void))
{
    return sys_exec(fnPtr);
}

void registerKey(enum REGISTERABLE_KEYS scancode, void (*fn)(enum REGISTERABLE_KEYS scancode))
{
    sys_register_key(scancode, fn);
}

int getWindowWidth(void)
{
    return sys_window_width();
}

int getWindowHeight(void)
{
    return sys_window_height();
}

void sleep(uint32_t miliseconds)
{
    sys_sleep_milis(miliseconds);
}

int32_t getRegisterSnapshot(int64_t *registers)
{
    return sys_get_register_snapshot(registers);
}

int32_t getCharacterWithoutDisplay(void)
{
    return sys_get_character_without_display();
}

/* Memory management wrappers */
int32_t getMemoryStatus(void * memStatus)
{
    return sys_get_mem_status(memStatus);
}

void *malloc(int size)
{
    return sys_malloc(size);
}

int32_t free(void *ptr)
{
    return sys_free(ptr);
}

// Process management wrappers
long createProcess(const char *name, int (*entry)(int, char **), int argc, char **argv, int priority, int foreground, int fds[2])
{
    creationParameters params = {
        .argc = argc,
        .argv = argv,
        .name = (char *)name,
        .priority = priority,
        .entryPoint = entry,
        .foreground = foreground,
        .fds = {fds[0], fds[1]}
    };
    return sys_create_process(&params);
}

long getpid(void)
{
    return sys_getpid();
}

int32_t kill(long pid)
{
    return sys_kill(pid);
}

int32_t block(long pid)
{
    return sys_block(pid);
}

int32_t unblock(long pid)
{
    return sys_unblock(pid);
}

Process *getProcessList(void)
{
     return (Process *) sys_ps();
}

int32_t nice(long pid, int32_t priority)
{
    return sys_change_priority(pid, priority);
}

int32_t yield(void)
{
    return sys_yield();
}

int32_t wait(long pid, int32_t *wstatus)
{
    return sys_wait(pid, wstatus);
}

// Semaphore management wrappers
int32_t semOpen(const char *name, int value)
{
    return sys_sem_open(name, value);
}

int32_t semClose(int semId)
{
    return sys_sem_close(semId);
}

void semWait(int semId)
{
    sys_sem_wait(semId);
}

void semPost(int semId)
{
    sys_sem_post(semId);
}

int32_t semValue(int semId)
{
    return sys_sem_value(semId);
}

void semDestroy(int semId)
{
    sys_sem_destroy(semId);
}

// ==================================================================
// File descriptor management wrappers
// ==================================================================

int32_t pipe(int fds[2])
{
    return sys_pipe(fds);
}

int32_t close(int fd)
{
    return sys_close(fd);
}

int32_t getFD(int *fds)
{
    return sys_get_fd(fds);
}

int32_t readAtCurrentPos(int fd, char *buf, int count)
{
    return sys_read_at_current_pos(fd, buf, count);
}

int32_t read(int fd, void *buf, int count)
{
    return sys_read(fd, buf, count);
}

int32_t write(int fd, const void *buf, int count)
{
    return sys_write(fd, buf, count);
}

// Aliases for consistency
int32_t openPipe(int fds[2])
{
    return sys_pipe(fds);
}

int32_t closeFD(int fd)
{
    return sys_close(fd);
}
