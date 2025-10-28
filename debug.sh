#!/bin/bash
# Script to run QEMU with GDB server enabled for debugging

echo "Starting QEMU with GDB server on port 1234..."
echo "Connect GDB with: gdb -x .gdbinit"
echo "Or use VS Code debugger (F5)"
echo ""

qemu-system-x86_64 \
    -hda Image/x64BareBonesImage.qcow2 \
    -m 512 \
    -audiodev coreaudio,id=speaker \
    -machine pcspk-audiodev=speaker \
    -s \
    -S

# Flags explanation:
# -s: shorthand for -gdb tcp::1234 (enables GDB server on port 1234)
# -S: freeze CPU at startup (wait for GDB to connect and continue)
