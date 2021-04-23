CC=gcc

CFLAGS=-O3

LDFLAGS=

all:app

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $<

app:main.o enigma.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f app *.o