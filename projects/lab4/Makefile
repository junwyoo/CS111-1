CC=gcc
CFLAGS=-lmraa -pthread -lm
default: lab4a.c lab4b.c
	$(CC) $(CFLAGS) -o lab4a lab4a.c 
	$(CC) $(CFLAGS) -o lab4b lab4b.c 

run:
	./lab4a
	./lab4b

clean:
	rm -rf lab4a lab4b log4a log4b.txt


dist:
	tar -czf lab4-304184918.tar.gz *.c log* Makefile 