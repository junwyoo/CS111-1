#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

int c;
int opt_yield;
int num_threads = 1;
int num_iters = 1;
int lock = 0;
char opt_sync = 'd';
char test_name[13];
long long counter = 0;
pthread_mutex_t mutex;
struct timespec start, end;

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield)
    	pthread_yield();
	*pointer = sum;
}

void compare_add(long long *pointer, long long value) {
	long long prev, sum;
	do {
		prev = *pointer;
		sum = prev + value;
		if(opt_yield)
			pthread_yield();
	} while(__sync_val_compare_and_swap(pointer, prev, sum) != prev);
}

void* thread_routine(void* counter) {
	for(int i = 0; i < num_iters; i++) {
		if (opt_sync == 'c') {
			compare_add((long long*) counter, 1);
		} 
		else if (opt_sync == 's') {
			while(__sync_lock_test_and_set(&lock, 1));
			add((long long *) counter, 1);
			__sync_lock_release(&lock);
		} 
		else if(opt_sync == 'm') {
			pthread_mutex_lock(&mutex);
			add((long long*) counter, 1);
			pthread_mutex_unlock(&mutex);
		} 
		else {
			add((long long*) counter, 1);
		}
	}

	for(int j = 0; j < num_iters; j++) {
		if (opt_sync == 'c') {
			compare_add((long long*) counter, -1);
		} 
		else if (opt_sync == 's') {
			while(__sync_lock_test_and_set(&lock, 1));
			add((long long *) counter, -1);
			__sync_lock_release(&lock);
		} 
		else if(opt_sync == 'm') {
			pthread_mutex_lock(&mutex);
			add((long long*) counter, -1);
			pthread_mutex_unlock(&mutex);
		} 
		else {
			add((long long*) counter, -1);
		}
	}
	return NULL;
}

void set_test_name() {
	if(opt_yield) {
		if(opt_sync == 'd')
			strcpy(test_name, "add-yield-none");
		else {
			switch(opt_sync) {
				case 'c': 
					strcpy(test_name, "add-yield-c");
					break;
				case 'm': 
					strcpy(test_name, "add-yield-m");
					break;	
				case 's': 
					strcpy(test_name, "add-yield-s");
					break;	
			}
	 	}
	} else {
		if(opt_sync == 'd')
			strcpy(test_name, "add-none");
		else {
			switch(opt_sync) {
				case 'c': 
					strcpy(test_name, "add-c");
					break;
				case 'm': 
					strcpy(test_name, "add-m");
					break;	
				case 's': 
					strcpy(test_name, "add-s");
					break;	
			}
	 	}
	}
}

int main(int argc, char**argv) {
	static struct option long_opts[] = {
		{"iterations", required_argument, 0, 'i'},
		{"sync", required_argument, 0, 's'},
		{"threads", required_argument, 0, 't'},
		{"yield", no_argument, &opt_yield, 'y'},
		{0, 0, 0, 0},
	};

	while((c = getopt_long(argc, argv, "t:i:", long_opts, NULL)) != -1) {
		switch(c) {
			case 'i':
				num_iters = atoi(optarg);
				break;
			case 's':
				if(strlen(optarg) == 1) {
					switch(optarg[0]) {
						case 'c':
						case 'm': 
						case 's':
							opt_sync = optarg[0]; 
							break;
						default:
							break;
					}
				}
				else 
					fprintf(stderr, "incorrect length for sync argument\n");
				break;
			case 't':
				num_threads = atoi(optarg);
				break;
		}
	}

	pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);

	if (opt_sync == 'm') 
    	pthread_mutex_init(&mutex, NULL);
  	
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (int i = 0; i < num_threads; i++) {
	    if (pthread_create(&(threads[i]), NULL, thread_routine, &counter) < 0) {
	    	perror("pthread create error"); 
	    	exit(EXIT_FAILURE);
	    }
	}

	for(int i = 0; i < num_threads; i++) {
		if (pthread_join(threads[i], NULL) < 0) {
	    	perror("pthread join error"); 
	    	exit(EXIT_FAILURE);
	    }
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	long long elapsed_time = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
	int num_ops = 2 * num_threads * num_iters;
	
	set_test_name();
	printf("%s,%d,%d,%d,%lld,%lld,%lld\n", test_name, num_threads, num_iters, num_ops, elapsed_time, elapsed_time / num_ops, counter);
	
	if(counter)
		exit(EXIT_FAILURE);
	exit(EXIT_SUCCESS);
}

