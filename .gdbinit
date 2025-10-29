# GDB initialization script for x64 BareBones OS
# This file is automatically loaded when GDB starts

# Connect to QEMU's GDB server
target remote localhost:1234

# Set architecture
set architecture i386:x86-64

# Load symbol file
file Kernel/kernel.bin

# Display assembly in Intel syntax (more readable)
set disassembly-flavor intel

# Enable pretty printing
set print pretty on

# Show all struct members
set print object on

# Display source lines when stopping
set listsize 20

# Auto-load breakpoint commands
set breakpoint pending on

# Disable pagination (useful for automated scripts)
set pagination off

# Layout configuration (uncomment if you want TUI mode by default)
# layout split
# focus cmd

# Useful breakpoints (uncomment as needed)
# break kmain
# break _start
# break exceptionDispatcher

# Custom commands
define hook-stop
    # This runs every time execution stoys
    # info registers
end

# Helper command to reload symbols
define reload
    file Kernel/kernel.bin
    echo Symbols reloaded\n
end

# Helper command to reset and restart
define reset
    monitor system_reset
    continue
end

echo \n
echo ========================================\n
echo GDB connected to QEMU\n
echo ========================================\n
echo Useful commands:\n
echo   reload  - Reload kernel symbols\n
echo   reset   - Reset the system\n
echo   c       - Continue execution\n
echo   si/ni   - Step instruction\n
echo   s/n     - Step line\n
echo ========================================\n
echo \n
