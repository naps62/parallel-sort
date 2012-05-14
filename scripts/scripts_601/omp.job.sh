#!/bin/bash
#
#PBS -l nodes=1:hex:ppn=24
#PBS -l walltime=2:00:00
#PBS -N 601_radix.omp
#PBS -m bea
#PBS -e out_601/omp.err
#PBS -o out_601/omp.out

cd $PBS_O_WORKDIR

NUM_EXECS=1
G=(2 4 8)
SIZES=(2048 32768 524288 8388608 134217728)
THREADS=(4 8 16)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		for threads in ${THREADS[@]}; do

			mkdir -p results_601/omp
			output=results_601/omp/g${g}_s${size}_t${threads}
			rm -rf $output && touch $output

			for try in `seq 1 $NUM_EXECS`; do
				echo "running try $try, g=$g, size=$size, threads=$threads"
				bin/radix.omp $size $g $threads >> $output
			done
		done
	done
done
