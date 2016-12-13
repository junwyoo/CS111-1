#!/bin/bash
rm -rf lab_2b_list.csv lab2_add.csv lab2_list.csv

ms_add_threads=(1 2 4 8 12)
ms_list_threads=(1 2 4 8 12 16 24)
threads_2b_3=(1 4 8 12 16)
iters_2b_3_1=(1 2 4 8 16)
iters_2b_3_2=(10 20 40 80)
threads_2b_45=(1 2 4 8 12)
lists_2b_45=(1 4 8 16)


#2b_1 gen
for t in "${ms_add_threads[@]}"; do
	echo "./lab2_add --iterations=10000 --threads=$t --sync=m"	
	./lab2_add --iterations=10000 --threads=$t --sync=m 1>>lab2_add.csv	

	echo "./lab2_add --iterations=10000 --threads=$t --sync=s"	
	./lab2_add --iterations=10000 --threads=$t --sync=s 1>>lab2_add.csv	
done

for t in "${ms_list_threads[@]}"; do
	echo "./lab2_list --iterations=1000 --threads=$t --sync=m"	
	./lab2_list --iterations=1000 --threads=$t --sync=m 1>>lab2_list.csv	

	echo "./lab2_list --iterations=1000 --threads=$t --sync=s"	
	./lab2_list --iterations=1000 --threads=$t --sync=s 1>>lab2_list.csv	
done

#2b_2 gen


#2b_3 gen
for t in "${threads_2b_3[@]}"; do
	for i in "${iters_2b_3_1[@]}"; do
		echo "./lab2_list --iterations=$i --threads=$t --yield=id"
		./lab2_list --iterations=$i --threads=$t --yield=id --lists=4 1>>lab_2b_list.csv
	done

	for i in "${iters_2b_3_2[@]}"; do
		echo "./lab2_list --iterations=$i --threads=$t --yield=id --sync=m"
		./lab2_list --iterations=$i --threads=$t --yield=id --lists=4 --sync=m 1>>lab_2b_list.csv

		echo "./lab2_list --iterations=$i --threads=$t --yield=id --sync=s"
		./lab2_list --iterations=$i --threads=$t --yield=id --lists=4 --sync=s 1>>lab_2b_list.csv
	done
done

#2b_45 gen
for t in "${threads_2b_45[@]}"; do
	for l in "${lists_2b_45[@]}"; do
		echo "./lab2_list --iterations=1000 --lists=$l --threads=$t --sync=m"
		./lab2_list --iterations=1000 --lists=$l --threads=$t --sync=m 1>>lab_2b_list.csv
	
		echo "./lab2_list --iterations=1000 --lists=$l --threads=$t --sync=s"
		./lab2_list --iterations=1000 --lists=$l --threads=$t --sync=s 1>>lab_2b_list.csv
	done
done


# for t in "${threads[@]}"; do
# 	for i in "${add_1_iter[@]}"; do
# 		echo "./lab2a_add --iterations=$i --threads=$t"
# 		./lab2_add --iterations=$i --threads=$t 1>>lab2_add.csv
# 	done
# done


# for t in "${threads[@]}"; do
# 	for i in "${add_2_iter[@]}"; do
# 		echo "./lab2a_add --iterations=$i --threads=$t --yield"
# 		./lab2_add --iterations=$i --threads=$t --yield 1>>lab2_add.csv
# 	done
# done

# for t in "${threads[@]}"; do
# 	echo "./lab2a_add --iterations=10000 --threads=$t --sync=c"
# 	./lab2_add --iterations=$i --threads=$t --sync=c 1>>lab2_add.csv

# 	echo "./lab2a_add --iterations=10000 --threads=$t --sync=m"
# 	./lab2_add --iterations=$i --threads=$t --sync=m 1>>lab2_add.csv

# 	echo "./lab2a_add --iterations=1000 --threads$t --sync=s"
# 	./lab2_add --iterations=$i --threads=$t --sync=s 1>>lab2_add.csv

# 	echo "./lab2a_add --iterations=1000 --threads$t"
# 	./lab2_add --iterations=$i --threads=$t 1>>lab2_add.csv

# 	echo "./lab2a_add --iterations=10000 --threads=$t --sync=c --yield"
# 	./lab2_add --iterations=$i --threads=$t --sync=c  --yield 1>>lab2_add.csv

# 	echo "./lab2a_add --iterations=10000 --threads=$t --sync=m --yield"
# 	./lab2_add --iterations=$i --threads=$t --sync=m  --yield 1>>lab2_add.csv

# 	echo "./lab2a_add --iterations=1000 --threads$t --sync=s --yield"
# 	./lab2_add --iterations=$i --threads=$t --sync=s  --yield 1>>lab2_add.csv

# 	echo "./lab2a_add --iterations=1000 --threads$t --yield"
# 	./lab2_add --iterations=$i --threads=$t  --yield 1>>lab2_add.csv
# done

# # list
# for i in "${list_1_iter[@]}"; do
# 	echo "./lab2_list --iterations=$i --threads=1"
# 	./lab2_list --iterations=$i --threads=1 1>>lab2_list.csv
# done


# for t in "${threads[@]}"; do
# 	for i in "${list_2_iter[@]}"; do
# 		for y in "${list_yield[@]}"; do
# 			echo "./lab2_list --iterations=$i --threads=$t --yield=$y --sync=m"
# 			./lab2_list --iterations=$i --threads=$t --yield=$y --sync=m 1>>lab2_list.csv

# 			echo "./lab2_list --iterations=$i --threads=$t --yield=$y --sync=s"
# 			./lab2_list --iterations=$i --threads=$t --yield=$y --sync=s 1>>lab2_list.csv

# 			echo "./lab2_list --iterations=$i --threads=$t --yield=$y"
# 			./lab2_list --iterations=$i --threads=$t --yield=$y 1>>lab2_list.csv
# 		done
# 		echo "./lab2_list --iterations=$i --threads=$t --sync=m"
# 		./lab2_list --iterations=$i --threads=$t --sync=m 1>>lab2_list.csv

# 		echo "./lab2_list --iterations=$i --threads=$t --sync=s"
# 		./lab2_list --iterations=$i --threads=$t --sync=s 1>>lab2_list.csv

# 		echo "./lab2_list --iterations=$i --threads=$t"
# 		./lab2_list --iterations=$i --threads=$t 1>>lab2_list.csv
# 	done
# done