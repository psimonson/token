CFLAGS=-std=c89 -Wall -Wextra $(shell pkg-config --cflags prs)
LDFLAGS=$(shell pkg-config --libs prs)

.PHONY: all clean
all: token

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

token: token.c.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *~ *.o token
