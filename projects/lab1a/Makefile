CC = gcc
CFLAGS = -Wall
TARGET = lab1a

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -pthread -o $(TARGET) $(TARGET).c

check:
	./lab1a
	
clean:
	$(RM) $(TARGET)

dist: 
	tar -czf lab1a-304184918.tar.gz *.c README Makefile 
