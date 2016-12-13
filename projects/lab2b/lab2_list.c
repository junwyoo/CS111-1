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
int num_lists = 1;
int lock = 0;
int opt_yield = 0;
char opt_sync = 'd';
char test_name[15] = "list-";
long long counter = 0;
// pthread_mutex_t mutex;
// SortedList_t* list;
SortedList_t* lists;
SortedListElement_t* elements; 
char *char_bank = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int *offset;
pthread_mutex_t* mutexes;
int* spin_locks;
int list_len = 0;
long long* wait_times;

void *thread_routine(void *arg){
	int tid = *(int *)arg;
	struct timespec start, end;

	for(int i = tid; i < num_iters * num_threads; i += num_threads) {
		if(opt_sync == 'm') {
			clock_gettime(CLOCK_MONOTONIC, &start);
			pthread_mutex_lock(&mutexes[offset[i]]);
			clock_gettime(CLOCK_MONOTONIC, &end);
			SortedList_insert(&lists[offset[i]], &elements[i]); 
			pthread_mutex_unlock(&mutexes[offset[i]]);
			wait_times[tid] = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
		}
		else if(opt_sync == 's') {
			while(__sync_lock_test_and_set(&spin_locks[offset[i]], 1));
			SortedList_insert(&lists[offset[i]], &elements[i]);
			__sync_lock_release(&spin_locks[offset[i]]);
		}
		else
			SortedList_insert(&lists[offset[i]], &elements[i]);
	}

	// SortedList_length(list);
	list_len = 0;

	if(opt_sync == 'm'){
		for(int i = 0; i < num_lists; i++) {
			clock_gettime(CLOCK_MONOTONIC, &start);
			pthread_mutex_lock(&mutexes[i]);
			clock_gettime(CLOCK_MONOTONIC, &end);
			list_len += SortedList_length(&lists[i]);
			pthread_mutex_unlock(&mutexes[i]);
			wait_times[tid] += (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
		}
	}
	else if(opt_sync == 's'){
		for(int i = 0; i < num_lists; i++) {
			while(__sync_lock_test_and_set(&spin_locks[i], 1) == 1);
			list_len += SortedList_length(&lists[i]);
			__sync_lock_release(&spin_locks[i]);
		}
	}
	else {
		for(int i = 0; i < num_lists; i++)
			list_len += SortedList_length(&lists[i]);
	}

	SortedListElement_t* temp;
	for(int i = tid; i < num_iters * num_threads; i += num_threads){
		if(opt_sync == 'm') {
			clock_gettime(CLOCK_MONOTONIC, &start);
			pthread_mutex_lock(&mutexes[offset[i]]);
			clock_gettime(CLOCK_MONOTONIC, &end);
			temp = SortedList_lookup(&lists[offset[i]], elements[i].key);
			SortedList_delete(temp);
			pthread_mutex_unlock(&mutexes[offset[i]]);
			wait_times[tid] += (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
		}
		else if(opt_sync == 's') {
			while(__sync_lock_test_and_set(&spin_locks[offset[i]], 1) == 1);
			temp = SortedList_lookup(&lists[offset[i]], elements[i].key);
			SortedList_delete(temp);
			__sync_lock_release(&spin_locks[offset[i]]);
		}
		else{
			temp = SortedList_lookup(&lists[offset[i]], elements[i].key);
			SortedList_delete(temp);
		}
	}
	return NULL;
}

void set_test_name() {
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

int hash_function(const char* key) {
  int hash = 0;
  for (int i = 0; i < strlen(key); i++)
	hash += (int)key[i];
  return hash % num_lists;
}

int main(int argc, char**argv) {
	static struct option long_opts[] = {
		{"iterations", required_argument, 0, 'i'},
		{"lists", required_argument, 0, 'l'},
		{"sync", required_argument, 0, 's'},
		{"threads", required_argument, 0, 't'},
		{"yield", required_argument, 0, 'y'},
		{0, 0, 0, 0},
	};

	while((c = getopt_long(argc, argv, "t:i:l:s:y:", long_opts, NULL)) != -1) {
		switch(c) {
			case 'i':
				num_iters = atoi(optarg);
				break;
			case 'l':
				num_lists = atoi(optarg);
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

	// list = malloc(sizeof(SortedList_t));
	// list->key = NULL;
	// list->next = list;
	// list->prev = list;

	lists = malloc(sizeof(SortedList_t) * num_lists);
	for(int i = 0; i < num_lists; i++) {
		lists[i].key = NULL;
		lists[i].next = &lists[i];
		lists[i].prev = &lists[i];
	}

	int num_elems = num_iters * num_threads;
	elements = malloc(sizeof(SortedListElement_t) * num_elems);
	pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
	int* tid = malloc(sizeof(int) * num_threads);
	offset = malloc(sizeof(int) * num_elems);
	wait_times = malloc(sizeof(long long) * num_threads);

	if(opt_sync == 'm') {
		mutexes = malloc(sizeof(pthread_mutex_t) * num_lists);
		for(int i = 0; i < num_lists; i++)
			pthread_mutex_init(&mutexes[i], NULL);
	} else if(opt_sync == 's') {
		spin_locks = malloc(sizeof(int) * num_lists);
		for(int i = 0; i < num_lists; i++)
			spin_locks[i] = 0;
	}

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

	struct timespec start, end;
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
	int num_ops = 3 * num_threads * num_iters;

	// int list_len = SortedList_length(list);
	list_len = 0;
	for(int i = 0; i < num_lists; i++)
		list_len += SortedList_length(&lists[i]);

	long long total_wait_time = 0;
	for(int i = 0; i < num_threads; i++)
		total_wait_time += wait_times[i];

	if(list_len != 0) {
		fprintf(stderr, "error: list length %d\n", list_len);
		exit(EXIT_FAILURE);
	}

	if(opt_sync == 'm')
		printf("%s,%d,%d,%d,%d,%lld,%lld,%lld\n",test_name, num_threads, num_iters, num_lists, num_ops, elapsed_time, elapsed_time / num_ops, total_wait_time / num_ops);
	else
		printf("%s,%d,%d,%d,%d,%lld,%lld\n",test_name, num_threads, num_iters, num_lists, num_ops, elapsed_time, elapsed_time / num_ops);
	
	free(threads);
	free(elements);
	free(tid);
	free(lists);
	free(offset);
	free(mutexes);
	free(spin_locks);
	free(wait_times);

	exit(EXIT_SUCCESS);
}

