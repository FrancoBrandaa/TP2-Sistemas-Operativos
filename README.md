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
./compile.sh 
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

### Comandos Built‑in (síncronos)

| Comando             | Descripción                                                       | Uso                    |
| ------------------- | ----------------------------------------------------------------- | ---------------------- |
| `help`              | Lista todos los comandos disponibles                              | `help`                 |
| `man <cmd>`         | Muestra descripción y sintaxis de un comando                      | `man ps`               |
| `clear`             | Limpia la pantalla                                                | `clear`                |
| `echo <args>`       | Imprime los argumentos separados por espacios                     | `echo Hello World`     |
| `font [increase|decrease]` | Aumenta o disminuye el tamaño de la fuente                       | `font increase`        |
| `mem`               | Muestra el estado de la memoria dinámica                          | `mem`                  |
| `ps`                | Lista procesos con PID, estado, prioridad y foreground/background | `ps`                   |
| `kill <pid>`        | Termina el proceso indicado                                       | `kill 5`               |
| `block <pid>`       | Alterna el estado entre bloqueado y listo                         | `block 5`              |
| `nice <pid> <prio>` | Cambia la prioridad (1–5, 5 es la más alta)                       | `nice 7 5`             |

Notas:
- Los built‑ins se ejecutan en el contexto de la shell (no crean procesos).
- La shell protege procesos críticos: no permite matar/bloquear `init` (PID 1) ni a sí misma.

### Comandos que crean procesos (soportan pipes; algunos corren en background por defecto)

| Comando                         | Descripción                                                                 | Uso                           | Detalles |
| ------------------------------- | --------------------------------------------------------------------------- | ----------------------------- | -------- |
| `counter [max]`                 | Cuenta de 1 a `max` y termina                                               | `counter 100`                 | `max` por defecto: 10 |
| `loop [seconds]`                | Imprime un saludo con su PID cada N segundos                                | `loop 3`                      | Corre en background por defecto; `seconds` por defecto: 2 |
| `loop_ps`                       | Muestra el estado de la shell periódicamente                                | `loop_ps`                     | Background por defecto |
| `filter`                        | Lee de stdin y filtra vocales                                               | `filter`                      | Termina con Ctrl+D; ideal para `cat | filter` |
| `cat`                           | Repite exactamente lo que recibe por stdin                                   | `cat`                         | Termina con Ctrl+D |
| `wc [-l] [-w] [-c]`            | Cuenta líneas, palabras y caracteres de stdin                               | `wc -l -w -c`                 | Sin opciones muestra todo |
| `test_mm <max_memory_bytes>`    | Stress test del memory manager                                               | `test_mm 2048`                | Por defecto usa 1024 si se omite |
| `test_processes <max_proc>`     | Crea N procesos tipo `endless_loop` y los mata/bloquea aleatoriamente       | `test_processes 10`           | Por defecto 5 |
| `test_prio <max_value>`         | Ejecuta 3 procesos que cuentan hasta `max_value` en 3 fases de prioridad     | `test_prio 1000000`           | Por defecto 1000000000 |
| `test_synchro [iterations]`     | Test de sincronización CON semáforo                                         | `test_synchro 5000`           | Iteraciones por defecto: 1000 |
| `test_no_synchro [iterations]`  | Test de sincronización SIN semáforo                                         | `test_no_synchro 5000`        | Iteraciones por defecto: 1000 |
| `mvar <writers> <readers>`      | Problema lectores/escritores sobre una variable compartida                  | `mvar 2 3`                    | Writers 1–26, readers ≥ 1; corre indefinidamente |

---

## Tests

Los siguientes tests están integrados como comandos de la shell. Todos heredan stdin/stdout de la shell (compatibles con pipes) y aceptan valores por defecto si se omiten los parámetros.

### 1) test_mm — Memory Manager

```bash
test_mm <max_memory_bytes>
```

- Ejecuta alocaciones y frees aleatorios hasta `max_memory_bytes`, verifica con `memcheck` y repite indefinidamente.
- Parámetro: `max_memory_bytes` (por defecto 1024 si se omite desde la shell).

### 2) test_processes — Creación/gestión de procesos

```bash
test_processes <max_processes>
```

- Crea `max_processes` procesos `endless_loop` (background) y luego, en bucles, los va bloqueando/desbloqueando o matando aleatoriamente hasta terminar con todos.
- Parámetro: `max_processes` (por defecto 5).

### 3) test_prio — Prioridades del scheduler

```bash
test_prio <max_value>
```

Tres fases sobre 3 procesos que cuentan hasta `max_value`:
- Fase A: misma prioridad para los 3 (terminan aproximadamente juntos).
- Fase B: cambia prioridades con `nice()` y se observa el efecto.
- Fase C: aplica `nice()` mientras están bloqueados y luego se desbloquean (el cambio persiste).

