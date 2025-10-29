# Guía de Debugging con GDB

## Configuración Completa

Se ha configurado GDB para debuggear el kernel del sistema operativo. Los archivos creados/modificados son:

### Archivos de Configuración

1. **`.gdbinit`** - Script de inicialización de GDB
2. **`debug.sh`** - Script para ejecutar QEMU con GDB server
3. **`.vscode/launch.json`** - Configuración de debug para VS Code
4. **`.vscode/tasks.json`** - Tareas de compilación para VS Code
5. **`Kernel/Makefile.inc`** - Modificado para incluir símbolos de debug (`-g -ggdb`)

## Métodos de Debugging
### Opción 2: Desde Terminal

1. **Terminal 1 - Ejecutar QEMU con GDB server:**

   ```bash
   ./debug.sh
   ```

   Esto inicia QEMU pausado esperando que GDB se conecte.

2. **Terminal 2 - Conectar GDB:**

   ```bash
   gdb -x .gdbinit
   ```

3. **Continuar ejecución:**
   ```
   (gdb) continue
   ```

### Opción 3: QEMU sin pausar al inicio

Si quieres que QEMU inicie normalmente pero con GDB disponible:

```bash
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -s
```

Luego conecta GDB cuando lo necesites:

```bash
gdb -x .gdbinit
```

## Comandos Útiles de GDB

### Breakpoints

```gdb
break kmain              # Break en función kmain
break kernel.c:42        # Break en línea específica
break *0x100000          # Break en dirección de memoria
info breakpoints         # Listar breakpoints
delete 1                 # Eliminar breakpoint #1
```

### Ejecución

```gdb
continue (c)             # Continuar ejecución
step (s)                 # Step into (una línea)
next (n)                 # Step over (una línea)
stepi (si)               # Step una instrucción
nexti (ni)               # Next una instrucción
finish                   # Ejecutar hasta return
```

### Inspección

```gdb
print variable           # Imprimir variable
print *pointer           # Imprimir valor apuntado
print/x variable         # Imprimir en hexadecimal
info registers           # Ver registros
info frame               # Ver stack frame
backtrace (bt)           # Ver call stack
list                     # Ver código fuente
disassemble              # Ver desensamblado
```

### Memoria

```gdb
x/10x 0x100000          # Examinar 10 words en hex desde dirección
x/10i 0x100000          # Examinar 10 instrucciones
x/s 0x100000            # Examinar string
watch variable          # Break cuando variable cambie
```

### Comandos Personalizados

```gdb
reload                   # Recargar símbolos del kernel
reset                    # Resetear el sistema (QEMU)
```

### Monitor Commands (QEMU)

```gdb
monitor info registers   # Información de QEMU
monitor system_reset     # Reset del sistema
monitor system_powerdown # Apagar QEMU
```

## Layout TUI (Terminal User Interface)

Para una mejor experiencia visual en terminal:

```gdb
layout split            # Código + assembly
layout src              # Solo código
layout asm              # Solo assembly
layout regs             # Registros
focus cmd               # Focus en comandos
refresh                 # Refrescar pantalla
```

O presiona `Ctrl+X` luego `A` para alternar TUI.

## Debugging de Assembly

Para debuggear código assembly:

```gdb
set disassembly-flavor intel    # Sintaxis Intel (ya configurado)
disassemble /m function_name    # Desensamblado con código fuente
stepi                           # Step instrucción por instrucción
info registers                  # Ver todos los registros
info registers rax rbx rcx      # Ver registros específicos
```

## Tips

1. **Recompilar con símbolos:** Después de cambios, recompila:

   ```bash
   make clean && make
   ```

2. **Si GDB no encuentra símbolos:**

   ```gdb
   file Kernel/kernel.bin
   ```

3. **Para debuggear desde el inicio:**

   - Usa la configuración "Debug Kernel (GDB + QEMU)" en VS Code
   - O ejecuta `./debug.sh` y luego `gdb -x .gdbinit`

4. **Breakpoint en el entry point:**

   ```gdb
   break _start
   continue
   ```

5. **Ver stack trace completo:**
   ```gdb
   bt full
   ```

## Solución de Problemas

### GDB no se conecta

- Verifica que QEMU esté ejecutándose con `-s`
- Verifica que el puerto 1234 no esté en uso: `netstat -an | grep 1234`

### No hay símbolos de debug

- Verifica que compilaste con las flags `-g -ggdb`
- Ejecuta: `make clean && make`

### QEMU no inicia

- Verifica que la imagen existe: `ls -lh Image/x64BareBonesImage.qcow2`
- Compila todo: `make clean && make all`

## Arquitectura del Sistema

- **Arquitectura:** x86-64
- **Kernel Binario:** `/home/mateuss/SO/TP2-Sistemas-Operativos/Kernel/kernel.bin` (formato binario plano para bootloader)
- **Kernel Debug:** `/home/mateuss/SO/TP2-Sistemas-Operativos/Kernel/kernel.elf` (formato ELF con símbolos de debug)
- **Entry Point:** `_start` en `loader.asm`
- **Puerto GDB:** 1234

**Nota importante:** El proceso de compilación genera dos archivos:
- `kernel.elf`: Ejecutable ELF con símbolos de debug completos (usado por GDB)
- `kernel.bin`: Binario plano (usado por el bootloader QEMU)

## Recursos Adicionales

- [GDB Cheat Sheet](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf)
- [Debugging with GDB Manual](https://sourceware.org/gdb/documentation/)
- [QEMU GDB Support](https://qemu.readthedocs.io/en/latest/system/gdb.html)
