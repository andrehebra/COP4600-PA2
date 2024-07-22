# Define the source files
SRC := \
    chash.c \
    dbase.c \
    cparse_input.c \
    cthread.c \
    main.c

# Define the object files
OBJ := $(SRC:%.c=%.o)

# Define the output program name
PRG := myprogram

# Set the compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -g -pthread

# Define the targets
.PHONY: all clean

# Default target
all: $(PRG)

# Build the program
$(PRG): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Build object files from source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJ) $(PRG)