
all:  bootloader kernel userland image

bootloader:
	cd Bootloader; make all

kernel:
	cd Kernel; make all

userland:
	cd Userland; make all

image: kernel bootloader userland
	cd Image; make all

clean:
	cd Bootloader; make clean
	cd Image; make clean
	cd Kernel; make clean
	cd Userland; make clean
	rm -f *.zip

# Memory manager selection targets - compile entire OS
naive:
	@echo "🔧 Building complete OS with Naive Memory Manager..."
	cd Bootloader; make all
	cd Kernel; make naive
	cd Userland; make all
	cd Image; make all
	@echo "✅ Complete OS built with Naive Memory Manager"

buddy:
	@echo "🔧 Building complete OS with Buddy Memory Manager..."
	cd Bootloader; make all
	cd Kernel; make buddy
	cd Userland; make all
	cd Image; make all
	@echo "✅ Complete OS built with Buddy Memory Manager"

# Show current kernel memory manager
status:
	cd Kernel; make status

.PHONY: bootloader image collections kernel userland all clean naive buddy status
