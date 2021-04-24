CC=gcc
CFLAGS=-O3 -MMD -MP
LDFLAGS=

SRCS=main.c enigma.c

all:app

app:$(SRCS:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^

-include $(SRCS:.c=.d)

clean:
	rm -f app *.o *.d