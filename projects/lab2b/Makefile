CC=gcc
CFLAGS=-pthread -g -Wall

build: lab2_add.c lab2_list.c
	$(CC) $(CFLAGS) -o lab2_add lab2_add.c
	$(CC) $(CFLAGS) -o lab2_list lab2_list.c SortedList.c

tests:
	bash test.sh

graphs:
	gnuplot lab2b.gp

profile:
	rm -rf *.prof
	LD_PRELOAD=/usr/lib/libprofiler.so.0 CPUPROFILE=raw.prof ./lab2_list --threads=12 --iterations=1000 --sync=s
	google-pprof --text ./lab2_list raw.prof >> exec.prof
	google-pprof --list=thread_routine ./lab2_list raw.prof >> exec.prof

tarball:
	rm -rf lab2b-304184918.tar.gz
	tar -czf lab2b-304184918.tar.gz *.c *.h *.gp Makefile README.txt 

clean:
	rm -rf lab2_list lab2_add *.csv *.png *.prof
