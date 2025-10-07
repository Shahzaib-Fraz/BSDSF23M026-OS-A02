CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src/ls.c
BIN = bin/ls

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf bin
