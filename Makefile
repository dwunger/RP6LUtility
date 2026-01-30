# Compiler settings
CC := gcc
CXX := g++
CFLAGS := -g -Wall -Wextra -std=c11 -I./src -I./include -I./googletest/googletest/include
CXXFLAGS := -Wall -Wextra -std=c++11 -I./src -I./include -I./googletest/googletest/include -DGTEST_OS_WINDOWS -D_UNICODE -DUNICODE
LDFLAGS += -lcomdlg32 -lole32 -lshell32
CXXFLAGS += -DEBUG_PRINT

# Directories
SRC_DIR := ./src
TEST_DIR := ./test/unit
BUILD_DIR := ./build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

# Source files (excluding extract.c and inject.c - they have their own main functions)
SRCS := $(filter-out $(SRC_DIR)/extract.c $(SRC_DIR)/inject.c,$(wildcard $(SRC_DIR)/*.c))
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Test files
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))
TEST_TARGET := $(BIN_DIR)/runTests

# Google Test library
GTEST_VERSION := release-1.11.0
GTEST_URL := https://github.com/google/googletest/archive/$(GTEST_VERSION).tar.gz
GTEST_DIR := ./googletest
GTEST_SRCS := $(GTEST_DIR)/googletest/src/gtest-all.cc
GTEST_OBJS := $(OBJ_DIR)/gtest-all.o

# Targets
.PHONY: debug run all clean test help extract inject run-inject
debug:
	@echo "=== DEBUG EXTRACT ==="
	powershell -Command "gdb -tui ./build/bin/extract.exe"
run:
	@echo "=== RUNNING EXTRACT ==="
	powershell -Command "./build/bin/extract.exe"

run-inject:
	@echo "=== RUNNING INJECT ==="
	powershell -Command "./build/bin/inject.exe"

all: extract inject

extract: $(BIN_DIR)/extract

inject: $(BIN_DIR)/inject

$(BIN_DIR)/extract: $(OBJS) $(OBJ_DIR)/extract.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/inject: $(OBJS) $(OBJ_DIR)/inject.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/extract.o: $(SRC_DIR)/extract.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/inject.o: $(SRC_DIR)/inject.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

test: unit-test

unit-test:
	@echo "=== UNIT TESTS ==="
	powershell -Command "./support/test/unit/make.bat"

$(OBJ_DIR):
	powershell -Command "New-Item -ItemType Directory -Force -Path $@"

$(BIN_DIR):
	powershell -Command "New-Item -ItemType Directory -Force -Path $@"

clean:
	powershell -Command "if (Test-Path $(BUILD_DIR)) {Remove-Item -Recurse -Force $(BUILD_DIR)}"
	powershell -Command "if (Test-Path $(GTEST_DIR)) {Remove-Item -Recurse -Force $(GTEST_DIR)}"

help:
	@echo "Available targets:"
	@echo "  all          - Build extract and inject executables"
	@echo "  extract      - Build the extraction executable (extract.exe)"
	@echo "  inject       - Build the injection executable (inject.exe)"
	@echo "  run          - Run the extract executable"
	@echo "  run-inject   - Run the inject executable"
	@echo "  test         - Build and run the unit tests"
	@echo "  clean        - Clean the build artifacts and Google Test library"
	@echo "  debug        - Start GDB debug session"
	@echo "  help         - Display this help message"
