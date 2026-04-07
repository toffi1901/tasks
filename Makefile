CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-pthread

task1: task1.o
	$(CC) task1.o -o task1 $(LDFLAGS)

task1.o: task1.c
	$(CC) $(CFLAGS) -c task1.c

clean:
	rm -rf *.o task1