# TP2 - Sistema Operativo x64

Sistema operativo básico desarrollado en C y Assembly para arquitectura x86-64. Incluye manejo de procesos, scheduling, semáforos, pipes y dos implementadores de memoria dinámica (Naive y Buddy System).

## Integrantes

| Nombre               | Padrón | Email                     |
| -------------------- | ------ | ------------------------- |
| Franco Branda        | 65506  | fbranda@itba.edu.ar       |
| Mateo Arias          | 64018  | matarias@itba.edu.ar      |
| Manuel García Puente | 65505  | mgarciapuente@itba.edu.ar |

---

## Tabla de Contenidos

- [Compilación y Ejecución](#compilación-y-ejecución)
- [Comandos de la Shell](#comandos-de-la-shell)
- [Tests](#tests)
- [Pipes y Background](#pipes-y-background)
- [Atajos de Teclado](#atajos-de-teclado)
- [Ejemplos de Uso](#ejemplos-de-uso)
- [Características Implementadas](#características-implementadas)
- [Limitaciones Conocidas](#limitaciones-conocidas)
- [Notas de Desarrollo](#notas-de-desarrollo)

---

## Compilación y Ejecución

### Prerequisitos

- Docker instalado y en ejecución
- QEMU (para ejecutar el sistema operativo)

### Compilación

El proyecto soporta dos implementaciones de memory manager:

```bash
# Compilar con Naive Memory Manager (por defecto)
./compile.sh naive

# Compilar con Buddy System Memory Manager
./compile.sh buddy

# Compilar con el memory manager por defecto
./compile.sh all
```

El script `compile.sh`:

1. Crea/inicia el contenedor Docker `tp2-so` si no existe
2. Compila el proyecto completo (bootloader, kernel, userland)
3. Corrige los permisos de los archivos generados

### Ejecución

```bash
# Ejecutar el sistema operativo en QEMU
./run.sh
```

**Nota:** El script `run.sh` está configurado para macOS (usa `coreaudio`). Para Linux, modificar la línea de audio en el script.

### Comandos Make Disponibles

```bash
make all          # Compila todo (bootloader + kernel + userland + image)
make naive        # Compila todo con Naive Memory Manager
make buddy        # Compila todo con Buddy Memory Manager
make clean        # Limpia archivos compilados
make status       # Muestra el memory manager actualmente configurado
```

---

## Comandos de la Shell

### Comandos Built-in (síncronos)

| Comando             | Descripción                                   | Uso                |
| ------------------- | --------------------------------------------- | ------------------ |
| `help`              | Muestra lista de comandos disponibles         | `help`             |
| `man <cmd>`         | Muestra ayuda detallada de un comando         | `man ps`           |
| `clear`             | Limpia la pantalla                            | `clear`            |
| `echo <args>`       | Imprime argumentos en pantalla                | `echo Hello World` |
| `font <size>`       | Cambia tamaño de fuente (1-3)                 | `font 2`           |
| `regs`              | Muestra registros guardados (requiere Ctrl+R) | `regs`             |
| `mem`               | Muestra estado de la memoria dinámica         | `mem`              |
| `ps`                | Lista todos los procesos activos              | `ps`               |
| `kill <pid>`        | Termina un proceso por PID                    | `kill 5`           |
| `block <pid>`       | Bloquea/desbloquea un proceso                 | `block 5`          |
| `nice <pid> <prio>` | Cambia prioridad de un proceso (1-10)         | `nice 5 8`         |

### Comandos de Proceso (pueden ejecutarse en background)

| Comando          | Descripción                        | Uso                  |
| ---------------- | ---------------------------------- | -------------------- |
| `counter [max]`  | Cuenta de 1 a max                  | `counter 100`        |
| `loop [seconds]` | Imprime PID cada N segundos        | `loop 3`             |
| `loop_ps`        | Ejecuta ps cada segundo            | `loop_ps &`          |
| `filter`         | Filtra vocales de entrada          | `cat file \| filter` |
| `cat`            | Imprime entrada estándar           | `cat`                |
| `wc`             | Cuenta líneas de entrada           | `cat file \| wc`     |
| `mvar`           | Variable compartida entre procesos | `mvar`               |

---

## Tests

El sistema incluye 4 tests exhaustivos:

### 1. test_mm - Test de Memory Manager

Prueba el allocator dinámico con múltiples procesos.

```bash
test_mm [max_memory]
```

**Parámetros:**

- `max_memory`: Cantidad máxima de memoria a alocar en bytes (default: 1024)

**Ejemplo:**

```bash
test_mm 2048
```

**Qué prueba:**

- Allocaciones y liberaciones concurrentes
- Fragmentación de memoria
- Condiciones de memoria agotada
- Corrección de punteros retornados

---

### 2. test_processes - Test de Creación de Procesos

Crea y elimina procesos recursivamente.

```bash
test_processes [max_processes]
```

**Parámetros:**

- `max_processes`: Número máximo de procesos a crear (default: 5)

**Ejemplo:**

```bash
test_processes 10
```

**Qué prueba:**

- Creación masiva de procesos
- Destrucción correcta de procesos
- Reciclaje de PIDs
- Límites del sistema

---

### 3. test_prio - Test de Prioridades

Verifica el scheduler con diferentes prioridades.

```bash
test_prio [iterations]
```

**Parámetros:**

- `iterations`: Número de iteraciones por proceso (default: 1000000000)

**Ejemplo:**

```bash
test_prio 500000000
```

**Qué prueba:**

- Scheduling round-robin
- Respeto de prioridades (más prioridad = más CPU time)
- Fairness entre procesos de igual prioridad

---

### 4. test_sync - Test de Sincronización

Prueba semáforos con múltiples procesos.

```bash
test_sync [iterations] [use_sem]
```

**Parámetros:**

- `iterations`: Número de incrementos por proceso (default: 1000)
- `use_sem`: 1 = con semáforos, 0 = sin semáforos (default: 1)

**Ejemplo:**

```bash
# Con semáforo (debe dar resultado correcto)
test_sync 10000 1

# Sin semáforo (demuestra race condition)
test_sync 10000 0
```

**Qué prueba:**

- Exclusión mutua con semáforos
- Race conditions (cuando use_sem=0)
- Operaciones atómicas

---

## Pipes y Background

### Operador de Pipe `|`

Conecta la salida de un comando con la entrada de otro.

```bash
# Ejemplo básico
counter 50 | wc

**Comportamiento:**

- Los procesos se ejecutan concurrentemente
- Salida del primero → entrada del segundo

### Operador de Background `&`

Ejecuta comandos en segundo plano sin bloquear la shell.

```bash
# Comando simple en background
counter 10 &

```

**Comportamiento:**

- La shell retorna inmediatamente
- El proceso sigue ejecutándose
- Mensajes de finalización se muestran asincrónicamente

---

## Atajos de Teclado

| Atajo       | Función                              |
| ----------- | ------------------------------------ |
| `Ctrl + C`  | Mata el proceso en foreground actual |
| `Ctrl + D`  | Envía EOF a la entrada estándar      |
| `Ctrl + R`  | Guarda registros para ver con `regs` |
| `Enter`     | Ejecuta comando                      |
| `Backspace` | Borra carácter anterior              |

**Nota sobre Ctrl+D:**

- Cierra la entrada estándar del proceso actual
- Útil para terminar comandos como `cat` que esperan input
- Si se usa en la shell directamente, puede cerrarla

---

## Ejemplos de Uso

### Ejemplo 1: Monitoring de Procesos en Background

```bash
# Iniciar múltiples procesos en background
loop
counter 100 &
loop_ps 

# Ver estado con ps
ps

# Matar un proceso específico
kill 5

# Bloquear/desbloquear un proceso
block 6
block 6  # Desbloquear

# Cambiar prioridad
nice 7 5  # Máxima prioridad
```

### Ejemplo 2: Uso de Pipes

```bash

# Filtrar vocales de la salida de counter
counter 100 | filter

```

### Ejemplo 3: Testing del Memory Manager

```bash
# Ver estado inicial de memoria
mem

# Ejecutar test con poca memoria
test_mm 512

# Ver cómo cambió el estado
mem

# Test más exigente
test_mm 4096

# Comparar entre Naive y Buddy
# (requiere recompilar con ./compile.sh buddy)
mem
```

### Ejemplo 4: Testing de Sincronización

```bash
# Con semáforo (correcto)
test_sync 5000 1
# Debería imprimir: "Final value: 50000" (10 procesos × 5000)

# Sin semáforo (race condition)
test_sync 5000 0
# Imprimirá un valor incorrecto debido a race conditions
```

### Ejemplo 5: Testing de Scheduler

```bash
# Ver funcionamiento del scheduler
test_prio 1000000000

# Mientras corre, en otra terminal ver procesos
ps

# Observar que procesos con más prioridad avanzan más rápido
```

---

## Características Implementadas

### ✅ Kernel

- [x] **Procesos**

  - Creación, destrucción y cambio de contexto
  - PIDs únicos y reciclables
  - Estados: READY, RUNNING, BLOCKED, KILLED
  - Proceso idle (PID 0)
  - Heap independiente por proceso

- [x] **Scheduler**

  - Round-robin con prioridades (1-10)
  - Mayor prioridad = más quantum time
  - Preemption por timer tick
  - Syscalls: `yield()`, `sleep()`, `getpid()`, `ps_info()`

- [x] **Semáforos**

  - Creación con valor inicial
  - Operaciones `wait()` y `post()`
  - Bloqueo y desbloqueo de procesos
  - Lista de procesos esperando

- [x] **Pipes**

  - Buffers circulares de 4KB
  - Bloqueo automático en lectura/escritura
  - EOF cuando todos los writers cierran
  - Conexión automática de stdin/stdout

- [x] **File Descriptors**

  - FD 0: stdin
  - FD 1: stdout
  - FD 2: stderr (no implementado totalmente)
  - Pipes como FDs

- [x] **Memory Managers**
  - **Naive**: Bitmap simple y rápido
  - **Buddy System**: Eficiente en fragmentación
  - Syscalls: `malloc()`, `free()`, `mem_info()`

### ✅ Userland

- [x] **Shell**

  - Parsing de comandos y argumentos
  - Pipes con múltiples comandos
  - Background execution
  - Comandos built-in y de proceso

- [x] **Biblioteca estándar**

  - stdio: `printf()`, `scanf()`, `fprintf()`, etc.
  - stdlib: `malloc()`, `free()`, `atoi()`, etc.
  - string: `strlen()`, `strcmp()`, `strcpy()`, etc.

- [x] **Tests completos**
  - test_mm, test_processes, test_prio, test_sync

---

## Limitaciones Conocidas

### Limitaciones del Sistema

1. **Memoria fija**: El kernel reserva memoria estática al inicio, no se puede expandir dinámicamente.

2. **Máximo de procesos**: Limitado por `MAX_PROCESSES` (típicamente 64).

3. **Pipes**:

   - Tamaño fijo de 4KB por pipe
   - No soporta pipes nombrados (named pipes)
   - Un proceso no puede tener más de 2 pipes abiertos

4. **File Descriptors**:

   - Solo 3 FDs por proceso (stdin, stdout, stderr)
   - stderr no está completamente implementado

5. **Semáforos**:

   - Máximo de semáforos limitado
   - No hay detección de deadlock

6. **Scheduler**:
   - No hay diferenciación entre procesos I/O-bound y CPU-bound
   - No implementa aging para prevenir starvation

### Características No Implementadas

- [ ] Sistema de archivos persistente
- [ ] Networking
- [ ] Múltiples terminales
- [ ] Variables de entorno
- [ ] Signals (excepto Ctrl+C)
- [ ] Job control completo
- [ ] Dynamic linking
- [ ] Multicore scheduling real (solo simula)

---

## Notas de Desarrollo

### Estructura del Proyecto

```
TP2-Sistemas-Operativos/
├── Bootloader/         # Pure64 bootloader
├── Kernel/             # Código del kernel
│   ├── memory/        # Naive y Buddy managers
│   ├── process/       # Procesos y scheduler
│   ├── drivers/       # Keyboard, video, sound, time
│   └── idt/           # Interrupts y syscalls
├── Userland/          # Código de usuario
│   ├── Shell/         # Shell interactiva
│   ├── tests/         # Tests del sistema
│   └── lib*/          # Bibliotecas estándar
└── Image/             # Imagen de disco generada
```

### Selección de Memory Manager

Al compilar con `make naive` o `make buddy`, se define una macro de compilación que selecciona el manager en `Kernel/memory/`:

- `MEMORY_MANAGER_NAIVE`: Usa `naiveManager.c`
- `MEMORY_MANAGER_BUDDY`: Usa `buddyManager.c`

### Debugging

```bash
# Compilar con símbolos de debug
./debug.sh

# Luego en otra terminal
gdb
(gdb) target remote :1234
(gdb) symbol-file Kernel/kernel.bin
```

Ver `GDB.md` para más información.

---

## Citas de Código y Uso de IA

### Código Base

El proyecto está basado en el template "x64 Bare Bones" proporcionado por la cátedra de Sistemas Operativos del ITBA.

### Código de Terceros

1. **Pure64 Bootloader**: Bootloader de 64 bits open source

   - Licencia: BSD
   - Fuente: `Bootloader/Pure64/`

2. **Font 8x8**: Fuente de bitmap básica
   - Archivo: `Kernel/include/font_basic_8x8.h`
   - Fuente: Dominio público

### Uso de IA

Se utilizó GitHub Copilot como asistente durante el desarrollo para:

- Autocompletado de código repetitivo
- Sugerencias de nombres de variables y funciones
- Generación de comentarios de documentación
- Ayuda con sintaxis de Assembly x86-64

**Nota importante**: Todo el código generado por IA fue revisado, modificado y testeado exhaustivamente. La lógica central del sistema operativo (scheduler, memory managers, semaphores, pipes) fue diseñada e implementada completamente por el equipo.

### Referencias Consultadas

- OSDev Wiki: https://wiki.osdev.org/
- Intel 64 and IA-32 Architectures Software Developer's Manual
- "Operating Systems: Three Easy Pieces" - Remzi H. Arpaci-Dusseau
- Documentación de QEMU y GDB

---

## Licencia

Ver archivo `License.txt` para más información.

---

**Trabajo Práctico 2 - Sistemas Operativos**  
Instituto Tecnológico de Buenos Aires (ITBA)  
2025
