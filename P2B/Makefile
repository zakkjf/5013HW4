CFLAGS= -Wall -Werror -g
LDFLAGS=-pthread 
CC=gcc
OBJECTS=main.o
TARGET=5013HW4P2C

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -pthread  -lnsl -lsocket -lresolv -L /usr/lib/happycoders -o $@

clean:
	rm ./$(TARGET) *.o
