#!/bin/bash
clear

mkdir results
alpha=$1
subset_size=$2
plateau_type=$3
plateau_value=$4
pre_process=$5
post_process=$6

echo "Running OPFSumm: kmax --> 5 to 50 (steps of 5)"

for kmax in 5 10 15 20 25 30 35 40 45 50
do
	for sample in ./dataset/*.dat
	do
		echo "Sample: $sample"
		../bin/opfsumm $sample $kmax $alpha $subset_size $plateau_type $plateau_value $pre_process $post_process	

		rm -f classifier.opf
	done
	
	mkdir ./results/k"$kmax"
	mv -f ./dataset/*.out ./results/k"$kmax"/
done

echo "Done."
