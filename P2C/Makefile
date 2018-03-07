CFLAGS= -Wall -Werror -g
LDFLAGS=-pthread 
CC=gcc
OBJECTS=main.o
TARGET=5013HW4P2C

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -pthread  -lrt -o $@

clean:
	rm ./$(TARGET) *.o
