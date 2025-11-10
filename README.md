# TP2 - Sistema Operativo x64

Sistema operativo básico desarrollado en C y Assembly para arquitectura x86-64. Incluye manejo de procesos, scheduling, semáforos, pipes y dos implementadores de memoria dinámica (Naive y Buddy System).

## Integrantes

| Nombre               | Legajo | Mail                      |
| -------------------- | ------ | ------------------------- |
| Franco Branda        | 65506  | fbranda@itba.edu.ar       |
| Mateo Arias          | 65613  | matarias@itba.edu.ar      |
| Manuel García Puente | 65505  | mgarciapuente@itba.edu.ar |

---

## Tabla de Contenidos TODO: AL FINAL corregir esto

- [Compilación y Ejecución](#compilación-y-ejecución)
- [Comandos de la Shell](#comandos-de-la-shell)
- [Tests](#tests)
- [Pipes y Background](#pipes-y-background)
- [Atajos de Teclado](#atajos-de-teclado)
- [Ejemplos de Uso](#ejemplos-de-uso)
- [Decisiones de Diseño y Limitaciones](#decisiones-de-diseño-y-limitaciones)
- [Citas de código y uso de la IA](#citas-de-código-y-uso-de-ia)

---

## Compilación y Ejecución

### Prerequisitos

- Docker instalado y en ejecución
- QEMU (para ejecutar el sistema operativo)

### Compilación

El proyecto soporta dos implementaciones de memory manager:

```bash
# Compilar con Naive Memory Manager 
./compile.sh naive

# Compilar con Buddy System Memory Manager
./compile.sh buddy

# Compilar con el memory manager por defecto (Naive)
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

## Comandos de la Shell:

### Comandos Built‑in (no crean procesos)

| Comando             | Descripción                                                       | Uso                    |
| ------------------- | ----------------------------------------------------------------- | ---------------------- |
| `help`              | Lista todos los comandos disponibles                              | `help`                 |
| `man <cmd>`         | Muestra nombre, descripción, sintaxis y tipo de un comando                      | `man ps`               |
| `clear`             | Limpia la pantalla                                                | `clear`                |
| `echo <args>`       | Imprime los argumentos separados por espacios                     | `echo Hello World`     |
| `font [increase/decrease]` | Aumenta o disminuye el tamaño de la fuente                       | `font increase`        |
| `mem`               | Muestra el estado de la memoria dinámica                          | `mem`                  |
| `ps`                | Lista procesos con PID, estado, prioridad y foreground/background | `ps`                   |
| `kill <pid>/kill all`        | Termina el proceso indicado o todos los posibles                                       | `kill 5`               |
| `block <pid>`       | Alterna el estado entre block y ready                         | `block 5`              |
| `nice <pid> <prio>` | Cambia la prioridad (1–5, 5 es la más alta) de un proceso indicado                    | `nice 7 5`             |

Notas:
- La shell protege procesos críticos: no permite matar/bloquear `init` (PID 1) ni a sí misma.

### Comandos que crean procesos
- Para que corran en background hay que agregarle `&` al final (se detalla más adelante).

| Comando                         | Descripción                                                                 | Uso                           | Detalles |
| ------------------------------- | --------------------------------------------------------------------------- | ----------------------------- | -------- |
| `counter [max]`                 | Cuenta de 1 a `max` y termina                                               | `counter 100`                 | `max` por defecto: 10 |
| `loop [seconds]`                | Imprime un saludo con su PID cada N segundos                                | `loop 3`                      |  `seconds` por defecto: 2 |
| `loop_ps`                       | Muestra el estado de la shell periódicamente                                | `loop_ps`                     | útil en background para verificar que la shell no hace espera activa. |
| `filter`                        | Lee de stdin y filtra vocales                                               | `filter`                      | Termina con Ctrl+D
| `cat`                           | Repite exactamente lo que recibe por stdin                                   | `cat`                         | Termina con Ctrl+D |
| `wc [-l] [-w] [-c]`            | Cuenta líneas, palabras y caracteres de stdin                               | `wc -l -w -c`                 | Por defecto muestra todo |
| `test_mm <max_memory_bytes>`    | Stress test del memory manager                                               | `test_mm 2048`                | Por defecto usa 1024 |
| `test_processes <max_proc>`     | Crea N procesos tipo `endless_loop` y los mata/bloquea aleatoriamente       | `test_processes 10`           | Por defecto 5 |
| `test_prio <max_value>`         | Ejecuta 3 procesos que cuentan hasta `max_value` en 3 fases de prioridad     | `test_prio 1000000`           | Por defecto 1000000 |
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

Parámetro: `max_value` (por defecto 1000000).

### 4) test_synchro / test_no_synchro — Sincronización

```bash
test_synchro [iterations]      # con semáforo
test_no_synchro [iterations]   # sin semáforo
```

- Crea pares de procesos que incrementan/decrementan sobre una variable global compartida.
- Con semáforo el valor final es 0. Sin semáforo se observan race conditions, el resultado es impredecible.
- Parámetro: `iterations` por proceso (por defecto 1000 en ambos comandos).

---

## Pipes y Background

### Operador de Background `&`

Ejecuta comandos en segundo plano sin bloquear la shell.

**Sintaxis:** `comando [&]`

**Reglas / comportamiento:**
- Agregar `&` al final fuerza background aunque el comando esté marcado como foreground por defecto.
- Hay procesos que no tiene sentido o no es posible correrlos en background (cat, filter, wc y mvar). En estos casos se le indica al usuario por pantalla y se corre el proceso indicado en foreground.
- La shell imprime el PID creado y continúa aceptando comandos. Al finalizar el proceso, su mensaje de cierre (si lo tiene) aparece intercalado.


```bash
# Ejemplo básico
loop &              # Inicia loop en background 
```

### Operador de Pipe `|`

Conecta la salida de un comando con la entrada de otro.

**Sintaxis básica:** `comandoA | comandoB`

**Reglas / comportamiento real :**
- Solo se soportan pipes de un único nivel (A | B). No hay cadenas múltiples (A | B | C) ni built‑ins a la izquierda/derecha.
- Ambos comandos deben ser procesos (no built‑ins). Si alguno no lo es, la shell devuelve error.
- La shell crea un pipe y ajusta los FDs:
  - Proceso izquierdo (productor): `stdout = pipe_write`, `stdin = FD_STDIN`.
  - Proceso derecho (consumidor): `stdin = pipe_read`, `stdout = FD_STDOUT`.
- Comportamiento respecto a foreground/background:
  - Semántica de `&` en pipelines: si la pipeline termina con `&`, ese operador se interpreta como petición de ejecutar el proceso izquierdo (productor) en background. El consumidor (derecho) sigue la regla general: siempre se ejecuta en background.
  - Validación: si el usuario pide `&` y el comando izquierdo tiene `allow_background = 1` → izquierdo en background; si `allow_background = 0` → error y la pipeline no se ejecuta.
  - Espera de la shell: la shell solo espera por el proceso izquierdo cuando éste se ejecuta en foreground (ausencia de `&` o `&` no permitido). El consumidor nunca bloquea la shell.
- Errores típicos manejados: comando inexistente, pipe mal formado (`|` sin comando a un lado), uso de built‑in en pipe, intento inválido de background en izquierdo.

**Ejemplos prácticos:** Ver "Ejemplo 2: Uso de Pipes" más abajo.

---

## Atajos de Teclado

| Atajo       | Función                              |
| ----------- | ------------------------------------ |
| `Ctrl + C`  | Mata el proceso en foreground actual |
| `Ctrl + D`  | Envía EOF a la entrada estándar      |
| `Enter`     | Ejecuta comando                      |
| `Backspace` | Borra carácter anterior              |
---

## Ejemplos de Uso:

### Ejemplo 1: Monitoring de Procesos en Background

```bash
# Iniciar múltiples procesos en background
loop & --> PID = 3
counter 100 & --> PID = 4
loop_ps & --> PID = 5

# Ver estado con ps
ps

# Matar un proceso específico
kill 3

# Bloquear/desbloquear un proceso
block 4
block 4  # Desbloquear

# Cambiar prioridad
nice 5 4
```

### Ejemplo 2: Uso de Pipes

- Reglas clave:
  - `&` al final de la pipeline afecta al proceso izquierdo (productor).
  - El proceso derecho (consumidor) se ejecuta siempre en background.
  - La shell espera únicamente si el productor queda en foreground.
  - Si se solicita `&` y el productor tiene `allow_background = 0` → error y no se ejecuta la pipeline.

```bash
# 1) Lectura manual y conteo
cat | wc
# - Escribe texto, termina con Ctrl+D en la entrada del shell.
# - cat (productor) por defecto en foreground, wc (consumidor) en background.
# - Shell espera a cat; wc procesa lo recibido.

# 2) Filtrado de texto
cat | filter
# - Igual que arriba; filter elimina vocales y termina cuando cat cierra su extremo de escritura.

# 3) Generación y conteo (no interactivo)
counter 100 | wc -l
# - counter genera datos (productor); wc cuenta líneas.
# - Por defecto counter = foreground, wc = background; shell espera a counter.

# 4) Pipeline con & al final 
loop | filter &
# - loop.allow_background = 1:
#   loop → background, filter → background; shell devuelve prompt inmediatamente.

# 5) Intento inválido de background en productor interactivo
cat | filter &
# - cat.allow_background = 0 → Error: "Command 'cat' cannot run in background (requires user input)"

# 6) Pipeline continua en foreground (sin &)
loop | filter
# - loop en foreground (bloquea la shell), filter en background.
# - Para detener loop: usar 'ps' para ver PID y 'kill <pid>'.
```

---

## Decisiones de Diseño y Limitaciones

### Recolección de procesos (wait)
Como no incluimos jerarquía en los procesos, decidimos pasarle como parámetro a nuestra función `wait` el `PID` del proceso que debe esperar y un puntero `status` donde devuelve la salida de este.

Ventajas: 
- Mantiene la tabla de procesos limpia sin necesidad de tener una jerarquia implementada, ya que esta hubiera generado otros nuevos problemas a solucionar. 

- El código de salida (`status`) está disponible inmediatamente después de hacer wait.

Limitaciones: 
- No hay un sistema de seguridad donde solo espera a procesos creados por el proceso actual, sino que cualquier proceso puede esperar a cualquier otro sin tener en cuenta el contexto de cada uno. 
- Hay riesgos de seguridad asociados a ataques de referencia indirecta sabiendo que los PID son secuencias de enteros.

### Jerarquía de procesos y terminación del padre

 Por lo mismo que antes, un proceso que crea otro no queda registrado como su padre y, por lo tanto, al matar al proceso creador los procesos que haya lanzado continúan ejecutándose sin verse afectados. No hay propagación de señales ni “muerte en cascada”.

Ventajas:
- Menor complejidad en la tabla de procesos y en la función `wait`.
- Comportamiento explícito y predecible para el usuario.
- Permite que el comando mvar funcione correctamente.

Limitaciones: 
- Al matar un proceso que crea y mata procesos constantemente (por ejemplo test_processes) quedan procesos corriendo que no sirven de mucho. Para esto existe el comando `kill all`. 
- No existe coordinación automática de cierre entre procesos relacionados. 
- El usuario debe gestionar manualmente la terminación de cada proceso.  

### Quantum y prioridades
 Se implemento un quantum uniforme con scheduler round‑robin multinivel con colas separadas por prioridad (1–5) donde siempre se toma el primer proceso READY de la cola de mayor prioridad disponible.
 
 Cuenta con un mecanismo de aging que aplica un "boost temporal" a procesos que esperan demasiado tiempo a ser seleccionados para correr, restaurando la prioridad original tras su turno.  
 
 Se decidió que Init (PID 1) y Shell (PID 2) sean excluidos del aging/boost. Esto para que la shell siempre tenga prioridad frente a otros procesos secundarios y para que el Init nunca ser ejecutado si hay otro proceso READY, pues este solo corre si no hay otros procesos disponibles.
 
 Ventajas: 
 - Implementación sencilla y estable
 - Resuelve el problema de la inanición 
 - Tiene un comportamiento predecible entre niveles de prioridad.

Limitaciones: 
- Aunque el aging funciona y cumple con su deber, es una versión muy simple de este y no contempla casos muy específicos.  

## Citas de Código y Uso de IA
### Código Base
 El proyecto está basado en el template "x64 Bare Bones" proporcionado por la cátedra de Sistemas Operativos del ITBA.
 
 #### Código de Terceros. 

 **Font 8x8**: Fuente de bitmap básica
 - Archivo: `Kernel/include/font_basic_8x8.h` 
 - Fuente: Dominio público 
    
  #### Uso de IA.
  
Se utilizó GitHub Copilot como asistente durante el desarrollo para:  
- Autocompletado de código repetitivo 
- Sugerencias de nombres de variables y funciones 
- Generación de comentarios de documentación
- Aporte de ideas para problemas/conflictos surgidos durante el desarrollo (Brainstorming).
  
**Nota importante**: Todo el código generado por IA fue revisado, modificado y testeado exhaustivamente. 

### Referencias Consultadas
- OSDev Wiki: https://wiki.osdev.org/
- Documentación de GDB
- Documentación de QEMU
