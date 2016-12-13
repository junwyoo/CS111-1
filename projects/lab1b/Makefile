CC=gcc
CFLAGS=-pthread -lmcrypt

default: client.c server.c
	$(CC) $(CFLAGS) -o client client.c 
	$(CC) $(CFLAGS) -o server server.c 

client: client.c
	$(CC) $(CFLAGS) -o client client.c 

server: server.c
	$(CC) $(CFLAGS) -o server server.c 

clean:
	rm -rf client server

dist:
	tar -czf lab1b-304184918.tar.gz *.c Makefile README my.key
