CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-pthread

task7: task7.o
	$(CC) task7.o -o task7 $(LDFLAGS)

task7.o: task7.c
	$(CC) $(CFLAGS) -c task7.c

clean:
	rm -rf *.o task7