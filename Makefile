CC=gcc
CFLAGS=-Wall -g

task2: task2.o
	$(CC) task2.o -o task2 $(LDFLAGS)

task2.o: task2.c
	$(CC) $(CFLAGS) -c task2.c

clean:
	rm -rf *.o task2