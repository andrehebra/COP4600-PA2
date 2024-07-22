CC = clang
CFLAGS = -Wall -pthread
LDFLAGS = -pthread

SRCS = main.c chash.c dbase.c cparse_input.c cthread.c
OBJS = $(SRCS:.c=.o)
TARGET = chash

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

chash.o: chash.c
	$(CC) $(CFLAGS) -c chash.c

dbase.o: dbase.c
	$(CC) $(CFLAGS) -c dbase.c

cparse_input.o: cparse_input.c
	$(CC) $(CFLAGS) -c cparse_input.c

cthread.o: cthread.c
	$(CC) $(CFLAGS) -c cthread.c

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean