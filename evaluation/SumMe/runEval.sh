#!/bin/bash
clear

mkdir summary

for opf_output in ./*.out
do
	filepath=${opf_output##*/}
	filename=${filepath%.gch.dat.out}
	sample_file=$filename.key
	
	echo "File path: $filepath"
	echo "File name: $filename"
	echo "Sample file: $sample_file"
	
	#sleep 3
	cp $sample_file ./summary
	
	python ./python/eval.py -k -ext .key ./summary/$filename.key
done



