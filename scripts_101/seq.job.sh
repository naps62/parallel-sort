#!/bin/bash
#
#PBS -l nodes=1:r101:ppn=4
#PBS -l walltime=2:00:00
#PBS -N radix.seq
#PBS -m bea
#PBS -e out_101/seq.err
#PBS -o out_101/seq.out

cd $PBS_O_WORKDIR

NUM_EXECS=1
G=(4)
SIZES=(2048 32768 524288 8388608 134217728)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		mkdir -p results_101/seq
		output=results_101/seq/g${g}_s${size}
		rm $output && touch $output

		for try in `seq 1 $NUM_EXECS`; do
			echo "running try $try, g=$g, size=$size"
			bin/radix.seq $size $g >> $output
		done
	done
done
