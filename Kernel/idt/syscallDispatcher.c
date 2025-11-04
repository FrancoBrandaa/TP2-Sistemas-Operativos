#include <syscallDispatcher.h>
#include <stddef.h>
#include <sound.h>
#include <keyboard.h>
#include <fonts.h>
#include <lib.h>
#include <video.h>
#include <time.h>
#include <memoryManager.h>
#include <process.h>
#include <scheduler.h>
#include <semaphoreManager.h>
#include <fds.h>

extern int64_t register_snapshot[18];
extern int64_t register_snapshot_taken;

// @todo Note: Technically.. registers on the stack are modifiable (since its a struct pointer, not struct).
int32_t syscallDispatcher(Registers *registers)
{
	switch (registers->rax)
	{
	case 3:
		return sys_read(registers->rdi, (signed char *)registers->rsi, registers->rdx);
	// Note: Register parameters are 64-bit
	case 4:
		return sys_write(registers->rdi, (char *)registers->rsi, registers->rdx);

	case 0x80000000:
		return sys_start_beep(registers->rdi);
	case 0x80000001:
		return sys_stop_beep();
	case 0x80000002:
		return sys_fonts_text_color(registers->rdi);
	case 0x80000003:
		return sys_fonts_background_color(registers->rdi);
	case 0x80000004: /* Reserved for sys_set_italics */
	case 0x80000005: /* Reserved for sys_set_bold */
	case 0x80000006: /* Reserved for sys_set_underline */
		return -1;
	case 0x80000007:
		return sys_fonts_decrease_size();
	case 0x80000008:
		return sys_fonts_increase_size();
	case 0x80000009:
		return sys_fonts_set_size((uint8_t)registers->rdi);
	case 0x8000000A:
		return sys_clear_screen();
	case 0x8000000B:
		return sys_clear_input_buffer();

	case 0x80000010:
		return sys_hour((int *)registers->rdi);
	case 0x80000011:
		return sys_minute((int *)registers->rdi);
	case 0x80000012:
		return sys_second((int *)registers->rdi);

	case 0x80000019:
		return sys_circle(registers->rdi, registers->rsi, registers->rdx, registers->rcx);
	case 0x80000020:
		return sys_rectangle(registers->rdi, registers->rsi, registers->rdx, registers->rcx, registers->r8);
	case 0x80000021:
		return sys_fill_video_memory(registers->rdi);

	case 0x800000A0:
		return sys_exec((int (*)(void))registers->rdi);

	case 0x800000B0:
		return sys_register_key((uint8_t)registers->rdi, (SpecialKeyHandler)registers->rsi);

	case 0x800000C0:
		return sys_window_width();
	case 0x800000C1:
		return sys_window_height();

	case 0x800000D0:
		return sys_sleep_milis(registers->rdi);

	case 0x800000E0:
		return sys_get_register_snapshot((int64_t *)registers->rdi);

	case 0x800000F0:
		return sys_get_character_without_display();

	case 0x80000100:
		return sys_get_mem_status((MemoryStatus *)registers->rdi);
	case 0x80000101:
		return (int64_t)sys_malloc(registers->rdi);
	case 0x80000102:
		return sys_free((void *)registers->rdi);

	case 0x80000200: //chequiar 
		return sys_create_process((const char *)registers->rdi,
								  (int (*)(int, char **))registers->rsi,
								  registers->rdx,
								  (char **)registers->rcx,
								  registers->r8,
								  registers->r9,
								  (int *)registers->r10);

	case 0x80000201:
		return sys_getpid();
	case 0x80000202:
		return sys_kill(registers->rdi);
	case 0x80000203:
		return sys_block(registers->rdi);
	case 0x80000204:
		return sys_unblock(registers->rdi);
	case 0x80000205:
		return (int64_t)sys_ps();
	case 0x80000206:
		return sys_change_priority(registers->rdi, registers->rsi);
	case 0x80000207:
		return sys_yield();
	case 0x80000208:
		return sys_wait(registers->rdi, (int *)registers->rsi);

	case 0x80000300:
		return sys_sem_open((const char *)registers->rdi, registers->rsi);
	case 0x80000301:
		return sys_sem_close(registers->rdi);
	case 0x80000302:
		sys_sem_wait(registers->rdi);
		return 0;
	case 0x80000303:
		sys_sem_post(registers->rdi);
		return 0;
	case 0x80000304:
		return sys_sem_value(registers->rdi);
	case 0x80000305:
		sys_sem_destroy(registers->rdi);
		return 0;

	// File descriptor syscalls
	case 0x80000400:
		return sys_pipe((int *)registers->rdi);
	case 0x80000401:
		return sys_close(registers->rdi);
	case 0x80000402:
		return sys_get_fd((int *)registers->rdi);
	case 0x80000403:
		return sys_read_at_current_pos(registers->rdi, (char *)registers->rsi, registers->rdx);

	default:
		return 0;
	}
}

