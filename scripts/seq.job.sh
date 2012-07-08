#!/bin/bash
#
#PBS -l nodes=1:hex:ppn=24
#PBS -l walltime=4:00:00
#PBS -N radix.seq
#PBS -m bea
#PBS -e out_601/seq.err
#PBS -o out_601/seq.out

cd $PBS_O_WORKDIR

NUM_EXECS=1
G=(2 3 4 5 6 7 8)
SIZES=(2048 32768 524288 8388608 134217728)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		mkdir -p results_601/seq/gcc
		mkdir -p results_601/seq/icc
		output_gcc=results_601/seq/gcc/g${g}_s${size}
		output_icc=results_601/seq/icc/g${g}_s${size}
		rm $output && touch $output

		for try in `seq 1 $NUM_EXECS`; do
			echo "running try $try, g=$g, size=$size"
			echo "running try $try, g=$g, size=$size" 1>&2
			bin_gcc/radix.seq $size $g >> $output_gcc
			bin_icc/radix.seq $size $g >> $output_icc
		done
	done
done
