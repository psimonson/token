CFLAGS=-std=c89 -Wall -Wextra $(shell pkg-config --cflags prs)
LDFLAGS=$(shell pkg-config --libs prs)
TARGET=token

.PHONY: all clean
all: $(TARGET)

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): main.c.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *~ *.o $(TARGET)
