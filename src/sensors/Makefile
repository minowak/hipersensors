CC=gcc
INCLUDES=-I../../include/
CFLAGS=-Wall $(INCLUDES)
OUTDIR=../../lib
SRC=cpusensor.c memsensor.c
OBJ=$(SRC:.c=.o)

all: prepare sensors

prepare:
	@mkdir -p $(OUTDIR)

sensors:
	$(CC) $(CFLAGS) -shared -fPIC cpusensor.c -o $(OUTDIR)/cpusensor.so
	$(CC) $(CFLAGS) -shared -fPIC memsensor.c -o $(OUTDIR)/memsensor.so

clean:
	rm -rf *.o
