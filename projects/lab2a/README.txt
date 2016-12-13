
===================
Name: Eric Sehun Oh
UID: 304184918
===================

Files included:

	Makefile:
		make tests - will generate csv files 
		make graphs - will run gnuplot, assuming that lab2_add.gp and lab2_list.gp will be in the folder.
		make tarball - will make dist
	lab2_add.c:
		C source file that implements two different add functions and allows for testing of synchronization and thread behavior with different types of protections.
	lab2_list.c:
		C source file that implements testing of thread synchronization and thread behavior on sorted doubly linked list behavior
	SortedList.h:
		a header file describing the interfaces for linked list operations.
	SortedList.c: 
		C source file that implements insert, delete, lookup, and length methods for a sorted doubly linked list (described in the provided header file, including correct placement of yield calls).
	test.sh:
		test bash script that generates csv files for lab2_add and lab2_list 
	lab2_add.csv:
		contains all results for all of the Part-1 tests.
	lab2_list.csv:
		contains all results for all of the Part-2 tests.
	lab2_add-1.png:	
		threads and iterations required to generate a failure (with and without yields)
	lab2_add-2.png:
		Average time per operation with and without yields.
	lab2_add-3.png:
		Average time per (single threaded) operation vs. the number of iterations.
	lab2_add-4.png:
		threads and iterations that can run successfully with yields under each of the three synchronization methods.
	lab2_add-5.png:
		Average time per (multi-threaded) operation vs. the number of threads, for all four versions of the add function.
	lab2_list-1.png:
		average time per (single threaded) unprotected operation vs. number of iterations (illustrating the correction of the per-operation cost for the list length).
	lab2_list-2.png:
		threads and iterations required to generate a failure (with and without yields).
	lab2_list-3.png:
		iterations that can run (protected) without failure.
	lab2_list-4.png
		average time per operation (for unprotected, mutex, and spin-lock) vs. number of threads.

====================
Answers to questions


QUESTION 2.1.1 - causing conflicts:
Why does it take many iterations before errors are seen? Why does a significantly smaller number of iterations so seldom fail?

	The more threads and iterations, the more ubiquitous the race conditions and higher the chance of errors surfacing becomes. At one thread, there will never be a race condition since there is only one thread acting upon the counter variable. Conceptually, iterations follow the same behavior.

QUESTION 2.1.2 - cost of yielding:
Why are the --yield runs so much slower?  Where is the additional time going?  Is it possible to get valid per-operation timings if we are using the --yield option?  If so, explain how.  If not, explain why not.

	When using the --yield, we yield the execution of the thread and perform a context switch. A context switch means that we have to save the state of the thread, values of the register, etc only to continue on the execution of this thread at a later time which becomes very time consuming. 

	We cannot get valid per-operation timings when using the yield option since the time and overhead for performing a context switch will be included in the overall elapsed time. This means the execution of the time does not accurately reflect the sole execution time of the thread, thus being much longer than the actual time.

QUESTION 2.1.3 - measurement errors:
Why does the average cost per operation drop with increasing iterations? If the cost per iteration is a function of the number of iterations, how do we know how many iterations to run (or what the “correct” cost is)?

	Since the time cost of creating a thread is quite high, with increasing number of iterations the average cost per operation becomes more reflective of the time actually performing the add. 

	As we drive the number of iteration to a large enough number 
	the cost per iteration will plateau and around that point is where the cost per iteration is the correct number with the thread initialization time being small enough to be ignored.

QUESTION 2.1.4 - costs of serialization:
Why do all of the options perform similarly for low numbers of threads? Why do the three protected operations slow down as the number of threads rises? Why are spin-locks so expensive for large numbers of threads?

	For low number of threads, less threads need to wait for the lock to be released and less threads will be using the lock thus the overall time required for operations should be relatively similar. However, as the number of threads rises, there will be a large competition for the lock and increased overall completion times of threads. Spin locks will cause a thread to "spin" and constantly check for the lock, taking up CPU power and thus will take more time.

QUESTION 2.2.1 - scalability of Mutex
Compare the variation in time per protected operation vs the number of threads (for mutex-protected operations) in Part-1 and Part-2, commenting on similarities/differences and offering explanations for them.

	For part 1, the average time for mutex-protected operations vs the number of threads for operations decreases as the number of threads increase. However, this was not the case for part two. With increasing number of threads, with iterations at 1000 when the initial start up costs are negligent, the average time increased. In part two, the lock is held for extended periods of time thus the time per operation increases.
	

QUESTION 2.2.2 - scalability of spin locks
Compare the variation in time per protected operation vs the number of threads for Mutex vs Spin locks, commenting on similarities/differences and offering explanations for them.

	In part 2, overall the time cost per mutex-protected operations were much smaller than the implementation containing spin-locks, but both were very inefficient. Because the critical section for part two is much larger than that of part 1, it is better for mutex locks to be used rather than just letting the thread spin for a very long time, however because the section is so large the parallelism is not effective. However, if the number of iterations are just right optimizations can be done.
