
===================
Name: Eric Sehun Oh
UID: 304184918
===================

Note

	- The development testing was done on Ubuntu 16.04 LTS in a VirtualBox environment on Mac OS X with 4 cores.
	- The order in which the makes should be done as follows:
		make
		make test
		make graphs
		make profile

===================

Files included:

	Makefile:
		make - will build lab2_add and newly implemented lab2_list
		make tests - will generate csv files for by running the test.sh bash script lab2_add, lab2_list, and lab_2b_list
		make graphs - will run gnuplot with the lab2b.gp file
		make profile - will create the execution profiling reports
		make tarball - will make dist
	lab2_add.c:
		C source file that implements two different add functions and allows for testing of synchronization and thread behavior with different types of protections.
	lab2_list.c:
		C source file that implements testing of thread synchronization and thread behavior on sorted doubly linked list behavior
		with additional features allowing for multiple lists and timing of mutex locks
	SortedList.h:
		a header file describing the interfaces for linked list operations.
	SortedList.c: 
		C source file that implements insert, delete, lookup, and length methods for a sorted doubly linked list (described in the provided header file, including correct placement of yield calls).
	test.sh:
		test bash script that generates csv files for lab2_add, lab2_list and lab_2b_list
	lab2b.gp: 
		gp file that will graph the necessary graphs for lab 2b


====================
Answers to questions

QUESTION 2.3.1 ­ 
Where do you believe most of the cycles are spent in the 1 and 2­ thread tests (for both add and list)? Why do you believe these to be the most expensive parts of the code? Where do you believe most of the time/cycles are being spent in the high­thread spin­lock tests? Where do you believe most of the time/cycles are being spent in the high­thread mutex tests?
	
	Since there is not much contention, most of the cycles for the add and list should be spent in their main routines. For the add, in the add function; for the list, looking up elements in the list. In highthread spinlock tests, I believe a lot of the time/cycles will be spent spinning and waiting since spin­locks waste increasingly more cycles as the probability of contention increases. In highthread mutex tests, they will be spending most time performing their main routines since it will just block instead of wasting cpu cycles like spin-lock.

	
QUESTION 2.3.2 
Where (what lines of code) are consuming most of the cycles when the spin­lock version of the list exerciser is run with a large number of threads? Why does this operation become so expensive with large numbers of threads?

	The line of code is at
		while(__sync_lock_test_and_set(&spin_locks[offset[i]], 1) == 1);
	where the thread spins and wait for the lock. This is because spin­locks waste increasingly more cycles as the probability of contention increases. In addition spinlocks do not implement fairness well and thus some threads face starvation.

QUESTION 2.3.3 ­ Mutex Wait Time:
Look at the average time per operation (vs # threads) and the average wait­ for ­mutex time (vs #threads). Why does the average lock­ wait time rise so dramatically with the number of contending threads? Why does the completion time per operation rise (less dramatically) with the number of contending threads? How is it possible for the wait time per operation to go up faster (or higher) than the completion time per operation?
	
	Since there is greater contention due to an increased number of threads, that means there are a greater number of threads waiting to enter the critical section since only one thread can enter at a time. Thus the lock wait time increases. However the average completion time is less affected and time increases less dramatically since they can just yield instead of spinning like spinlocks. 
	

QUESTION 2.3.4 ­ Performance of Partitioned Lists
Explain the change in performance of the synchronized methods as a function of the number of lists. Should the throughput continue increasing as the number of lists is further increased? If not, explain why not. It seems reasonable to suggest the throughput of an N­way partitioned list should be equivalent to the throughput of a single list with fewer (1/N) threads. Does this appear to be true in the above curves? If not, explain why not.

	The performance of the synchronized methods as the number of lists increases does not necessarily increase. There may be less time consumed during lookup for a list per list however this difference is 
	minimal and negligible when taking into account the addition number of context switches that must occur due to multiple lists and the additional computing that must be done to manage these sublists.

