CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = value_counts

all: $(TARGET)

$(TARGET): value_counts.c
	$(CC) $(CFLAGS) -o $(TARGET) value_counts.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
