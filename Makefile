CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = bin/ls

all: $(TARGET)

$(TARGET): src/ls.c
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(TARGET) src/ls.c

clean:
	rm -rf bin

.PHONY: all clean
