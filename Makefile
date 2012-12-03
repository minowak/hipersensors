CC=gcc
INCLUDES=-I./include
CFLAGS=-Wall -g $(INCLUDES)
VPATH=./src:./include:./lib
NAME=sensor_runner
SRC=sensor_runner.c
OBJ=$(SRC:.c=.o)
HEADERS=sensors.h
LIBSRC=src/cpusensor.c
LIBOBJ=$(LIBSRC:.c=.o)

LDFLAGS=-lpthread

all: libs runner

libs: $(LIBSRC) $(LIBOBJ)
	$(CC) $(CFLAGS) -fPIC -c $^ $(LDFLAGS) 
	$(CC) $(CFLAGS) -shared -fPIC -o ./lib/cpusensor.so $(LIBOBJ) $(LDFLAGS)

runner: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(NAME) -ldl $(LDFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $^ -ldl $(LDFLAGS)

clean:
	rm -rf *.o
	rm -rf ./src/*.o
	rm -rf ./src/*~
	rm -rf ./include/*~
