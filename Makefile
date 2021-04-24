CC=gcc
AR=ar
CFLAGS=-O3 -MMD -MP
LDFLAGS=

SRCS=main.c enigma.c

all:app

lib:libenigma.a

app:$(SRCS:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^

libenigma.a:$(SRCS:.c=.o)
	$(AR) rcs $@ $^

-include $(SRCS:.c=.d)

clean:
	rm -f app *.o *.d *.a