// ==================================================================
// Linux syscalls
// ==================================================================

int32_t sys_write(int32_t fd, char *__user_buf, int32_t count)
{
	//usar el sistema de fds
	return writeToFD(fd, __user_buf, count, getTextColor());
	//return printToFd(fd, __user_buf, count);
}

int32_t sys_read(int32_t fd, signed char *__user_buf, int32_t count)
{
	// Para otros FDs (pipes, etc.) usar el sistema de streams
    // if (fd < 0 || fd >= MAX_FDS || !fileDescriptors[fd].isOpen || fileDescriptors[fd].mode == W) {
    //     return -1; // FD inválido o no tiene permiso de lectura
    // }
	//usar el sistema de fds
	return readFromFD(fd, (char *)__user_buf, count);

	// lo viejo
	// int32_t i;
	// int8_t c;
	// for (i = 0; i < count && (c = getKeyboardCharacter(AWAIT_RETURN_KEY | SHOW_BUFFER_WHILE_TYPING)) != EOF; i++)
	// {
	// 	*(__user_buf + i) = c;
	// }
	// return i;
}

// ==================================================================
// Custom system calls
// ==================================================================

int32_t sys_start_beep(uint32_t nFrequence)
{
	play_sound(nFrequence);
	return 0;
}

int32_t sys_stop_beep(void)
{
	setSpeaker(SPEAKER_OFF);
	return 0;
}

int32_t sys_fonts_text_color(uint32_t color)
{
	setTextColor(color);
	return 0;
}

int32_t sys_fonts_background_color(uint32_t color)
{
	setBackgroundColor(color);
	return 0;
}

int32_t sys_fonts_decrease_size(void)
{
	return decreaseFontSize();
}

int32_t sys_fonts_increase_size(void)
{
	return increaseFontSize();
}

int32_t sys_fonts_set_size(uint8_t size)
{
	return setFontSize(size);
}

int32_t sys_clear_screen(void)
{
	clear();
	return 0;
}

int32_t sys_clear_input_buffer(void)
{
	while (clearBuffer() != 0)
		;
	return 0;
}

uint16_t sys_window_width(void)
{
	return getWindowWidth();
}

uint16_t sys_window_height(void)
{
	return getWindowHeight();
}

// ==================================================================
// Date system calls
// ==================================================================

int32_t sys_hour(int *hour)
{
	*hour = getHour();
	return 0;
}

int32_t sys_minute(int *minute)
{
	*minute = getMinute();
	return 0;
}

int32_t sys_second(int *second)
{
	*second = getSecond();
	return 0;
}

// ==================================================================
// Draw system calls
// ==================================================================

int32_t sys_circle(uint32_t hexColor, uint64_t topLeftX, uint64_t topLeftY, uint64_t diameter)
{
	drawCircle(hexColor, topLeftX, topLeftY, diameter);
	return 0;
}

int32_t sys_rectangle(uint32_t color, uint64_t width_pixels, uint64_t height_pixels, uint64_t initial_pos_x, uint64_t initial_pos_y)
{
	drawRectangle(color, width_pixels, height_pixels, initial_pos_x, initial_pos_y);
	return 0;
}

int32_t sys_fill_video_memory(uint32_t hexColor)
{
	fillVideoMemory(hexColor);
	return 0;
}

// ==================================================================
// Custom exec system call
// ==================================================================

int32_t sys_exec(int32_t (*fnPtr)(void))
{
	clear();

	uint8_t fontSize = getFontSize();				  // preserve font size
	uint32_t text_color = getTextColor();			  // preserve text color
	uint32_t background_color = getBackgroundColor(); // preserve background color

	SpecialKeyHandler map[F12_KEY - ESCAPE_KEY + 1] = {0};
	clearKeyFnMapNonKernel(map); // avoid """processes/threads/apps""" registering keys across each other over time. reset the map every time

	int32_t aux = fnPtr();

	restoreKeyFnMapNonKernel(map);
	setFontSize(fontSize);
	setTextColor(text_color);
	setBackgroundColor(background_color);

	clear();
	return aux;
}

