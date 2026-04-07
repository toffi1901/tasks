CC=gcc
CFLAGS=-Wall -g

task6: task6.o
	$(CC) task6.o -o task6 

task6.o: task6.c
	$(CC) $(CFLAGS) -c task6.c

clean:
	rm -rf *.o task6
