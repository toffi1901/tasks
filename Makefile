CC=gcc
CFLAGS=-Wall -g

task5: task5.o
	$(CC) task5.o -o task5

task5.o: task5.c
	$(CC) $(CFLAGS) -c task5.c

clean:
	rm -rf *.o task5