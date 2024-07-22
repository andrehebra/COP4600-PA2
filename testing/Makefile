# Makefile for the concurrent hash table program

# Compiler and compiler flags
CC = gcc
CFLAGS = -pthread -Wall -Wextra -Werror

# Target executable name
TARGET = concurrent_hash_table

# Source files
SRCS = concurrent_hash_table.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile the source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove generated files
clean:
	rm -f $(OBJS) $(TARGET) output.txt

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
