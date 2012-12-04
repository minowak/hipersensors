CC=gcc
INCLUDES=-I./include
CFLAGS=-Wall -g $(INCLUDES)
VPATH=./src:./include:./lib
NAME=sensor_runner
SRC=sensor_runner.c
OBJ=$(SRC:.c=.o)
HEADERS=sensors.h

LDFLAGS=-lpthread

all: libs runner

libs:
	@make -C ./src/sensors

runner: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(NAME) -ldl $(LDFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $^ -ldl $(LDFLAGS)

clean:
	@make clean -C ./src/sensors
	rm -rf *.o
	rm -rf ./src/*.o
	rm -rf ./src/*~
	rm -rf ./include/*~
