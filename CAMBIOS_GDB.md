# Cambios Realizados para Soporte de GDB

## Problema Original
GDB no podía leer el archivo `kernel.bin` porque estaba en formato binario plano, no en formato ELF ejecutable que GDB necesita para leer símbolos de debug.

## Solución Implementada

### 1. Modificaciones en `Kernel/Makefile.inc`
- **Agregado `OBJCOPY`**: Herramienta para convertir formatos de archivos objeto
- **Flags de debug en GCC**: `-g -ggdb` para generar símbolos de debug DWARF
- **Flags de debug en NASM**: `-g -F dwarf` para información de debug en assembly

```makefile
OBJCOPY=x86_64-linux-gnu-objcopy
GCCFLAGS=... -g -ggdb
ASMFLAGS=-felf64 -g -F dwarf
```

### 2. Nuevo archivo `Kernel/kernel_elf.ld`
- Linker script específico para generar formato ELF
- Cambiado `OUTPUT_FORMAT("binary")` por `OUTPUT_FORMAT("elf64-x86-64")`
- Mantiene la misma estructura de memoria que el original

### 3. Modificaciones en `Kernel/Makefile`
- **Generación de dos archivos**:
  - `kernel.elf`: Formato ELF con símbolos de debug (para GDB)
  - `kernel.bin`: Formato binario plano (para el bootloader)
- **Proceso de compilación**:
  1. Compila y linkea con `kernel_elf.ld` → genera `kernel.elf`
  2. Convierte `kernel.elf` a binario plano → genera `kernel.bin`
- **Limpieza actualizada**: Ahora también elimina archivos `.elf`

```makefile
KERNEL_ELF=kernel.elf

$(KERNEL): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL)

$(KERNEL_ELF): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel_elf.ld -o $(KERNEL_ELF) ...
```

### 4. Actualización de `.gdbinit`
- Cambiado para cargar `Kernel/kernel.elf` en lugar de `Kernel/kernel.bin`
- Actualizado el comando `reload` para usar `kernel.elf`

```gdb
file Kernel/kernel.elf
```

### 5. Actualización de `DEBUG_GUIDE.md`
- Documentación actualizada con información sobre ambos archivos
- Aclaración sobre el uso de cada archivo

## Resultado

Ahora al compilar el kernel se generan:

1. **`kernel.elf`** (107KB): 
   - Formato: ELF 64-bit LSB executable
   - Contiene: Símbolos de debug completos
   - Uso: GDB para debugging

2. **`kernel.bin`** (30KB):
   - Formato: Binario plano
   - Contiene: Solo código ejecutable
   - Uso: Bootloader QEMU

## Cómo Usar

### Compilar con símbolos de debug:
```bash
make clean && make
```


### Debugging con GDB:
```bash
# Terminal 1: Iniciar QEMU con GDB server
./debug.sh

(La posta no la otra cosa)
cd Kernel
gdb kernel.elf
targer remote :1234
break (funcion)
continue (Avanza hatsa break)
next -> va a sig linea







# Terminal 2: Conectar GDB
gdb -x .gdbinit
```

GDB ahora puede:
- Leer todos los símbolos de funciones y variables
- Mostrar código fuente mientras debuggeas
- Establecer breakpoints por nombre de función
- Inspeccionar estructuras de datos correctamente
- Ver el stack trace completo con nombres de funciones

## Verificación

Para verificar que `kernel.elf` tiene símbolos de debug:
```bash
file Kernel/kernel.elf
# Output: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), 
#         statically linked, with debug_info, not stripped

readelf -S Kernel/kernel.elf | grep debug
# Debe mostrar secciones: .debug_info, .debug_abbrev, .debug_line, etc.
```

## Archivos Modificados

1. ✅ `Kernel/Makefile.inc` - Agregadas flags de debug
2. ✅ `Kernel/kernel_elf.ld` - Nuevo linker script para ELF
3. ✅ `Kernel/Makefile` - Genera kernel.elf y kernel.bin
4. ✅ `.gdbinit` - Carga kernel.elf
5. ✅ `DEBUG_GUIDE.md` - Actualizada documentación
