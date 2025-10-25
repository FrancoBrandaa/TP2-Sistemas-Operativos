include ../Makefile.inc

MODULE=shell.bin
SOURCES=$(wildcard [^_]*.c)

all: $(MODULE)

$(MODULE): $(SOURCES)
	# $(GCC) $(GCCFLAGS) -T shellModule.ld _loader.c -L../ $(SOURCES) -l:libc.a -l:libsys.a -o ../$(MODULE)
	
	$(GCC) $(GCCFLAGS) -I../tests -T shellModule.ld _loader.c -L../ $(SOURCES) ../tests/my_test_processes.c ../tests/test_prio.c ../tests/test_util.c -l:libc.a -l:libsys.a -o ../$(MODULE)


clean:
	rm -rf *.o

.PHONY: all clean print
