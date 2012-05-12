#!/bin/bash
#
#PBS -l nodes=1:r601:ppn=24
#PBS -l walltime=2:00:00
#PBS -N radix.seq
#PBS -m bea
#PBS -e out/seq.err
#PBS -o out/seq.out

cd $PBS_O_WORKDIR

NUM_EXECS=5
G=(2 4 8)
SIZES=(256 4096 65536 1048576 16777216)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		mkdir -p results/seq
		output=results/seq/g${g}_s${size}
		rm $output && touch $output

		for try in `seq 1 $NUM_EXECS`; do
			echo "running try $try, g=$g, size=$size"
			bin/radix.seq $size $g >> $output
		done
	done
done
