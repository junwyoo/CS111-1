#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include "SortedList.h"

int c;
int t_len;
int num_threads = 1;
int num_iters = 1;
int lock = 0;
int opt_yield = 0;
char opt_sync = 'd';
char test_name[15] = "list-";
long long counter = 0;
pthread_mutex_t mutex;
struct timespec start, end;
SortedList_t* list;
SortedListElement_t* elements; 
char *char_bank = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void *thread_routine(void *arg){
	int tid = *(int *)arg;
	for(int i = tid; i < num_iters * num_threads; i += num_threads) {
		if(opt_sync == 'm') {
			pthread_mutex_lock(&mutex);
			SortedList_insert(list, &elements[i]);
			pthread_mutex_unlock(&mutex);
		}
		else if(opt_sync == 's') {
			while(__sync_lock_test_and_set(&lock, 1) == 1);
			SortedList_insert(list, &elements[i]);
			__sync_lock_release(&lock);
		}
		else
			SortedList_insert(list, &elements[i]);
	}

	SortedList_length(list);
	SortedListElement_t* temp;

	for(int i = tid; i < num_iters * num_threads; i += num_threads) {
		if(opt_sync == 'm'){
			pthread_mutex_lock(&mutex);
			temp = SortedList_lookup(list, elements[i].key);
			if(temp == NULL) {
				fprintf(stderr, "error: key not found\n");
				exit(EXIT_FAILURE);
			}
			SortedList_delete(temp);
			pthread_mutex_unlock(&mutex);
		}
		else if(opt_sync == 's'){
			while(__sync_lock_test_and_set(&lock, 1) == 1);
			temp = SortedList_lookup(list, elements[i].key);
			if(temp == NULL) {
				fprintf(stderr, "error: key not found\n");
				exit(EXIT_FAILURE);
			}
			SortedList_delete(temp);
			__sync_lock_release(&lock);
		}
		else{
			temp = SortedList_lookup(list, elements[i].key);
			if(temp == NULL) {
				fprintf(stderr, "error: key not found\n");
				exit(EXIT_FAILURE);
			}	
			SortedList_delete(temp);
		}
	}
	return NULL;
}

void set_test_name(){
	if(!opt_yield)
		strcat(test_name, "none");
	strcat(test_name,"-");
	switch(opt_sync) {
		case 'm':
		case 's':
			t_len = strlen(test_name);
			test_name[t_len] = opt_sync;
			test_name[t_len + 1] = '\0';
			break;
		case 'd':
			strcat(test_name, "none");
			break;
	}
}

int main(int argc, char**argv) {
	static struct option long_opts[] = {
		{"iterations", required_argument, 0, 'i'},
		{"sync", required_argument, 0, 's'},
		{"threads", required_argument, 0, 't'},
		{"yield", required_argument, 0, 'y'},
		{0, 0, 0, 0},
	};

	while((c = getopt_long(argc, argv, "t:i:s:y:", long_opts, NULL)) != -1) {
		switch(c) {
			case 'i':
				num_iters = atoi(optarg);
				break;
			case 's':
				if(strlen(optarg) == 1) {
					switch(optarg[0]) {
						case 'm': 
						case 's':
							opt_sync = optarg[0]; 
							break;
						default:
							fprintf(stderr, "invalid sync opt\n");
							exit(EXIT_FAILURE);
							break;
					}
				}
				else {
					fprintf(stderr, "incorrect length for sync argument\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'y':
				if (strlen(optarg) == 0 || strlen(optarg) > 3) {
					fprintf(stderr, "incorrect yield options");
					exit(EXIT_FAILURE);
				}
				strcat(test_name,optarg);
				for(int i = 0; i < strlen(optarg); i++) {
					if (optarg[i] == 'i')
						opt_yield |= INSERT_YIELD;
					else if (optarg[i] == 'd')
						opt_yield |= DELETE_YIELD;
					else if (optarg[i] == 'l')
						opt_yield |= SEARCH_YIELD;
					else {
						fprintf(stderr, "incorrect yield options");
						exit(EXIT_FAILURE);
					}
        		}	
        		break;
		}
	}

	set_test_name();

	list = malloc(sizeof(SortedList_t));
	list->key = NULL;
	list->next = list;
	list->prev = list;

	int num_elems = num_iters * num_threads;

	elements = malloc(sizeof(SortedListElement_t) * num_elems);
	pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
	int* tid = malloc(num_threads * sizeof(int));

	srand(time(NULL));
	for(int i = 0; i < num_elems; i++) {
		int key_len = rand() % 5 + 10;
		char *key = malloc((key_len + 1) * sizeof(char));
		int j;
		for(j = 0; j < key_len; j++){
			key[j] = char_bank[rand() % strlen(char_bank)];
		}
		key[j] = '\0';
		elements[i].key = key;
	}

	if (opt_sync == 'm') 
    	pthread_mutex_init(&mutex, NULL);
  	
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (int i = 0; i < num_threads; i++) {
		tid[i] = i;
		if(pthread_create(&(threads[i]), NULL, thread_routine, &tid[i]) < 0) {
			perror("pthread create error"); 
	    	exit(EXIT_FAILURE);
		}
	}

	for(int i = 0; i < num_threads; i++) {
		if(pthread_join(threads[i], NULL) < 0) {
			perror("pthread create error"); 
	    	exit(EXIT_FAILURE);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	long long elapsed_time = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
	int num_ops = 2 * num_threads * num_iters;

	int list_len = SortedList_length(list);
	if(list_len != 0) {
		fprintf(stderr, "error: list length %d\n", list_len);
		exit(EXIT_FAILURE);
	}

	printf("%s,%d,%d,%d,%d,%lld,%lld\n",test_name, num_threads, num_iters, 1, num_ops, elapsed_time, elapsed_time / num_ops);
	
	free(threads);
	free(elements);
	free(tid);

	exit(EXIT_SUCCESS);
}