Parámetro: `max_value` (por defecto 1000000000).

### 4) test_synchro / test_no_synchro — Sincronización

```bash
test_synchro [iterations]      # con semáforo
test_no_synchro [iterations]   # sin semáforo
```

- Crea pares de procesos que incrementan/decrementan sobre una variable global compartida.
- Con semáforo el valor final es determinístico; sin semáforo se observan race conditions.
- Parámetro: `iterations` por proceso (por defecto 1000 en ambos comandos).

---

## Pipes y Background

### Operador de Pipe `|`

Conecta la salida de un comando con la entrada de otro.

```bash
# Ejemplo básico
counter 50 | wc

**Sintaxis básica:** `comandoA | comandoB`

**Reglas / comportamiento real:**
- Solo se soportan por ahora pipes de un único nivel (A | B). No hay cadenas múltiples (A | B | C) ni built‑ins a la izquierda/derecha.
- Ambos comandos deben ser de tipo proceso (no built‑ins). Si alguno no lo es, la shell devuelve error.
- La shell crea un pipe (buffer circular de 4KB) y ajusta los FDs:
  - Proceso izquierdo: `stdout = pipe_write`, `stdin = FD_STDIN`.
  - Proceso derecho: `stdin = pipe_read`, `stdout = FD_STDOUT`.
- Ambos procesos se crean como foreground. El izquierdo escribe en el pipe (su stdout apunta al extremo de escritura) y el derecho lee del pipe (su stdin apunta al extremo de lectura). La shell espera primero al derecho y luego al izquierdo antes de devolver el prompt.
- EOF para el proceso derecho ocurre cuando el izquierdo cierra su extremo de escritura (fin del proceso) y no quedan writers.
- Ctrl+D en la shell NO envía EOF al pipe: solo cierra la entrada estándar de la shell. Para terminar el lector, debe finalizar el escritor.
- No se mezclan colores/formato: cada proceso escribe usando sus propios FDs.

**Errores típicos manejados:** comando inexistente, pipe mal formado (`|` sin comando a un lado), uso de built‑in en pipe.

**Ejemplos:**
```bash
counter 100 | wc          # Cuenta la cantidad de líneas/ palabras emitidas por counter
cat | filter              # Ingresa texto, filtra vocales
cat | wc -c               # Cuenta caracteres ingresados manualmente
```

### Operador de Background `&`

Ejecuta comandos en segundo plano sin bloquear la shell.

```bash
# Comando simple en background
counter 10 &

```

**Sintaxis:** `comando [&]`

**Reglas / comportamiento:**
- Agregar `&` al final fuerza background aunque el comando esté marcado como foreground por defecto.
- Comandos marcados internamente como background (por ejemplo `loop`, `loop_ps`) no necesitan `&`, pero aceptan que se les agregue (no cambia nada).
- Un proceso en background NO puede leer desde `stdin` de la shell (lecturas retornan EOF inmediatamente); esto evita que bloquee la interacción del usuario.
- La shell imprime el PID creado y continúa aceptando comandos. Al finalizar el proceso, su mensaje de cierre (si lo tiene) aparece intercalado.
- Si se combina con pipe, el `&` se ignora: ambos procesos del pipe se crean como foreground y la shell gestiona la espera secuencial.

**Ejemplos:**
```bash
loop &              # Inicia loop en background (equivalente a solo 'loop')
counter 50 &        # Cuenta sin bloquear la shell
loop_ps &           # Monitorea estado de la shell mientras interactúas
```

**Limitaciones actuales:**
- No hay job control avanzado (fg, bg, jobs).
- No se puede enviar entrada interactiva a procesos ya iniciados en background.
- `&` múltiple o en medio del comando no está soportado (solo al final).

---

## Atajos de Teclado

| Atajo       | Función                              |
| ----------- | ------------------------------------ |
| `Ctrl + C`  | Mata el proceso en foreground actual |
| `Ctrl + D`  | Envía EOF a la entrada estándar      |
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

# Contar solo líneas de la entrada
cat | wc -l

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
test_synchro 5000
# Determinístico: suma y resta protegidas por semáforo

# Sin semáforo (race condition)
test_no_synchro 5000
# Observa variaciones en el valor global por condiciones de carrera
```

### Ejemplo 5: Testing de Scheduler

```bash
# Ver funcionamiento del scheduler
test_prio 1000000000

# Mientras corre, en otra terminal ver procesos
ps

# Observar que procesos con más prioridad avanzan más rápido
```

### Ejemplo 6: mvar en ejecución

```bash
# Crear 2 writers y 3 readers (stream coloreado infinito hasta que mates los procesos)
mvar 2 3
# Para detenerlos: 'ps' para ver PIDs y luego 'kill <pid>'
```

---

