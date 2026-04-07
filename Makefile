CC=gcc
CFLAGS=-Wall -g -D_GNU_SOURCE
LDFLAGS=-pthread

task8: task8.o
	$(CC) task8.o -o task8 $(LDFLAGS)

task8.o: task8.c
	$(CC) $(CFLAGS) -c task8.c

clean:
	rm -rf *.o task8