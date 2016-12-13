#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_add.csv lab2_list.csv lab_2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#   lab2b_1.png ... throughput vs number of threads for mutex and spin­lock synchronized adds and list operations.
#   lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex­synchronized list operations.
#   lab2b_3.png ... number of successful iterations for each synchronization method.
#   lab2b_4.png ... throughput vs number of threads for mutexes with partitioned lists.
#   lab2b_5.png ... throughput vs number of threads for spin­locks with partitioned lists.
#

# general plot parameters
set terminal png
set datafile separator ","

# throughput vs num threads for m/s synch adds/list
set title "2b-1: throughput vs number of threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "throughput (op/s)"
set logscale y 10
set output 'lab2b_1.png'
set key left top

plot \
     "< grep add-m lab2_add.csv" using ($2):(1000000000/($6)) \
	title 'mutex add, 10000 iters' with linespoints lc rgb 'red', \
     "< grep add-s lab2_add.csv" using ($2):(1000000000/($6)) \
	title 'spin-lock add, 10000 iters' with linespoints lc rgb 'green', \
     "< grep list-none-m lab2_list.csv" using ($2):(1000000000/($6)) \
	title 'mutex list, 1000 iters' with linespoints lc rgb 'blue', \
     "< grep list-none-s lab2_list.csv" using ($2):(1000000000/($6)) \
	title 'spin-lock list, 1000 iters' with linespoints lc rgb 'orange'

# mean time per mutex wait and mean time per operation for mutex­synchronized list operations.
set title "2b-2: mean time per mutex wait and mean time per operation vs number of threads"
set xlabel "Threads"
set logscale x 2
set ylabel "mean time for wait/op"
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep list-none-m lab_2b_list.csv | grep 1000,1," using ($2):($7) \
	title 'mean time per operation' with linespoints lc rgb 'red', \
     "< grep list-none-m lab_2b_list.csv | grep 1000,1," using ($2):($8) \
	title 'mean time per lock' with linespoints lc rgb 'green'

# number of successful iterations for each synchronization method.
set title "2b-3: number of iterations vs number of threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Iterations"
set logscale y 10
set output 'lab2b_3.png'

plot \
     "< grep list-id-m lab_2b_list.csv" using ($2):($3) \
	title 'mutex' with points lc rgb 'red', \
     "< grep list-id-s lab_2b_list.csv" using ($2):($3) \
	title 'spin-lock' with points lc rgb 'green', \
     "< grep list-id-none lab_2b_list.csv" using ($2):($3) \
	title 'none' with points lc rgb 'blue'

# throughput vs number of threads for mutexes with partitioned lists.
set title "2b-4: throughput vs number of threads for mutex"
set xlabel "Threads"
set logscale x 2
set ylabel "throughput (op/s)"
set output 'lab2b_4.png'
set key right top

plot \
     "< grep list-none-m lab_2b_list.csv | grep 1000,1," using ($2):(1000000000/($6)) \
	title 'list = 1' with linespoints lc rgb 'red', \
     "< grep list-none-m lab_2b_list.csv | grep 1000,4," using ($2):(1000000000/($6)) \
	title 'lists = 4' with linespoints lc rgb 'green', \
     "< grep list-none-m lab_2b_list.csv | grep 1000,8," using ($2):(1000000000/($6)) \
	title 'lists = 8' with linespoints lc rgb 'blue', \
     "< grep list-none-m lab_2b_list.csv | grep 1000,16," using ($2):(1000000000/$6) \
	title 'lists = 16' with linespoints lc rgb 'orange'

# throughput vs number of threads for spin­locks with partitioned lists.
set title "2b-5: throughput vs number of threads for spin-lock"
set xlabel "Threads"
set logscale x 2
set ylabel "throughput (op/s)"
set output 'lab2b_5.png'
set key right top

plot \
     "< grep list-none-s lab_2b_list.csv | grep 1000,1," using ($2):(1000000000/($6)) \
	title 'list = 1' with linespoints lc rgb 'red', \
     "< grep list-none-s lab_2b_list.csv | grep 1000,4," using ($2):(1000000000/($6)) \
	title 'lists = 4' with linespoints lc rgb 'green', \
     "< grep list-none-s lab_2b_list.csv | grep 1000,8," using ($2):(1000000000/($6)) \
	title 'lists = 8' with linespoints lc rgb 'blue', \
     "< grep list-none-s lab_2b_list.csv | grep 1000,16," using ($2):(1000000000/($6)) \
	title 'lists = 16' with linespoints lc rgb 'orange'