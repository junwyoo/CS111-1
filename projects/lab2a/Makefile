CC=gcc
CFLAGS=-pthread -Wall

build: lab2_add.c lab2_list.c
	$(CC) $(CFLAGS) -o lab2_add lab2_add.c
	$(CC) $(CFLAGS) -o lab2_list lab2_list.c SortedList.c

tests:
	bash test.sh

graphs:
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

tarball:
	rm -rf lab2a-304184918.tar.gz
	tar -czf lab2a-304184918.tar.gz *.c *.h *.png *.gp *.csv Makefile README.txt 

clean:
	rm -rf lab2_add lab2_list
