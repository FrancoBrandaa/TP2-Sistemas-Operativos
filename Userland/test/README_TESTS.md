# Memory Manager Tests

Este directorio contiene los tests unitarios para el Memory Manager implementado con Buddy System.

## Estructura de Archivos

```
src/
├── AllTest.c              # Punto de entrada principal de tests
├── CuTest.c              # Framework de testing CuTest
├── MemoryManager.c       # Implementación del Memory Manager
├── MemoryManagerLib.c    # Funciones auxiliares del Memory Manager  
├── MemoryManagerTest.c   # Tests específicos del Memory Manager
└── include/
    ├── CuTest.h
    ├── MemoryManager.h
    ├── MemoryManagerLib.h
    └── MemoryManagerTest.h
```

## Tests Implementados

1. **testAllocMemory**: Prueba la allocación básica de memoria
2. **testTwoAllocations**: Verifica que dos allocaciones retornen direcciones diferentes y no se superpongan
3. **testWriteMemory**: Verifica que se puede escribir y leer de la memoria allocada
4. **testFreeMemory**: Prueba la liberación de memoria y que los contadores se actualicen correctamente
5. **testFreeAndRealloc**: Prueba liberar memoria y volver a allocar
6. **testMemoryAlignment**: Prueba allocaciones de diferentes tamaños y verificación de alineación

## Cómo Ejecutar

### Opción 1: Script automatizado
```bash
./run_tests.sh
```

### Opción 2: Manual
```bash
# Compilar
make clean
make

# Ejecutar
./MemoryManagerTest.out
```

## Configuración

Los tests están configurados con los siguientes parámetros:

- **MANAGED_MEMORY_SIZE**: 20480 bytes (20KB) - Tamaño total del heap para tests
- **ALLOCATION_SIZE**: 1024 bytes - Tamaño estándar de allocación para tests
- **TEST_MIN_ORDER**: 6 (64 bytes mínimo)
- **TEST_MAX_ORDER**: 15 (32KB máximo)

## Características del Memory Manager Testeado

- **Algoritmo**: Buddy System
- **Tamaño mínimo de bloque**: 64 bytes
- **Headers**: Cada bloque allocado tiene un header con información de tamaño, orden y magic number
- **Validación**: Magic numbers para detectar corrupción de memoria
- **Merge automático**: Los bloques freed se mergean automáticamente con sus buddies
- **Contadores**: Tracking de memoria usada y libre

## Salida Esperada

Si todos los tests pasan, verás:
```
......

OK (6 tests)
```

## Compatibilidad

Este código está diseñado para compilar y ejecutar en:
- ✅ Linux (container)
- ✅ macOS 
- ✅ Cualquier sistema con clang/gcc compatible con C99

## Troubleshooting

Si hay errores de compilación:
1. Verificar que clang esté instalado
2. Verificar que las flags de compilación sean compatibles con tu compilador
3. En sistemas antiguos, puede ser necesario cambiar `clang` por `gcc` en `Makefile.inc`