// ==================================================================
// Custom keyboard system calls
// ==================================================================

int32_t sys_register_key(uint8_t scancode, SpecialKeyHandler fn)
{
	registerSpecialKey(scancode, fn, 0);
	return 0;
}

// ==================================================================
// Sleep system calls
// ==================================================================
int32_t sys_sleep_milis(uint32_t milis)
{
	sleepTicks((milis * SECONDS_TO_TICKS) / 1000);
	return 0;
}

// ==================================================================
// Register snapshot system calls
// ==================================================================
int32_t sys_get_register_snapshot(int64_t *registers)
{
	if (register_snapshot_taken == 0)
		return 0;

	uint8_t i = 0;

	while (i < 18)
	{
		*(registers++) = register_snapshot[i++];
	}

	return 1;
}

int32_t sys_get_character_without_display(void)
{
	return getKeyboardCharacter(0);
}

// ==================================================================
// Memory management system calls
// ==================================================================

int32_t sys_get_mem_status(MemoryStatus *memStatus)
{
	getMemoryStatus(memStatus);
	return 0;
}

void *sys_malloc(int size)
{
	return allocMemory(size);
}

int32_t sys_free(void *ptr)
{
	freeMemory(ptr);
	return 0;
}

// ==================================================================
// Process management system calls
// ==================================================================

long sys_create_process(const char *name, int (*entry)(int, char **), int argc, char **argv, int priority, int foreground, int fds[2])
{
	creationParameters params;
	params.name = (char *)name;
	params.entryPoint = entry;
	params.argc = argc;
	params.argv = argv;
	params.priority = priority;
	params.foreground = foreground;
	params.fds[0] = fds[0];
	params.fds[1] = fds[1];

	return createProcess(&params);
}

long sys_getpid(void)
{
	return getpid();
}

int32_t sys_kill(long pid)
{
	return kill(pid);
}

int32_t sys_block(long pid)
{
	if (pid == 1 || !isValidPID(pid))
		return -1; // El usuario no puede bloquear el proceso init
	return blockProcess(pid);
}

int32_t sys_unblock(long pid)
{
	return unblockProcess(pid);
}

void *sys_ps(void)
{
	return getProcessesInformation();
}

int32_t sys_change_priority(long pid, int32_t priority)
{
	return changeProccessPriority(pid, priority);
}

int32_t sys_yield(void)
{
	yield();
	return 0;
}

int32_t sys_wait(long pid, int *wstatus)
{
	// posible checkeo por si hace falta?
	waitProcess(pid, wstatus);
	return 0;
}

// ==================================================================
// Semaphore system calls
// ==================================================================

int32_t sys_sem_open(const char *name, int value)
{
	int semId;

	if (name == NULL)
	{
		return -1;
	}

	if (findSemByName(name) < 0)
	{
		semId = semCreate(name, value);

		if (semId < 0)
		{
			return -1;
		}

		return semId;
	}
	return semOpen(name);
}

int32_t sys_sem_close(int semId)
{
	return semClose(semId);
}

void sys_sem_wait(int semId)
{
	if (semId < 0 || !semExists(semId))
	{
		return;
	}

	semWait(semId);
}

void sys_sem_post(int semId)
{
	if (semId < 0 || !semExists(semId))
	{
		return;
	}

	semPost(semId);
}

int32_t sys_sem_value(int semId)
{
	return semValue(semId);
}

void sys_sem_destroy(int semId)
{
	if (semId < 0 || !semExists(semId))
	{
		return;
	}

	semDestroy(semId);
}

// ==================================================================
// File descriptor system calls
// ==================================================================

int32_t sys_pipe(int *fds)
{
	if (fds == NULL) {
		return -1;
	}
	return createPipe(fds);
}

int32_t sys_close(int fd)
{
	return closeFD(fd);
}

int32_t sys_get_fd(int *fds)
{
	if (fds == NULL) {
		return -1; // Error: fds is NULL
	}

	return getFileDescriptors(fds);
}

int32_t sys_read_at_current_pos(int fd, char *buf, int count)
{
	return readFromFDAt(fd, buf, count, getReadPos(fd));
}