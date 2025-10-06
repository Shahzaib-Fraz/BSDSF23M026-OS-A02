# ----------------------------
# Makefile for lsv1.0.0 Project
# ----------------------------

# Compiler
CC = gcc

# Directories
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

# Target executable name
TARGET = $(BIN_DIR)/ls

# Source and Object files
SRC = $(SRC_DIR)/ls-v1.0.0.c
OBJ = $(OBJ_DIR)/ls-v1.0.0.o

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11

# Default rule
all: $(TARGET)

# Build rule: compile and link
$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)
	@echo "✅ Build successful! Executable created at $(TARGET)"

# Compile .c to .o
$(OBJ): $(SRC)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)
	@echo "Compiled: $(SRC) -> $(OBJ)"

# Run the program
run: all
	@echo "Running program..."
	@$(TARGET)

# Clean compiled files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "🧹 Cleaned all build files."

# Phony targets (not real files)
.PHONY: all clean run
