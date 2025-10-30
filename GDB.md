## Cómo Usar

### Compilar con símbolos de debug:
```bash
make clean && make
```


### Debugging con GDB:
```bash
# Terminal 1: Iniciar QEMU con GDB server
./debug.sh

# Terminal 2: para usar el gdb posta
cd Kernel
gdb kernel.elf
targer remote :1234
break function (pones donde quieras que frene, ej: break main)
continue (Avanza hatsa break)
next o n (va a sig linea)
p variable (imprime el valor de la variable global pero tenes que estar en ese .c)

dsp hay otros pero no me acuerdo lo que me dijo euge