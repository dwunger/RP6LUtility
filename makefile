CC=gcc
CFLAGS=-I./src

# Directories
BUILD_DIR=./build
TESTS_DIR=./tests

# Source files
SRC_DIR=./src
SRCS=$(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Executable
EXEC=$(BUILD_DIR)/unpack

# Default target
all: $(BUILD_DIR) $(EXEC)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $@

# Rule for creating object files from C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Link objects into single executable
$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Test target
test: $(BUILD_DIR) $(BUILD_DIR)/test

$(BUILD_DIR)/test: $(TESTS_DIR)/test.c $(BUILD_DIR)/b010_editor.o | $(BUILD_DIR)
	$(CC) -o $@ $^ $(CFLAGS)
	$@

.PHONY: all clean test
