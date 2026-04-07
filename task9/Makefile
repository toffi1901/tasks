CC=gcc
CFLAGS=-Wall -g

task9: task9.o
	$(CC) task9.o -o task9 $(LDFLAGS)

task9.o: task9.c
	$(CC) $(CFLAGS) -c task9.c

clean:
	rm -rf *.o task9
