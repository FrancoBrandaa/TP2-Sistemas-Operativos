#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <video.h>
#include <idtLoader.h>
#include <fonts.h>
#include <syscallDispatcher.h>
#include <sound.h>
#include <memoryManager.h>
#include <process.h>
#include <scheduler.h>
#include <interrupts.h>
#include <semaphoreManager.h>
#include <fds.h>
#include <keyboard.h>

#define SHELL_PRIORITY MAX_PRIORITY
#define INIT_PRIORITY MIN_PRIORITY

// extern uint8_t text;
// extern uint8_t rodata;
// extern uint8_t data;

extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void *const shellModuleAddress = (void *)0x400000;

// HEAP
static void *const memoryStart = (void *)0xF00000;
const int memorySize = (1 << 22); // 4 MB (was 1 MB)

typedef int (*EntryPoint)();

void clearBSS(void *bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void *getStackBase()
{
	return (void *)((uint64_t)&endOfKernel + PageSize * 8 // The size of the stack itself, 32KiB
					- sizeof(uint64_t)					  // Begin at the top of the stack
	);
}

void *initializeKernelBinary()
{
	void *moduleAddresses[] = {
		shellModuleAddress,
	};

	loadModules(&endOfKernelBinary, moduleAddresses);

	clearBSS(&bss, &endOfKernel - &bss);

	return getStackBase();
}

void process_idle()
{
	while (1)
	{
		_hlt();
		// print("estoy en idle juju");
	}
}

int main()
{
	load_idt();

	createMemoryManager(memoryStart, memorySize);
	initSemManager();
	initFileDescriptors();
	initKeyboard();
	setFontSize(2);

	initProcesses();
	initScheduler();

	// habilitamos las interrupciones
	_sti();

	creationParameters params;
	params.name = "init";
	params.argc = 0;
	params.argv = NULL;
	params.priority = INIT_PRIORITY;
	params.entryPoint = (entryPoint)&process_idle;
	params.foreground = 0;
	params.fds[0] = 0; // STDIN
	params.fds[1] = 1; // STDOUT
	createProcess(&params);

	params.name = "shell";
	params.entryPoint = (entryPoint)shellModuleAddress;
	params.foreground = 1;
	params.argc = 0;
	params.argv = NULL;
	params.priority = SHELL_PRIORITY;
	params.fds[0] = 0; // STDIN
	params.fds[1] = 1; // STDOUT
	createProcess(&params);

	forceSwitchContext(); // para que arranque el primer proceso, no dependo del timer.

	return 0;
}
