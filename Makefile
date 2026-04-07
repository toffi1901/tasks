CC=gcc
CFLAGS=-Wall -g

task3: task3.o
	$(CC) task3.o -o task3

task3.o: task3.c
	$(CC) $(CFLAGS) -c task3.c

clean:
	rm -rf *.o task3