#!/bin/bash
#SBATCH --cpus-per-task=1
#SBATCH --mem=16G
#SBATCH --time=9:20:00
#SBATCH --array=1-30
#SBATCH --output=arrayjob_%A_%a.out

i=1
while read -r line; do
    if [ $SLURM_ARRAY_TASK_ID -eq $i ]
    then
        ./cppcode/build/cppex datasets/testfiles/$line.test > logs/full/$1/$line.txt
    fi
    (( i = $i +1 ))

    if [ $SLURM_ARRAY_TASK_ID -eq $i ]
    then
        ./cppcode/build/cppex datasets/testfiles/$line.test D > logs/full/$1/D/$line.txt
    fi
    (( i = $i +1 ))

done < all_series.txt
