CC=gcc
CFLAGS=-g -Wall

build: lab3a.c
	$(CC) $(CFLAGS) -o lab3a lab3a.c

run:
	./lab3a disk-image

check:
	diff super.csv c_csv/super.csv
	diff group.csv c_csv/group.csv
	sort bitmap.csv -no bitmap.csv
	sort -n c_csv/bitmap.csv > sorted_bitmap.csv
	diff bitmap.csv sorted_bitmap.csv
	diff inode.csv c_csv/inode.csv
	diff directory.csv c_csv/directory.csv
	diff indirect.csv c_csv/indirect.csv

dist:
	rm -rf lab3a-304184918.tar.gz
	tar -czf lab3a-304184918.tar.gz *

clean:
	rm -rf lab3a *.csv
