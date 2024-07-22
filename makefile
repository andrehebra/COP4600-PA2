CC = gcc
CFLAGS = -Wall -pthread

SRCS = main.c chash.c dbase.c
OBJS = $(SRCS:.c=.o)
TARGET = chash

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $

clean:
	rm -f $(OBJS) $(TARGET)