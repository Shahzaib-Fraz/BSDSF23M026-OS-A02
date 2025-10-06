CC = gcc
CFLAGS = -Wall -Wextra

SRC_V1 = src/ls-v1.0.0.c
SRC_V2 = src/ls-v1.1.0.c

BIN_DIR = bin

all: $(BIN_DIR)/ls-v1.0.0 $(BIN_DIR)/ls-v1.1.0

$(BIN_DIR)/ls-v1.0.0: $(SRC_V1)
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/ls-v1.1.0: $(SRC_V2)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BIN_DIR)/ls-v1.0.0 $(BIN_DIR)/ls-v1.1.0
