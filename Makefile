# Makefile for ls v1.2.0 - Column Display
# Compiles src/ls-v1.1.2.c into ls binary

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
SRC = src/ls-v1.1.2.c
BIN = ls

.PHONY: all clean release

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

clean:
	rm -f $(BIN)
	rm -rf release

release: all
	@echo "Packaging release binary for v1.2.0..."
	@mkdir -p release
	cp $(BIN) release/ls-v1.2.0
