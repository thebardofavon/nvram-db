CC = gcc
CFLAGS = -Wall -Wextra -g -I./src -pthread
TARGET = nvram_db
THREAD_TEST = thread_test

# Source files
SRC = src/db_main.c src/free_space.c src/ram_free_space.c src/ram_bptree.c src/wal.c src/lock_manager.c
TEST_SRC = src/thread_test.c src/free_space.c src/ram_bptree.c src/wal.c src/lock_manager.c

# Object files
OBJ = $(SRC:.c=.o)
TEST_OBJ = $(TEST_SRC:.c=.o)

# Default target
all: $(TARGET) $(THREAD_TEST)

# Link object files to create executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Build the thread test
$(THREAD_TEST): $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(TEST_OBJ) $(TARGET) $(THREAD_TEST)

# Run the database
run: $(TARGET)
	./$(TARGET)

# Run the thread test
run-test: $(THREAD_TEST)
	./$(THREAD_TEST)

.PHONY: all clean run run-test