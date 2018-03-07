CFLAGS= -Wall -Werror -g
LDFLAGS=-pthread 
CC=gcc
OBJECTS=main.o
TARGET=5013HW4P2D

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -pthread -o $@

clean:
	rm ./$(TARGET) *.o
