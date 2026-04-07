CC=gcc
CFLAGS=-Wall -g

task4: task4.o
	$(CC) task4.o -o task4

task4.o: task4.c
	$(CC) $(CFLAGS) -c task4.c

clean:
	rm -rf *.o task4