## Características Implementadas

### ✅ Kernel

- [x] **Procesos**

  - Creación, destrucción y cambio de contexto
  - PIDs únicos y reciclables
  - Estados: READY, RUNNING, BLOCKED, KILLED
  - Proceso idle (PID 1)

- [x] **Scheduler**
  - Round-robin multinivel con colas separadas por prioridad (1–5)
  - Quantum uniforme (DEFAULT_QUANTUM) para todos los procesos
  - Selección: siempre se toma el primer proceso READY de la cola de mayor prioridad disponible
  - Aging: procesos que esperan acumulan `agingCounter`; al superar `AGING_THRESHOLD` reciben boost temporal (hasta `MAX_AGING_BOOST` niveles). Al terminar su turno se restaura `originalPriority`.
  - Init (PID 1) y Shell (PID 2) excluidos del aging/boost
  - Preemption por timer tick y yield voluntario (`yield()`)
  - Syscalls relevantes: `yield()`, `sleep()`, `getpid()`


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
  - Pipes 
  - Background execution
  - Comandos built-in y de proceso

- [x] **Biblioteca estándar**

  - stdio: `printf()`, `scanf()`, `fprintf()`, etc.
  - stdlib: `malloc()`, `free()`, `atoi()`, etc.
  - string: `strlen()`, `strcmp()`, `strcpy()`, etc.

- [x] **Tests completos**
  - test_mm, test_processes, test_prio, test_sync

## Decisiones de Diseño

| Tema | Elección | Ventajas | Trade-offs |
|------|----------|----------|------------|
| Recolección de procesos (wait) | `wait(pid,&status)` bloquea al padre hasta terminar el hijo; el kernel libera recursos al finalizar (no zombies persistentes). | Tabla de procesos limpia; simple de razonar; status disponible inmediatamente. | Sin señal asíncrona (no SIGCHLD); si el padre nunca llama a `wait` no ve el código de salida; no hay reapertura de procesos huérfanos. |
| Quantum y prioridades | Quantum uniforme + aging con boost temporal, restaurando prioridad original tras turno. | Implementación sencilla; evita inanición; comportamiento estable. | Menos control fino por prioridad (no ajusta tamaño de quantum); posible subutilización de CPU para procesos muy interactivos. |
| stdin en background | Lecturas en procesos background devuelven EOF inmediatamente. | Evita bloquear la shell; fácil de implementar. | Limita comandos interactivos en background; no hay redirección de entrada alternativa. |
| File Descriptors | Fijos (0 stdin, 1 stdout, 2 stderr parcial) + pipe de un salto (A | B). | Código simple; herencia de FDs clara; bajo overhead. | Sin pipelines múltiples, redirecciones (> >> <) ni stderr completo. |
| Memory Manager seleccionable | Elección Naive o Buddy por flag de compilación (no hot‑swap). | Comparación directa; reduce complejidad en runtime. | No se puede cambiar en ejecución; requiere recompilar para experimentar. |
| Semáforos | Conteo clásico sin detección de deadlock ni prioridades. | Ligero y rápido; API mínima (`open/close/wait/post/destroy`). | Posibles deadlocks si se abusa; sin herencia de prioridad ni timeouts. |

## Resumen de Verificación de Requerimientos

| Requerimiento | Sección / Ejemplo |
|--------------|-------------------|
| Compilación y ejecución | "Compilación y Ejecución" + scripts `compile.sh`, `run.sh` |
| Descripción comandos/tests y parámetros | Tablas en "Comandos de la Shell" y sección "Tests" |
| Pipes y background (`|`, `&`) | "Pipes y Background" (reglas, ejemplos) |
| Atajos de teclado (interrumpir / EOF) | "Atajos de Teclado" (Ctrl+C, Ctrl+D) |
| Ejemplos de funcionamiento | Sección "Ejemplos de Uso" (1–6) |
| Requerimientos faltantes / parciales | "Limitaciones Conocidas" + "Características No Implementadas" |
| Justificación de diseño | Nueva sección "Decisiones de Diseño" |

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
  - Round-robin multinivel con colas separadas por prioridad (1–5)
  - Quantum uniforme (DEFAULT_QUANTUM) para todos los procesos
  - Selección: siempre se toma el primer proceso READY de la cola de mayor prioridad disponible
  - Aging: procesos que esperan acumulan `agingCounter`; al superar `AGING_THRESHOLD` reciben boost temporal (hasta `MAX_AGING_BOOST` niveles). Al terminar su turno se restaura `originalPriority`.
  - Init (PID 1) y Shell (PID 2) excluidos del aging/boost
  - Preemption por timer tick y yield voluntario (`yield()`)
  - Syscalls relevantes: `yield()`, `sleep()`, `getpid()`



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

### Selección de Memory Manager (con compile escribir)

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
