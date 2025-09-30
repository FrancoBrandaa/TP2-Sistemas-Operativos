#!/bin/bash

# Script para ejecutar los tests del Memory Manager
# Funciona tanto en macOS como en Linux

echo "=== Memory Manager Tests ==="
echo "Compilando tests..."

# Limpiar compilaciones anteriores
make clean

# Compilar
if make; then
    echo "✅ Compilación exitosa"
    echo "Ejecutando tests..."
    echo ""
    
    # Ejecutar tests
    if ./MemoryManagerTest.out; then
        echo ""
        echo "✅ Todos los tests pasaron exitosamente!"
    else
        echo ""
        echo "❌ Algunos tests fallaron"
        exit 1
    fi
else
    echo "❌ Error en la compilación"
    exit 1
fi

echo ""
echo "=== Tests completados ==="