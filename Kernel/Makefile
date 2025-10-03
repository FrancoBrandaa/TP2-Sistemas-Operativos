include Makefile.inc

KERNEL=kernel.bin
SOURCES=$(wildcard *.c ./drivers/*.c ./idt/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
HOT_OBJECTS=./drivers/video.o fonts.o # Compiled with -O3

# Memory manager selection (default: naive)
MEMORY_MANAGER ?= naive
ifeq ($(MEMORY_MANAGER),buddy)
    MEMORY_SRC = ./memory/buddyManager.c
else
    MEMORY_SRC = ./memory/naiveManager.c
endif

OBJECTS=$(SOURCES:.c=.o) $(MEMORY_SRC:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)

$(HOT_OBJECTS) : %.o: %.c
	$(GCC) -O3 $(GCCFLAGS) -I./include -c $< -o $@

$(filter-out $(HOT_OBJECTS),$(OBJECTS)) : %.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -I./font_assets -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# font.o:
# 	objcopy -O elf64-x86-64 -B i386 -I binary ./font_assets/Solarize.12x29.psf font.o

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf */*.o *.o *.bin

# Memory manager selection targets
naive:
	@echo "Switching to Naive Memory Manager..."
	$(MAKE) clean
	$(MAKE) MEMORY_MANAGER=naive all
	@echo "✅ Kernel compiled with Naive Memory Manager"

buddy:
	@echo "Switching to Buddy Memory Manager..."
	$(MAKE) clean
	$(MAKE) MEMORY_MANAGER=buddy all
	@echo "✅ Kernel compiled with Buddy Memory Manager"

# Show current memory manager
status:
	@echo "Current Memory Manager: $(MEMORY_MANAGER)"
	@echo "Memory Source: $(MEMORY_SRC)"

.PHONY: all clean naive buddy status
