#!/bin/bash
#
#PBS -l nodes=1:r401:ppn=16
#PBS -l walltime=2:00:00
#PBS -N 401_radix.seq
#PBS -m bea
#PBS -e out_401/seq.err
#PBS -o out_401/seq.out

cd $PBS_O_WORKDIR

NUM_EXECS=5
G=(2 4 8)
SIZES=(256 4096 65536 1048576 16777216)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		mkdir -p results_401/seq
		output=results_401/seq/g${g}_s${size}
		rm $output && touch $output

		for try in `seq 1 $NUM_EXECS`; do
			echo "running try $try, g=$g, size=$size"
			bin/radix.seq $size $g >> $output
		done
	done
done
