CC=gcc
AR=ar
CFLAGS=-O3 -MMD -MP
LDFLAGS=

SRCS=main.c enigma.c
LIBSRCS=enigma.c

all:app

lib:libenigma.a

app:$(SRCS:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^

libenigma.a:$(LIBSRCS:.c=.o)
	$(AR) rcs $@ $^

-include $(SRCS:.c=.d)

clean:
	rm -f app *.o *.d *.a